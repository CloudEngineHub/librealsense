// License: Apache 2.0 See LICENSE file in root directory.
// Copyright(c) 2026 RealSense, Inc. All Rights Reserved.

#pragma once
#include <rosbag2_storage_default_plugins/sqlite/sqlite_storage.hpp>

#include "ros2_file_format.h"


namespace librealsense
{
    using namespace device_serializer;

    class context;
    class frame_source;
    class info_container;
    class options_container;

    // Reader for `.db3` files produced by native ROS2 tooling - i.e., `ros2 bag record`
    // against any source publishing standard `sensor_msgs/Image` + `sensor_msgs/CameraInfo`
    // topics (e.g., the `realsense2_camera` ROS2 wrapper). The on-disk container is the
    // same SQLite/CDR format that librealsense itself uses, but the topic schema is
    // entirely different: no `/file_version`, no `/device_N/sensor_M/...` topology, no
    // per-frame metadata messages, no option/notification topics. This class synthesizes
    // the librealsense `device_snapshot` from `sensor_msgs/CameraInfo` intrinsics + image
    // topic naming heuristics.
    //
    // For files written by the librealsense `ros2_writer` (those carry `/file_version`),
    // see `ros2_reader` instead. The factory in `ros_factory.cpp` selects between the two
    // by sniffing the first message's topic.
    class ros2_native_reader : public reader
    {
    public:
        ros2_native_reader(const std::string& file, const std::shared_ptr<context> ctx);

        device_snapshot query_device_description(const nanoseconds& time) override;
        std::shared_ptr<serialized_data> read_next_data() override;
        void seek_to_time(const nanoseconds& seek_time) override;
        std::vector<std::shared_ptr<serialized_data>> fetch_last_frames(const nanoseconds& seek_time) override;
        nanoseconds query_duration() const override;
        void reset() override;
        void enable_stream(const std::vector<stream_identifier>& stream_ids) override;
        void disable_stream(const std::vector<stream_identifier>& stream_ids) override;
        const std::string& get_file_name() const override;

    private:
        nanoseconds get_file_duration();
        frame_holder alloc_and_move_frame(std::vector<uint8_t>&& data,
            const stream_identifier& stream_id, frame_additional_data additional_data) const;
        void setup_frame(frame_interface* frame_ptr, const stream_identifier& sid) const;

        device_snapshot read_native_device_description();
        void prepare_for_streaming();
        std::shared_ptr<serialized_frame> create_frame(const std::shared_ptr<rosbag2_storage::SerializedBagMessage>& msg);

        // Topic-name heuristics: ROS2 doesn't standardize stream identification on the
        // topic - infer from path segments like /color/, /depth/, etc.
        static rs2_stream native_stream_type_from_topic(const std::string& topic);
        static rs2_format native_format_from_image_encoding(const std::string& encoding, rs2_stream stream_type);
        // Mirror typical RealSense topology: sensor 0 = RGB, 1 = Stereo (Depth+IR), 2 = Motion.
        static uint32_t native_sensor_index_for_stream(rs2_stream stream_type);

        std::shared_ptr<rosbag2_storage::storage_interfaces::ReadWriteInterface> _storage;
        std::shared_ptr<metadata_parser_map> m_metadata_parser_map;
        device_snapshot                      m_initial_device_description;
        nanoseconds                          m_total_duration;
        std::string                          m_file_path;
        std::shared_ptr<frame_source>        m_frame_source;
        std::vector<rosbag2_storage::TopicMetadata> _topics_cache;
        std::shared_ptr<context>             m_context;

        bool                                 _initialized = false;
        std::set<stream_identifier>          _enabled_streams;
        std::map<stream_identifier, std::shared_ptr<serialized_data>> _last_frame_cache;

        // Maps populated during read_native_device_description so per-frame work is O(log N):
        //   _topic_to_stream_id : topic name (storage cursor) -> stream id
        //   _profile_by_stream  : stream id -> profile (used by setup_frame to size video frames)
        std::map<std::string, stream_identifier>            _topic_to_stream_id;
        std::map<stream_identifier, std::shared_ptr<stream_profile_interface>> _profile_by_stream;
        std::map<stream_identifier, uint64_t>               _native_frame_counters;
        // Built once in prepare_for_streaming, re-applied by reset() on every seek/loop.
        std::vector<std::string>                            _streaming_filter_topics;
        // First-message epoch baseline, latched once in read_native_device_description.
        // Deliberately NOT cleared by reset() so bag-relative timestamps stay stable
        // across seek_to_time calls.
        int64_t                                             _native_first_timestamp_ns = -1;
    };
}
