# License: Apache 2.0. See LICENSE file in root directory.
# Copyright(c) 2026 RealSense, Inc. All Rights Reserved.

# Not frequently changing, no need to test for each commit
# test:donotrun:!nightly
# test:donotrun:!dds
# test:device D555

import pyrealsense2 as rs
from rspy import test, log
from rspy.timer import Timer
import pyrsutils as rsutils
import time


# Improved Close Range Depth defaults, cross-checked against FW-advertised filter JSON.
#
# The downscale ratio is exposed by FW as a dds_enum_option with choices ["1","2","4"];
# librealsense exposes enum options as float = choice index.
# Default choice is "2", which is index 1.
CLOSE_RANGE_ENABLE_DEFAULT = 0.0        # filter defaults to OFF
CLOSE_RANGE_RATIO_DEFAULT_INDEX = 1.0   # choices ["1","2","4"], default "2" -> index 1
CLOSE_RANGE_SHIFT_DEFAULT = 0.0
CLOSE_RANGE_THRESHOLD_DEFAULT = 550.0

# Metadata bit in the embedded_filters bitmask (DPP Filter Bitmask bit 5)
CLOSE_RANGE_METADATA_BIT = 1 << 5

# FW gate from spec section 6 (compatibility matrix)
CLOSE_RANGE_MIN_FW = '7.58.39650'

MAX_TIME_TO_WAIT_FOR_FRAMES = 10  # [sec]


def check_option_in_list(option_id, options_list):
    for option in options_list:
        if option_id == option:
            return True
    return False


def set_get_filter_option_value(embedded_filter, option, value_to_assign):
    initial_value = embedded_filter.get_option(option)
    option_step = embedded_filter.get_option_range(option).step
    embedded_filter.set_option(option, value_to_assign)
    test.check_approx_abs(embedded_filter.get_option(option), value_to_assign, option_step)
    embedded_filter.set_option(option, initial_value)
    test.check_approx_abs(embedded_filter.get_option(option), initial_value, option_step)


def is_fw_version_below(curr_fw, min_fw):
    current_fw_version = rsutils.version(curr_fw)
    min_fw_version = rsutils.version(min_fw)
    return current_fw_version < min_fw_version


dev, _ = test.find_first_device_or_exit()
depth_sensor = dev.first_depth_sensor()
fw_version = dev.get_info(rs.camera_info.firmware_version)

depth_profile = next(p for p in
                     depth_sensor.profiles if p.fps() == 30
                     and p.stream_type() == rs.stream.depth
                     and p.format() == rs.format.z16
                     and p.as_video_stream_profile().width() == 640
                     and p.as_video_stream_profile().height() == 360)

test_close_range_filter = not is_fw_version_below(fw_version, CLOSE_RANGE_MIN_FW)

