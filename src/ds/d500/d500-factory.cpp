// License: Apache 2.0. See LICENSE file in root directory.
// Copyright(c) 2022 Intel Corporation. All Rights Reserved.

#include "device.h"
#include "image.h"
#include "metadata-parser.h"

#include <src/core/matcher-factory.h>
#include <src/proc/color-formats-converter.h>
#include <src/core/advanced_mode.h>

#include "d500-info.h"
#include "d500-private.h"
#include "ds/ds-options.h"
#include "ds/ds-timestamp.h"
#include "d500-active.h"
#include "d500-color.h"
#include "d500-motion.h"
#include "sync.h"
#include <src/ds/ds-thermal-monitor.h>
#include <src/ds/d500/d500-options.h>
#include <src/ds/d500/d500-auto-calibration.h>

#include <src/platform/platform-utils.h>

#include "firmware_logger_device.h"
#include "device-calibration.h"

#include <rsutils/string/hexdump.h>
using rsutils::string::hexdump;

#include <mutex>
#include <chrono>
#include <vector>
#include <iterator>
#include <cstddef>


namespace librealsense
{


class d555_device
    : public d500_active
    , public d500_color
    , public d500_motion
    , public ds_advanced_mode_base
    , public extended_firmware_logger_device
{
public:
    d555_device( std::shared_ptr< const d500_info > dev_info )
        : device( dev_info )
        , backend_device( dev_info )
        , d500_device( dev_info )
        , d500_active( dev_info )
        , d500_color( dev_info, RS2_FORMAT_YUYV )
        , d500_motion( dev_info )
        , ds_advanced_mode_base( d500_device::_hw_monitor, get_depth_sensor() )
        , extended_firmware_logger_device( dev_info,
                                           d500_device::_hw_monitor,
                                           get_firmware_logs_command() )
    {
        auto & depth_sensor = get_depth_sensor();
        group_multiple_fw_calls(depth_sensor, [&]()
        {
            auto emitter_always_on_opt = std::make_shared<emitter_always_on_option>( d500_device::_hw_monitor,
                                                                                     ds::LASERONCONST, ds::LASERONCONST);
            depth_sensor.register_option( RS2_OPTION_EMITTER_ALWAYS_ON, emitter_always_on_opt );

            auto thermal_compensation_toggle = std::make_shared< d500_thermal_compensation_option >( d500_device::_hw_monitor );

            // Monitoring SOC PVT (not OHM) because it correlates to D400 ASIC temperature and we keep the model the same.
            auto temperature_sensor = depth_sensor.get_option_handler( RS2_OPTION_SOC_PVT_TEMPERATURE );

            _thermal_monitor = std::make_shared< ds_thermal_monitor >( temperature_sensor, thermal_compensation_toggle );

            depth_sensor.register_option( RS2_OPTION_THERMAL_COMPENSATION,
                                          std::make_shared< thermal_compensation >( _thermal_monitor, thermal_compensation_toggle ) );

            // We usually use "Custom" visual preset becasue we don't know what is the current setting.
            // When connected by Ethernet D555 does not support "Custom" so we set here to "Default" to match.
            depth_sensor.get_option( RS2_OPTION_VISUAL_PRESET ).set( RS2_RS400_VISUAL_PRESET_DEFAULT );
        } );  // group_multiple_fw_calls
    }

    std::shared_ptr< matcher > create_matcher( const frame_holder & frame ) const override
    {
        std::vector<stream_interface *> streams = { _depth_stream.get() , _left_ir_stream.get() , _right_ir_stream.get(),     _color_stream.get() };
        std::vector<stream_interface *> mm_streams = { _ds_motion_common->get_accel_stream().get(),
                                                       _ds_motion_common->get_gyro_stream().get() };
        streams.insert( streams.end(), mm_streams.begin(), mm_streams.end() );
        return matcher_factory::create( RS2_MATCHER_DEFAULT, streams );
    }


    std::vector< tagged_profile > get_profiles_tags() const override
    {
        std::vector< tagged_profile > tags;

        tags.push_back( { RS2_STREAM_COLOR, -1,
                          896, 504, RS2_FORMAT_RGB8, 30,
                          profile_tag::PROFILE_TAG_SUPERSET | profile_tag::PROFILE_TAG_DEFAULT } );
        tags.push_back( { RS2_STREAM_DEPTH, -1,
                          896, 504, RS2_FORMAT_Z16, 30,
                          profile_tag::PROFILE_TAG_SUPERSET | profile_tag::PROFILE_TAG_DEFAULT } );
        tags.push_back( { RS2_STREAM_INFRARED, -1,
                          896, 504, RS2_FORMAT_Y8, 30,
                          profile_tag::PROFILE_TAG_SUPERSET } );
        tags.push_back( { RS2_STREAM_GYRO, -1,
                          0, 0, RS2_FORMAT_MOTION_XYZ32F, (int)odr::IMU_FPS_200,
                          profile_tag::PROFILE_TAG_SUPERSET | profile_tag::PROFILE_TAG_DEFAULT } );
        tags.push_back( { RS2_STREAM_ACCEL, -1,
                          0, 0, RS2_FORMAT_MOTION_XYZ32F, (int)odr::IMU_FPS_100,
                          profile_tag::PROFILE_TAG_SUPERSET | profile_tag::PROFILE_TAG_DEFAULT } );

        return tags;
    };

    bool contradicts( const stream_profile_interface * a, const std::vector< stream_profile > & others ) const override
    {
        if( auto vid_a = dynamic_cast< const video_stream_profile_interface * >( a ) )
        {
            for( auto & request : others )
            {
                if( a->get_framerate() != 0 && request.fps != 0 && ( a->get_framerate() != request.fps ) )
                    return true;
            }
        }
        return false;
    }
};

