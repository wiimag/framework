/*
 * Copyright 2023 - All rights reserved.
 * License: https://equals-forty-two.com/LICENSE
 * 
 * Framework API interface
 */

#pragma once

#include <foundation/platform.h>

#ifndef FOUNDATION_FORCEINLINE
#   define FOUNDATION_FORCEINLINE inline
#endif

typedef uint8_t byte_t;
typedef uint64_t hash_t;
typedef int64_t tick_t;
typedef double deltatime_t;
typedef uint32_t object_t;
typedef uint128_t uuid_t;
typedef void* user_data_t;

constexpr object_t INVALID_HANDLE = UINT32_MAX;

/*! Default API callback with user data context. */
typedef void(*api_callback_t)(void* user_data);

/*! API callback with payload and user data context. */
typedef void(*api_callback_payload_t)(void* payload, size_t payload_size, void* user_data);

/*! Set of flags used to customize the registration of a new menu item. 
 *
 *  Sync with #AppMenuFlags in app.h
 */
typedef enum : uint32_t
{
    APP_MENU_NONE = 0,

    /*! Insert the menu items after all other menu items, this helps preserve the system menu order. */
    APP_MENU_APPEND = 1 << 0,

    /*! Menu item defines a shortcut */
    APP_MENU_SHORTCUT = 1 << 1,

} api_app_menu_flags_t; 

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

#ifndef STRING_CONST
#   define STRING_CONST(__name) __name, sizeof(__name) - 1
#endif

struct api_app_t
{
    /*! Register a new menu item.
     *
     *  @param path Path to the menu item, separated by slashes.
     *  @param shortcut Shortcut to the menu item, can be null.
     *  @param flags Flags to customize the registration of the menu item.
     *  @param callback Callback to invoke when the menu item is selected.
     *  @param user_data User data to pass to the callback.
     */
    void (*register_menu)(STRING_PARAM(path), STRING_PARAM(shortcut), api_app_menu_flags_t flags, api_callback_t callback, user_data_t user_data);
};

struct api_log_t
{
    /*! Log a message with the given context. 
     *
     *  @param context Context of the message.
     *  @param fmt Format string.
     *  @param length Length of the format string.
     *  @param ... Arguments to the format string.
     */
    void (*info)(hash_t context, const char* fmt, size_t length, ...);

    /*! Log a warning message with the given context. 
     *
     *  @param context Context of the message.
     *  @param fmt Format string.
     *  @param length Length of the format string.
     *  @param ... Arguments to the format string.
     */
    void (*warning)(hash_t context, const char* fmt, size_t length, ...);

    /*! Log an error message with the given context. 
     *
     *  @param context Context of the message.
     *  @param err Error code.
     *  @param fmt Format string.
     *  @param length Length of the format string.
     *  @param ... Arguments to the format string.
     */
    void (*error)(hash_t context, uint32_t err, const char* fmt, size_t length, ...);
};

struct api_dispatcher_t
{
    /*! Register a new event listener.
     *
     *  @param event_name Name of the event.
     *  @param callback Callback to invoke when the event is posted.
     *  @param user_data User data to pass to the callback.
     *  @return Handle to the registered event.
     */
    object_t (*register_event)(STRING_PARAM(event_name), bool(*callback)(void* payload, size_t payload_size, user_data_t user_data), user_data_t user_data);

    /*! Unregister an event listener.
     *
     *  @param register_event_id Handle to the registered event.
     *  @return True if the event was unregistered, false otherwise.
     */
    bool (*unregister_event)(object_t register_event_id);

    /*! Post an message event.
     *
     *  @param event_name Name of the event.
     *  @param payload Payload to pass to the event listeners.
     *  @param payload_size Size of the payload.
     *  @return True if the event was posted, false otherwise.
     */
    bool (*post_event)(STRING_PARAM(event_name), void* payload, size_t payload_size);

    /*! Dispatch a callback that will be invoked on the main thread.
     *
     *  @param callback Callback to invoke.
     *  @param user_data User data to pass to the callback.
     */
    void (*call_delayed)(api_callback_t callback, user_data_t user_data);
};

struct api_system_t {};
struct api_expression_t {};

/*! Structure used to access the plugin API. */
struct api_interface_t
{
    api_app_t* app{ nullptr };
    api_log_t* log{ nullptr };
    api_dispatcher_t* dispatcher{ nullptr };
    api_expression_t* expression{ nullptr };
    api_system_t* system{ nullptr };
};

/*! Structure used to register callbacks for the plugin. */
struct api_plugin_callbacks_t
{
    void (*update)() { nullptr };
    void (*render)() { nullptr };
};

struct api_plugin_context_t;

/*! Plugin API initialization structure */
typedef void (*initialize_apis_fn)(const api_plugin_context_t* context, api_interface_t** api);

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

    /*! Plugin callbacks that the system invokes at certain times.
     *
     *  The callbacks will mainly be used for realtime events. Otherwise you can use the dispatcher
     *  to post events and register listeners.
     */
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
