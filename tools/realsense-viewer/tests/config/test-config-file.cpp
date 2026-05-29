// License: Apache 2.0. See LICENSE file in root directory.
// Copyright(c) 2026 RealSense, Inc. All Rights Reserved.

#include "viewer-test-helpers.h"
#include "rs-config.h"

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