    std::shared_ptr< device_interface > d500_info::create_device()
    {
        using namespace ds;

        if( _group.uvc_devices.empty() )
            throw std::runtime_error("Depth Camera not found!");

        auto dev_info = std::dynamic_pointer_cast< const d500_info >( shared_from_this() );

        auto pid = _group.uvc_devices.front().pid;
        switch( pid )
        {
        case ds::D555_PID:
            return std::make_shared< d555_device >( dev_info );

        default:
            throw std::runtime_error( rsutils::string::from() << "unsupported D500 PID 0x" << hexdump( pid ) );
        }
    }

    std::vector<std::shared_ptr<d500_info>> d500_info::pick_d500_devices(
        std::shared_ptr<context> ctx,
        platform::backend_device_group& group)
    {
        std::vector<platform::uvc_device_info> chosen;
        std::vector<std::shared_ptr<d500_info>> results;

        auto valid_pid = filter_by_product(group.uvc_devices, ds::rs500_sku_pid);
        auto group_devices = group_devices_and_hids_by_unique_id(group_devices_by_unique_id(valid_pid), group.hid_devices);

        for (auto& g : group_devices)
        {
            auto& devices = g.first;
            auto& hids = g.second;

            bool is_mi_0_present = mi_present(devices, 0);

            // Device with multi sensors can be enabled only if all sensors (RGB + Depth) are present
            auto is_pid_of_multisensor_device = [](int pid) { return std::find(std::begin(ds::d500_multi_sensors_pid), 
                std::end(ds::d500_multi_sensors_pid), pid) != std::end(ds::d500_multi_sensors_pid); };


#if !defined(__APPLE__) // Not supported by macos
            auto is_pid_of_hid_sensor_device = [](int pid) { return std::find(std::begin(ds::d500_hid_sensors_pid), 
                std::end(ds::d500_hid_sensors_pid), pid) != std::end(ds::d500_hid_sensors_pid); };
            bool is_device_hid_sensor = false;
            for (auto&& uvc : devices)
            {
                if (is_pid_of_hid_sensor_device(uvc.pid))
                    is_device_hid_sensor = true;
            }
#endif

            if (!devices.empty() && is_mi_0_present)
            {
                platform::usb_device_info hwm;

                std::vector<platform::usb_device_info> hwm_devices;
                if (ds::d500_try_fetch_usb_device(group.usb_devices, devices.front(), hwm))
                {
                    hwm_devices.push_back(hwm);
                }

                auto info = std::make_shared< d500_info >( ctx,
                                                           std::move( devices ),
                                                           std::move( hwm_devices ),
                                                           std::move( hids ) );
                chosen.insert(chosen.end(), devices.begin(), devices.end());
                results.push_back(info);

            }
        }

        trim_device_list(group.uvc_devices, chosen);

        return results;
    }

    inline std::shared_ptr<matcher> create_composite_matcher(std::vector<std::shared_ptr<matcher>> matchers)
    {
        return std::make_shared<timestamp_composite_matcher>(matchers);
    }
}
