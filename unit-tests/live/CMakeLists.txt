
# recordings URL is same as in live-test.cmake
set(PP_Rosbag_Recordings_URL https://librealsense.intel.com/rs-tests/Rosbag_unit_test_records)

# for rec-play/test-non-realtime.py
dl_file( ${PP_Rosbag_Recordings_URL} recordings recording_deadlock.bag OFF )

# for rec-play/test-playback-stress.py and for post-processing/post-processing-from-bag.py
dl_file( ${PP_Rosbag_Recordings_URL} recordings all_combinations_depth_color.bag OFF )

# for post-processing/post-processing-from-bag.py
dl_file( ${PP_Rosbag_Recordings_URL} recordings [aligned_2c]_all_combinations_depth_color.bag OFF )

# for post-processing/post-processing-from-bag.py
dl_file( ${PP_Rosbag_Recordings_URL} recordings [aligned_2d]_all_combinations_depth_color.bag OFF )

# for post-processing/post-processing-from-bag.py
dl_file( ${PP_Rosbag_Recordings_URL} recordings [pointcloud]_all_combinations_depth_color.bag OFF )

# for 3D/projection-from-recording.bag
dl_file( ${PP_Rosbag_Recordings_URL} recordings single_depth_color_640x480.bag OFF )

