/*
 * Copyright 2023 - All rights reserved.
 * License: https://equals-forty-two.com/LICENSE
 * 
 * Plugin Implementation
 */

#include "plugin.h"

#include <api/api.h>

#include <framework/app.h>
#include <framework/module.h>

#include <foundation/path.h>
#include <foundation/library.h>
#include <foundation/environment.h>

#define HASH_PLUGIN static_hash_string("plugin", 6, 0xcbbfaea91dab646ULL)

struct plugin_t
{
    object_t handle{ 0 };
    api_interface_t* api{ nullptr };

    load_plugin_api_fn load{ nullptr };
    unload_plugin_api_fn unload{ nullptr };

    api_plugin_context_t context;
};

static plugin_t* _plugins = nullptr;

FOUNDATION_FORCEINLINE plugin_t* plugin_get(plugin_handle_t handle)
{
    FOUNDATION_ASSERT(handle.index < array_size(_plugins));
    return &_plugins[handle.index];
}

FOUNDATION_FORCEINLINE plugin_t* plugin_resolve(plugin_handle_t handle)
{
    if (handle.index >= array_size(_plugins))
        return nullptr;
    return &_plugins[handle.index];
}

FOUNDATION_STATIC void app_initialize_apis(const api_plugin_context_t* context, api_interface_t** api)
{
    FOUNDATION_ASSERT(api);

    plugin_t* plugin = plugin_get({context->reserved});
    plugin->api = *api = (api_interface_t*)memory_allocate(HASH_PLUGIN, sizeof(api_interface_t), 8, MEMORY_PERSISTENT | MEMORY_ZERO_INITIALIZED);

    //
    // App API
    //
    static api_app_t app_api
    {
        // register_menu
        [](const char* path, size_t path_length, const char* shortcut, size_t shortcut_length, app_menu_flags_t flags, void(*callback)(void*), void* user_data)
        {
            app_register_menu(HASH_PLUGIN, path, path_length, shortcut, shortcut_length, flags, callback, user_data);
        }
    };

    //
    // Log API
    //
    static api_log_t log_api
    {
        // info
        [](hash_t context, const char* fmt, size_t length, ...)
        {
            va_list args;
            va_start(args, length);

            string_t str = string_allocate_vformat(fmt, length, args);
            log_info(context, STRING_ARGS(str));
            string_deallocate(str.str);
            va_end(args);
        },

        // warning
        [](hash_t context, const char* fmt, size_t length, ...)
        {
            va_list args;
            va_start(args, length);
            string_t str = string_allocate_vformat(fmt, length, args);
            log_warn(context, WARNING_SCRIPT, STRING_ARGS(str));
            string_deallocate(str.str);
            va_end(args);
        },

        // error
        [](hash_t context, uint32_t err, const char* fmt, size_t length, ...)
        {
            va_list args;
            va_start(args, length);
            string_t str = string_allocate_vformat(fmt, length, args);
            log_error(context, (error_t)err, STRING_ARGS(str));
            string_deallocate(str.str);
            va_end(args);
        },
    };

    //
    // Dispatcher API
    //
    static api_dispatcher_t dispatcher_api
    {
        // register_event
        [](const char* event_name, size_t event_name_length, bool(*callback)(void* payload, size_t payload_size, void* user_data), void* user_data)
        {
            return dispatcher_register_event_listener(event_name, event_name_length, [callback](const dispatcher_event_args_t& args)
            {
                return callback(args.data, args.size, args.user_data);
            }, DISPATCHER_EVENT_OPTION_NONE, user_data);
        },

        // unregister_event
        [](uint32_t register_event_id)
        {
            return dispatcher_unregister_event_listener(register_event_id);
        },

        // post_event
        [](const char* event_name, size_t event_name_length, void* payload, size_t payload_size)
        {
            return dispatcher_post_event(event_name, event_name_length, payload, 
                payload_size, payload_size > 0 ? DISPATCHER_EVENT_OPTION_COPY_DATA : DISPATCHER_EVENT_OPTION_NONE);
        },

        // call_delayed
        [](void(*callback)(void* user_data), void* user_data)
        {
            if (user_data)
                dispatch([callback, user_data]() { callback(user_data); });
            else
                dispatch([callback]() { callback(nullptr); });
        },
    };

    (*api)->app = &app_api;
    (*api)->log = &log_api;
    (*api)->dispatcher = &dispatcher_api;
}

//
// PUBLIC
//

