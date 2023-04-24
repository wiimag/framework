/*
 * Copyright 2023 - All rights reserved.
 * License: https://equals-forty-two.com/LICENSE
 * 
 * Framework API interface
 */

#pragma once

#include "types.h"

#ifndef BUILD_DYNAMIC_LINK
#   define BUILD_DYNAMIC_LINK 0
#endif

#ifndef FOUNDATION_PLATFORM_WINDOWS
    #ifdef _WIN32
        #define FOUNDATION_PLATFORM_WINDOWS 1
    #else
        #define FOUNDATION_PLATFORM_WINDOWS 0
    #endif
#endif

#if BUILD_DYNAMIC_LINK && FOUNDATION_PLATFORM_WINDOWS
#ifdef __cplusplus
#   define FRAMEWORK_API_EXPORT extern "C" __declspec(dllexport)
#else
#   define FRAMEWORK_API_EXPORT __declspec(dllexport)
#endif

#else
#   define FRAMEWORK_API_EXPORT
#endif

#ifndef STRING_PARAM
#   define STRING_PARAM(__name) const char* __name, size_t __name##_length
#endif

struct api_app_t
{
    void (*register_menu)(STRING_PARAM(path), STRING_PARAM(shortcut), app_menu_flags_t flags, api_callback_t callback, user_data_t user_data);
};

struct api_log_t
{
    void (*info)(hash_t context, const char* fmt, size_t length, ...);
    void (*warning)(hash_t context, const char* fmt, size_t length, ...);
    void (*error)(hash_t context, uint32_t err, const char* fmt, size_t length, ...);
};

struct api_dispatcher_t
{
    object_t (*register_event)(STRING_PARAM(event_name), bool(*callback)(void* payload, size_t payload_size, user_data_t user_data), user_data_t user_data);
    bool (*unregister_event)(object_t register_event_id);
    bool (*post_event)(STRING_PARAM(event_name), void* payload, size_t payload_size);

    void (*call_delayed)(api_callback_t callback, user_data_t user_data);
};

struct api_system_t {};
struct api_expression_t {};

struct api_interface_t
{
    api_app_t* app{ nullptr };
    api_log_t* log{ nullptr };
    api_dispatcher_t* dispatcher{ nullptr };
    api_expression_t* expression{ nullptr };
    api_system_t* system{ nullptr };
};

struct api_plugin_context_t;

/*! Plugin API initialization structure */
typedef void (*initialize_apis_fn)(const api_plugin_context_t* context, api_interface_t** api);

struct api_plugin_callbacks_t
{
    void (*update)() { nullptr };
    void (*render)() { nullptr };
};

/*! Structure used to load and initialize a plugin.
    The plugin must implement the load_plugin and unload_plugin functions.
*/
struct api_plugin_context_t
{
    /*! Set plugin context. This is used for logging and memory allocation. */
    hash_t context;

    /*! Display name of the plugin */
    char name[32];

    /*! Version major of the plugin */
    int version_major;

    /*! Version minor of the plugin */
    int version_minor;

    /*! The plugin can invoke this function to initialize the APIs it needs.
     *  The framework will fill all the requested APIs and the plugin can use them.
     */
    initialize_apis_fn initialize_apis;

    /*! Plugin callbacks that the system invokes at certain times */
    api_plugin_callbacks_t callbacks;

    /*! Reserved for internal usage */
    uint32_t reserved{ UINT32_MAX };
};

/*! Load plugin function
    @param plugin_context Plugin API context
    @return 0 if plugin loaded successfully, <0 if error occurred
*/
typedef int (*load_plugin_api_fn)(api_plugin_context_t* plugin_context);

/*! Unload plugin function
*/
typedef int (*unload_plugin_api_fn)();
