// License: Apache 2.0. See LICENSE file in root directory.
// Copyright(c) 2022 RealSense, Inc. All Rights Reserved.

#pragma once

#include "device.h"
#include "core/video.h"
#include "ds-device-common.h"

#include <src/platform/uvc-option.h>

namespace librealsense
{
    // UVC PU auto-exposure control that preserves the current exposure value across
    // the AE-off transition. Switching the UVC AE control from auto to manual does not,
    // by itself, guarantee that the device re-applies a sensible exposure to the sensor:
    // on some backends the stream brightness can jump or the device keeps the last
    // auto-computed exposure even though the reported value differs. Capturing the
    // exposure before disabling AE and re-writing it after forces the device to apply it.
    class uvc_pu_auto_exposure_option : public uvc_pu_option
    {
    public:
        uvc_pu_auto_exposure_option( const std::weak_ptr< uvc_sensor > & ep,
                                     const std::weak_ptr< option > & exposure_option );

        void set( float value ) override;

    private:
        std::weak_ptr< option > _exposure_option;
    };

    class ds_color_common
    {
    public:
        ds_color_common( const std::shared_ptr< uvc_sensor > & raw_color_ep,
            synthetic_sensor& color_ep,
            firmware_version fw_version,
            std::shared_ptr<hw_monitor> hw_monitor,
            device* owner);
        void register_color_options();
        void register_standard_options();
        void register_metadata();

    private:
        std::shared_ptr< uvc_sensor > _raw_color_ep;
        synthetic_sensor& _color_ep;
        firmware_version _fw_version;
        std::shared_ptr<hw_monitor> _hw_monitor;
        device* _owner;
    };
}
