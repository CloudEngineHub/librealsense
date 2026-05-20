// License: Apache 2.0. See LICENSE file in root directory.
// Copyright(c) 2023 RealSense, Inc. All Rights Reserved.

#include "rs-config.h"

#include <librealsense2/rs.h>
#include <rsutils/os/special-folder.h>
#include <rsutils/json.h>
#include <rsutils/json-config.h>
#include <fstream>
#include <cstdio>
#include <string>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

using json = rsutils::json;

using namespace rs2;

static bool atomic_replace_file(const char* source, const char* target)
{
#ifdef _WIN32
    return MoveFileExA(source, target, MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH) != 0;
#else
    return std::rename(source, target) == 0;
#endif
}

static std::string make_temp_filename(const char* filename)
{
    std::string temp(filename);
    temp += ".";

    // Append the process ID to the temp filename to avoid conflicts when multiple instances of the application are running
#ifdef _WIN32
    temp += std::to_string(GetCurrentProcessId());
#else
    temp += std::to_string(getpid());
#endif
    temp += ".tmp";
    return temp;
}

void config_file::set(const char* key, const char* value)
{
    _j[key] = value;
    save();
}

void config_file::set_default(const char* key, const char* calculate)
{
    _defaults[key] = calculate;
}

void config_file::remove(const char* key)
{
    _j.erase(key);
    save();
}

void config_file::reset()
{
    _j = json::object();
    save();
}

std::string config_file::get(const char* key, const char* def) const
{
    auto it = _j.find(key);
    if (it != _j.end() && it->is_string())
    {
        return it->string_ref();
    }
    return get_default(key, def);
}

bool config_file::contains(const char* key) const
{
    auto it = _j.find(key);
    return it != _j.end() && it->is_string();
}

std::string config_file::get_default(const char* key, const char* def) const
{
    auto it = _defaults.find(key);
    if (it == _defaults.end()) return def;
    return it->second;
}

config_value config_file::get(const char* key) const
{
    if (!contains(key)) return config_value(get_default(key, ""));
    return config_value(get(key, ""));
}

void config_file::save(const char* filename)
{
    try
    {
        const auto temp_filename = make_temp_filename(filename);

        std::ofstream out(temp_filename.c_str());
        out << std::setw( 2 ) << _j;
        out.close();

        if (!out.good())
        {
            std::remove(temp_filename.c_str());
            return;
        }

        if (!atomic_replace_file(temp_filename.c_str(), filename))
        {
            std::remove(temp_filename.c_str());
        }
    }
    catch (...)
    {
    }
}

config_file& config_file::instance()
{
    static config_file inst( rsutils::os::get_special_folder( rsutils::os::special_folder::app_data )
                             + RS2_CONFIG_FILENAME );
    return inst;
}

config_file::config_file( std::string const & filename )
    : _filename( filename )
{
    try
    {
        auto j = rsutils::json_config::load_from_file( filename );
        if( j.exists() )
            _j = std::move( j );
    }
    catch(...)
    {

    }
}

void config_file::save()
{
    save(_filename.c_str());
}

config_file::config_file()
    : _j( rsutils::json::object() )
{
}

config_file& config_file::operator=(const config_file& other)
{
    if (this != &other)
    {
        _j = other._j;
        _defaults = other._defaults;
        save();
    }
    return *this;
}

bool config_file::operator==(const config_file& other) const
{
    return _j == other._j;
}