if test_close_range_filter:
    with test.closure("Get Improved Close Range Depth embedded filter options"):
        close_range_filter = depth_sensor.get_embedded_filter(rs.embedded_filter_type.improved_close_range_depth)
        test.check(close_range_filter)

        close_range_options = close_range_filter.get_supported_options()
        test.check_equal(len(close_range_options), 4)
        test.check(check_option_in_list(rs.option.embedded_filter_enabled, close_range_options))
        test.check(check_option_in_list(rs.option.downscale_ratio, close_range_options))
        test.check(check_option_in_list(rs.option.disparity_shift, close_range_options))
        test.check(check_option_in_list(rs.option.threshold, close_range_options))

    with test.closure("Improved Close Range Depth embedded filter defaults"):
        test.check_equal(close_range_filter.get_option(rs.option.embedded_filter_enabled), CLOSE_RANGE_ENABLE_DEFAULT)
        test.check_equal(close_range_filter.get_option(rs.option.downscale_ratio), CLOSE_RANGE_RATIO_DEFAULT_INDEX)
        test.check_equal(close_range_filter.get_option(rs.option.disparity_shift), CLOSE_RANGE_SHIFT_DEFAULT)
        test.check_equal(close_range_filter.get_option(rs.option.threshold), CLOSE_RANGE_THRESHOLD_DEFAULT)

    with test.closure("Improved Close Range Depth embedded filter set/get options"):
        # Disable first so we can safely permute downscale ratio without tripping the active-mutex on the device
        close_range_filter.set_option(rs.option.embedded_filter_enabled, 0.0)
        # Disparity shift first: spec says shift>0 forces ratio=1, so test shift before ratio
        # to avoid the order-dependency between the two options.
        set_get_filter_option_value(close_range_filter, rs.option.disparity_shift, 100.0)
        # Ratio: index 2 -> choice "4" (quarter-res)
        set_get_filter_option_value(close_range_filter, rs.option.downscale_ratio, 2.0)
        set_get_filter_option_value(close_range_filter, rs.option.threshold, 600.0)
        # Restore enable for the metadata test below
        close_range_filter.set_option(rs.option.embedded_filter_enabled, CLOSE_RANGE_ENABLE_DEFAULT)

    with test.closure("Improved Close Range Depth invalid ratio rejected"):
        # Ratio is an enum-index in [0..2] for choices "1"/"2"/"4" - 3.0 is out of range
        test.check_throws(
            lambda: close_range_filter.set_option(rs.option.downscale_ratio, 3.0),
            RuntimeError)

    waiting_for_test = False
    wait_for_frames_timer = Timer(MAX_TIME_TO_WAIT_FOR_FRAMES)
    close_range_enabled = False

    def close_range_check_callback(frame):
        # Only stop waiting once we've actually checked a frame carrying the embedded_filters
        # metadata - otherwise an early frame with no metadata could silently pass the test.
        global waiting_for_test, close_range_enabled
        if frame.supports_frame_metadata(rs.frame_metadata_value.embedded_filters):
            md_val = frame.get_frame_metadata(rs.frame_metadata_value.embedded_filters)
            value_to_check = CLOSE_RANGE_METADATA_BIT if close_range_enabled else 0
            test.check_equal(md_val & CLOSE_RANGE_METADATA_BIT, value_to_check)
            waiting_for_test = False

    def stream_and_check_close_range_filter():
        global waiting_for_test
        depth_sensor.open(depth_profile)
        depth_sensor.start(close_range_check_callback)
        wait_for_frames_timer.start()
        waiting_for_test = True
        while waiting_for_test and not wait_for_frames_timer.has_expired():
            time.sleep(0.5)
        test.check(not wait_for_frames_timer.has_expired())
        depth_sensor.stop()
        depth_sensor.close()

    def enable_close_range_filter():
        close_range_filter.set_option(rs.option.embedded_filter_enabled, 1.0)
        test.check_equal(close_range_filter.get_option(rs.option.embedded_filter_enabled), 1.0)

    def disable_close_range_filter():
        close_range_filter.set_option(rs.option.embedded_filter_enabled, 0.0)
        test.check_equal(close_range_filter.get_option(rs.option.embedded_filter_enabled), 0.0)

    # Skipped due to known FW issue: the close-range bit (1 << 5) of the embedded_filters
    # metadata is not set on depth frames even when the filter is enabled. Re-enable once
    # the FW fix is available.
    SKIP_METADATA_BIT_TEST_FW_ISSUE = True

    if not SKIP_METADATA_BIT_TEST_FW_ISSUE:
        with test.closure("Improved Close Range Depth embedded filter metadata member"):
            disable_close_range_filter()
            close_range_enabled = False
            stream_and_check_close_range_filter()
            time.sleep(1)
            enable_close_range_filter()
            close_range_enabled = True
            stream_and_check_close_range_filter()
            time.sleep(1)
            disable_close_range_filter()
    else:
        log.w("Skipping 'Improved Close Range Depth embedded filter metadata member' "
              "- known FW issue (metadata bit not set)")
else:
    print("Improved Close Range Depth Embedded Filter not tested")


test.print_results_and_exit()