plugin_handle_t plugin_load(const char* path, size_t length)
{
    plugin_t plugin{};
    string_const_t plugin_file_name = path_file_name(path, length);

    plugin.handle = library_load(path, length);
    if (plugin.handle == 0)
    {
        log_warnf(HASH_PLUGIN, WARNING_SUSPICIOUS, STRING_CONST("Unable to load plugin: %.*s"), STRING_FORMAT(plugin_file_name));
        return { PLUGIN_HANDLE_INVALID };
    }

    // Make sure the dll has a valid plugin interface (i.e. `load_plugin(...)`)
    plugin.load = (load_plugin_api_fn)library_symbol(plugin.handle, STRING_CONST("load_plugin"));
    plugin.unload = (unload_plugin_api_fn)library_symbol(plugin.handle, STRING_CONST("unload_plugin"));
    if (plugin.load == nullptr)
    {
        log_warnf(HASH_PLUGIN, WARNING_SUSPICIOUS, STRING_CONST("Unable to initialize plugin: %.*s"), STRING_FORMAT(plugin_file_name));
        library_release(plugin.handle);
        return { PLUGIN_HANDLE_INVALID };
    }

    plugin_handle_t handle = { array_size(_plugins) };
    plugin.context.reserved = handle.index;
    plugin.context.initialize_apis = app_initialize_apis;

    string_const_t plugin_base_name = path_base_file_name(STRING_ARGS(plugin_file_name));
    string_copy(STRING_BUFFER(plugin.context.name), STRING_ARGS(plugin_base_name));

    array_push(_plugins, plugin);
    int exit_code = plugin.load(&plugin.context);
    if (exit_code != 0)
    {
        log_warnf(HASH_PLUGIN, WARNING_SUSPICIOUS, STRING_CONST("Unable to initialize plugin: %.*s (%d)"), STRING_FORMAT(plugin_file_name), exit_code);
        library_release(plugin.handle);
        array_pop_safe(_plugins);
        return { PLUGIN_HANDLE_INVALID };
    }
    
    _plugins[handle.index].context = plugin.context;
    log_infof(plugin.context.context, STRING_CONST("Loaded plugin: %s v.%d.%d"), plugin.context.name, plugin.context.version_major, plugin.context.version_minor);
    return handle;
}

unsigned plugin_count()
{
    return array_size(_plugins);
}

void plugin_render()
{
    for (unsigned i = 0, end = array_size(_plugins); i < end; ++i)
    {
        plugin_t* plugin = _plugins + i;
        if (plugin->context.callbacks.render)
            plugin->context.callbacks.render();
    }
}

void plugin_update()
{
    for (unsigned i = 0, end = array_size(_plugins); i < end; ++i)
    {
        plugin_t* plugin = _plugins + i;
        if (plugin->context.callbacks.update)
            plugin->context.callbacks.update();
    }
}

void plugin_unload(plugin_handle_t handle)
{
    plugin_t* plugin = plugin_get(handle);
    if (plugin->unload)
        plugin->unload();

    if (plugin->api)
        memory_deallocate(plugin->api);

    library_release(plugin->handle);
}

//
// SYSTEM
//

FOUNDATION_STATIC void plugins_initialize()
{
    // Find plugin directory
    char plugin_dir_buffer[BUILD_MAX_PATHLEN];
    string_const_t resources_dir = environment_get_resources_path();
    string_t plugin_dir = string_copy(plugin_dir_buffer, sizeof(plugin_dir_buffer), resources_dir.str, resources_dir.length);
    plugin_dir = string_append(STRING_ARGS(plugin_dir), sizeof(plugin_dir_buffer), STRING_CONST("/plugins"));
    if (!fs_is_directory(STRING_ARGS(plugin_dir)))
        return;

    // Load plugins (i.e. all DLLs in plugin directory)
    string_t* plugin_file_names = fs_matching_files(STRING_ARGS(plugin_dir), STRING_CONST("^.*\\.dll$"), false);
    for (unsigned i = 0, end = array_size(plugin_file_names); i < end; ++i)
    {
        const string_t& plugin_file_name = plugin_file_names[i];

        char plugin_file_path_buffer[BUILD_MAX_PATHLEN];
        string_t plugin_path = path_concat(STRING_BUFFER(plugin_file_path_buffer), STRING_ARGS(plugin_dir), STRING_ARGS(plugin_file_name));

        plugin_load(STRING_ARGS(plugin_path));
    }

    string_array_deallocate(plugin_file_names);
}

FOUNDATION_STATIC void plugins_shutdown()
{
    for (unsigned i = 0, end = array_size(_plugins); i < end; ++i)
    {
        plugin_t* plugin = _plugins + i;
        plugin_unload({i});
    }
    array_deallocate(_plugins);
}

DEFINE_MODULE(PLUGIN, plugins_initialize, plugins_shutdown, MODULE_PRIORITY_BASE);
