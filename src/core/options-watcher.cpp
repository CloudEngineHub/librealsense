// License: Apache 2.0. See LICENSE file in root directory.
// Copyright(c) 2023 Intel Corporation. All Rights Reserved.


#include <src/core/options-watcher.h>
#include <proc/synthetic-stream.h>


namespace librealsense {


options_watcher::options_watcher( std::chrono::milliseconds update_interval )
    : _update_interval( update_interval )
    , _destructing( false )
{
}

options_watcher::~options_watcher()
{
    _destructing = true;
    stop();
}

void options_watcher::register_option( rs2_option id, std::shared_ptr< option > option )
{
    registered_option opt = { option, 0.0f }; //Option actual value will be queried in start if needed.

    {
        std::lock_guard< std::mutex > lock( _mutex );
        _options[id] = std::move( opt );
    }

    if( should_start() )
        start();
}

void options_watcher::unregister_option( rs2_option id )
{
    {
        std::lock_guard< std::mutex > lock( _mutex );
        _options.erase( id );
    }

    if( should_stop() )
        stop();
}

rsutils::subscription options_watcher::subscribe( callback && cb )
{
    rsutils::subscription ret = _on_values_changed.subscribe( std::move( cb ) );

    if( should_start() )
        start();

    return ret;
}

bool options_watcher::should_start() const
{
    return ! should_stop();
}

bool options_watcher::should_stop() const
{
    return _on_values_changed.size() == 0 || _options.size() == 0 || _destructing;
}

void options_watcher::start()
{
    if( ! _updater.joinable() ) // If not already started
    {
        _updater = std::thread( [this]() {
            update_options();
            thread_loop();
        } );
    }
}

void options_watcher::stop()
{
    _stopping.notify_all();
    if( _updater.joinable() )
    {
        try
        {
            _updater.join();
        }
        catch( ... )
        {
            // Nothing to do on error
        }
    }
}

void options_watcher::thread_loop()
{
    // Checking should_stop because subscriptions can be canceled without us knowing
    while( ! should_stop() )
    {
        {
            std::unique_lock< std::mutex > lock( _mutex );
            _stopping.wait_for( lock, _update_interval );
        }

        // Checking for stop conditions after sleep.
        if( should_stop() )
            break;

        std::map< rs2_option, std::shared_ptr< option > > updated_options = update_options();

        // Checking stop conditions after update, if stop requested no need to notify.
        if( should_stop() )
            break;

        notify( updated_options );
    }
}

std::map< rs2_option, std::shared_ptr< option > > options_watcher::update_options()
{
    std::map< rs2_option, std::shared_ptr< option > > updated_options;

    std::lock_guard< std::mutex > lock( _mutex );

    if( should_stop() )
        return updated_options;

    for( auto & opt : _options )
    {
        try
        {
            auto curr_val = opt.second.sptr->query();

            if( opt.second.last_known_value != curr_val )
            {
                opt.second.last_known_value = curr_val;
                updated_options[opt.first] = opt.second.sptr;
            }
        }
        catch( ... )
        {
            // Some options cannot be queried all the time (i.e. streaming only)
        }

        // Checking stop conditions after each query to ensure stop when requested.
        if( should_stop() )
            break;
    }

    return updated_options;
}

void options_watcher::notify( const std::map< rs2_option, std::shared_ptr< option > > & updated_options )
{
    if( ! updated_options.empty() )
        _on_values_changed.raise( updated_options );
}

}  // namespace librealsense
