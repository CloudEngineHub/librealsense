// License: Apache 2.0. See LICENSE file in root directory.
// Copyright(c) 2026 RealSense, Inc. All Rights Reserved.
#pragma once

#include <string>


namespace rsutils {
namespace os {


// Writes content to filename atomically via a temp file and rename.
// The temp file is named "<filename>.<pid>.<tid>.<counter>.tmp" (thread-unique); stale copies are benign and safe to delete.
// Returns true on success, false on any failure (open, write, or rename).
//
bool atomic_write_file( const std::string & filename, const std::string & content );


}  // namespace os
}  // namespace rsutils
