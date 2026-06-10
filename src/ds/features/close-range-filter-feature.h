// License: Apache 2.0. See LICENSE file in root directory.
// Copyright(c) 2026 RealSense, Inc. All Rights Reserved.

#pragma once

#include <src/feature-interface.h>

#include <memory>


namespace librealsense {

class d500_depth_sensor;
class uvc_sensor;

class close_range_filter_feature : public feature_interface
{
public:
    static const feature_id ID;

    close_range_filter_feature( d500_depth_sensor & depth_sensor, std::weak_ptr< uvc_sensor > raw_depth_ep );

    feature_id get_id() const override;
};

}  // namespace librealsense
