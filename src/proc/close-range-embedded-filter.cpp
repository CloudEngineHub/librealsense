// License: Apache 2.0. See LICENSE file in root directory.
// Copyright(c) 2026 RealSense, Inc. All Rights Reserved.

#include "close-range-embedded-filter.h"

rsutils::subscription librealsense::close_range_embedded_filter::register_options_changed_callback(options_watcher::callback&& cb)
{
    return _options_watcher.subscribe(std::move(cb));
}
