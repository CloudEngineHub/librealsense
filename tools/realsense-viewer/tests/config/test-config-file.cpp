// License: Apache 2.0. See LICENSE file in root directory.
// Copyright(c) 2026 RealSense, Inc. All Rights Reserved.

#include "viewer-test-helpers.h"
#include "rs-config.h"

#include <fstream>
#include <string>
#include <cstdio>
#include <cstdlib>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif


namespace {

static std::string get_temp_dir()
{
#ifdef _WIN32
    char buf[MAX_PATH + 1] = {};
    GetTempPathA( sizeof( buf ), buf );
    return std::string( buf );
#else
    const char * d = std::getenv( "TMPDIR" );
    return ( d && *d ) ? std::string( d ) + "/" : "/tmp/";
#endif
}

static std::string make_test_config_path()
{
#ifdef _WIN32
    std::string pid = std::to_string( GetCurrentProcessId() );
#else
    std::string pid = std::to_string( getpid() );
#endif
    return get_temp_dir() + "rs_config_test_" + pid + ".json";
}

// RAII wrapper: removes the file at path on destruction
struct scoped_file
{
    std::string path;

    explicit scoped_file( std::string p )
        : path( std::move( p ) )
    {
    }

    ~scoped_file() { std::remove( path.c_str() ); }

    void write( const std::string & content ) const
    {
        std::ofstream f( path.c_str() );
        f << content;
    }
};

}  // namespace


// Verify that a value written via set() is persisted to disk and correctly
// loaded by a fresh config_file instance.
VIEWER_TEST( "config", "regular_update" )
{
    (void)test;
    std::string path = make_test_config_path();
    scoped_file guard( path );

    {
        rs2::config_file cfg( path );
        cfg.set( "greeting", "hello_world" );
        // set() calls save() internally: writes a .pid.tmp file then atomically renames it
    }

    rs2::config_file reloaded( path );
    IM_CHECK( reloaded.contains( "greeting" ) );
    IM_CHECK( reloaded.get( "greeting", "" ) == std::string( "hello_world" ) );
}


// Simulate a crash that interrupted a save mid-way: the temp file was written
// but the atomic rename to the canonical path never happened.
// Verify that the original file is unmodified and loaded correctly on restart,
// and that a subsequent save succeeds normally.
VIEWER_TEST( "config", "original_preserved_after_crashed_write" )
{
    (void)test;
    std::string path = make_test_config_path();
    scoped_file config_guard( path );

    // Use a name that cannot collide with any real save() temp file
    // ("<path>.<pid>.<tid>.<counter>.tmp"), simulating a leftover from a prior crash.
    std::string stale_tmp_path = path + ".stale.tmp";
    scoped_file temp_guard( stale_tmp_path );

    // --- Establish original state (represents a successful earlier save) ---
    config_guard.write( R"({"key":"original_value"})" );

    // --- Simulate crash: temp file was written, rename never happened ---
    // The crashed process got this far before dying:
    temp_guard.write( R"({"key":"new_value_never_committed"})" );

    // --- App restarts: load from the canonical path ---
    rs2::config_file cfg( path );

    // The original content must survive — the incomplete save did not commit
    IM_CHECK( cfg.get( "key", "" ) == std::string( "original_value" ) );

    // --- Subsequent normal saves must still work after restart ---
    cfg.set( "key", "post_restart_value" );

    rs2::config_file reloaded( path );
    IM_CHECK( reloaded.get( "key", "" ) == std::string( "post_restart_value" ) );
}
