// License: Apache 2.0. See LICENSE file in root directory.
// Copyright(c) 2026 RealSense, Inc. All Rights Reserved.
#pragma once

#include <rsutils/json-fwd.h>
#include <realdds/dds-embedded-filter.h>
#include <dds/rs-dds-embedded-filter.h>
#include <src/proc/minz-embedded-filter.h>


namespace librealsense {

    // Class librealsense::rs_dds_embedded_minz_filter:
    // A facade for a realdds::dds_minz_filter exposing librealsense interface
    // handles librealsense embedded MinZ filter specific logic and parameter validation
    // Communication to HW is delegated to realdds::dds_minz_filter
    class rs_dds_embedded_minz_filter
        : public rs_dds_embedded_filter
        , public minz_embedded_filter
    {
    public:
        rs_dds_embedded_minz_filter(const std::shared_ptr< realdds::dds_embedded_filter >& dds_embedded_filter,
            set_embedded_filter_callback set_embedded_filter_cb,
            query_embedded_filter_callback query_embedded_filter_cb);
        virtual ~rs_dds_embedded_minz_filter() = default;

        // Override interface methods
        inline rs2_embedded_filter_type get_type() const override { return RS2_EMBEDDED_FILTER_TYPE_MINZ; }

        // Override abstract class methods
        virtual void add_option(std::shared_ptr< realdds::dds_option > option) override;

    private:
        void validate_filter_option( rsutils::json const & option_j ) const;
        void validate_disparity_shift_option( rsutils::json const & opt_j ) const;
        void validate_threshold_option( rsutils::json const & opt_j ) const;

        // FW-advertised option names for the "Improved Close Range Depth" filter.
        // static constexpr avoids per-instance std::string allocation/copy.
        static constexpr const char * ENABLE_OPTION_NAME          = "Enable";
        static constexpr const char * DOWNSCALE_RATIO_OPTION_NAME = "Secondary frame downscale ratio";  // dds_enum_option, choices {"1","2","4"}
        static constexpr const char * DISPARITY_SHIFT_OPTION_NAME = "Secondary frame disparity shift";
        static constexpr const char * THRESHOLD_OPTION_NAME       = "Threshold";
    };

}  // namespace librealsense
