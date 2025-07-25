// License: Apache 2.0. See LICENSE file in root directory.
// Copyright(c) 2022 Intel Corporation. All Rights Reserved.

#include "y16i-10msb-to-y16y16.h"
#include "stream.h"
// CUDA TODO
//#ifdef RS2_USE_CUDA
//#include "cuda/cuda-conversion.cuh"
//#endif

namespace librealsense
{
    struct y16i_pixel { uint16_t left : 16, right : 16;
                        // Explanation of "return x << 6 | x >> 4" :
                        // Since the data is received only in 10 bits, and the conversion is to 16 bits, 
                        // the range moves from [0 : 2^10-1] to [0 : 2^16-1], so the values should be converted accordingly:
                        // x in range [0 : 2^10-1] is converted to y = x * (2^16-1)/(2^10-1) approx = x * (64 + 1/16)
                        // And x * (64 + 1/16) = x * 64 + x * 1/16 = x << 6 | x >> 4
                        // This operation is done using shiftings to make it more efficient, and with a non-significant accuracy loss.
                        uint16_t l() const { return left << 6 | left >> 4; }
                        uint16_t r() const { return right << 6 | right >> 4; }
    };
    void unpack_y16_y16_from_y16i_10msb( uint8_t * const dest[], const uint8_t * source, int width, int height, int actual_size)
    {
        auto count = width * height;
// CUDA TODO
//#ifdef RS2_USE_CUDA
//        rscuda::split_frame_y16_16_from_y16i_10msb_cuda(dest, count, reinterpret_cast<const y16i_pixel*>(source));
//#else
        split_frame(dest, count, reinterpret_cast<const y16i_pixel*>(source),
            [](const y16i_pixel& p) -> uint16_t { return (p.l()); },
            [](const y16i_pixel& p) -> uint16_t { return (p.r()); });
//#endif
    }

    y16i_10msb_to_y16y16::y16i_10msb_to_y16y16(int left_idx, int right_idx)
        : y16i_10msb_to_y16y16("Y16I 10msb to Y16L Y16R Transform", left_idx, right_idx) {}

    y16i_10msb_to_y16y16::y16i_10msb_to_y16y16(const char* name, int left_idx, int right_idx)
        : interleaved_functional_processing_block(name, RS2_FORMAT_Y16I, RS2_FORMAT_Y16, RS2_STREAM_INFRARED, RS2_EXTENSION_VIDEO_FRAME, 1,
            RS2_FORMAT_Y16, RS2_STREAM_INFRARED, RS2_EXTENSION_VIDEO_FRAME, 2)
    {}

    void y16i_10msb_to_y16y16::process_function( uint8_t * const dest[], const uint8_t * source, int width, int height, int actual_size, int input_size)
    {
        unpack_y16_y16_from_y16i_10msb(dest, source, width, height, actual_size);
    }
}
