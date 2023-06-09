/*
 * Copyright 2022-2023 - All rights reserved.
 * License: https://wiimag.com/LICENSE
 * 
 * Application module management.
 * 
 * A module is meant to be standalone and self-contained as much as possible.
 */

#pragma once

#include <framework/string.h>
#include <framework/common.h>

#include <foundation/hash.h>

typedef void (*module_initialize_handler_t)(void);
typedef void (*module_shutdown_handler_t)(void);

typedef function<void()> module_invoke_handler_t;

constexpr int MODULE_PRIORITY_CRITICAL    = (-100);
constexpr int MODULE_PRIORITY_SYSTEM      =  (-20);
constexpr int MODULE_PRIORITY_REALTIME    =  (-10);
constexpr int MODULE_PRIORITY_BASE        =    (0);
constexpr int MODULE_PRIORITY_HIGH        =   (10);
constexpr int MODULE_PRIORITY_MODULE      =   (20);
constexpr int MODULE_PRIORITY_LOW         =   (30);
constexpr int MODULE_PRIORITY_TESTS       =  (100);
constexpr int MODULE_PRIORITY_UI_HEADLESS =  (190);
constexpr int MODULE_PRIORITY_UI          =  (200);

FOUNDATION_FORCEINLINE const char* module_name_to_lower_static(const char* name, size_t len)
{
    static thread_local char ts_name_buffer[16];
    string_t lname = string_to_lower_ascii(STRING_BUFFER(ts_name_buffer), name, len);
    return lname.str;
}

/*! @def DEFINE_MODULE
 * 
 * Register a module to be initialized and shutdown at the appropriate time.
 *
 * @param NAME          Name of the module/service.
 * @param initialize_fn Function to call to initialize the service.
 * @param shutdown_fn   Function to call to shutdown the service.
 * @param ...           Optional priority of the service.
 */
#define DEFINE_MODULE(NAME, initialize_fn, shutdown_fn, ...)    \
    const Module __##NAME##_service(#NAME, HASH_##NAME, [](){   \
        memory_context_push(HASH_##NAME);                       \
        initialize_fn();                                        \
        memory_context_pop();                                   \
    }, shutdown_fn, __VA_ARGS__)

 /*! Service object to be instantiated in the global scope to manage 
  *  the service initialization and shutdown sequence. 
  */
class Module
{
public:
    FOUNDATION_NOINLINE Module(const char* FOUNDATION_RESTRICT name, hash_t module_hash,
        module_initialize_handler_t initialize_handler,
        module_shutdown_handler_t shutdown_handler,
        int priority);

    FOUNDATION_NOINLINE Module(const char* FOUNDATION_RESTRICT name, hash_t module_hash,
        module_initialize_handler_t initialize_handler,
        module_shutdown_handler_t shutdown_handler)
        : Module(name, module_hash, initialize_handler, shutdown_handler, MODULE_PRIORITY_LOW)
    {
    }
};

/*! Initialize the service system and all statically registered services. */
void module_initialize();

/*! Shutdown the service system and all other registered services. */
void module_shutdown();

/*! Register a service handler that can be invoked for all services later on.
 * 
 * @param service_key Hash key of the registered service.
 * @param handler_key Hash key for the handler.
 * @param handler Handler function to invoke. 
 */
void module_register_handler(hash_t service_key, hash_t handler_key, const module_invoke_handler_t& handler);

/*! Register a service to act to menu events.
 * 
 * @param service_key Hash key of the registered service.
 * @param menu_handler Handler function to invoke.
 */
void module_register_menu(hash_t service_key, const module_invoke_handler_t& menu_handler);

/*! Register a service to act to menu status events.
 *
 * @param service_key Hash key of the registered service.
 * @param menu_status_handler Handler function to invoke.
 */
void module_register_menu_status(hash_t service_key, const module_invoke_handler_t& menu_status_handler);

/*! Register a service to act to tab events.
 *
 * @param service_key Hash key of the registered service.
 * @param tabs_handler Handler function to invoke.
 */
void module_register_tabs(hash_t service_key, const module_invoke_handler_t& tabs_handler);

/*! Register a service to render new windows.
 *
 * @param service_key Hash key of the registered service.
 * @param window_handler Handler function to invoke.
 */
void module_register_window(hash_t service_key, const module_invoke_handler_t& window_handler);

/*! Register a service to update the application.
 *
 * @param service_key Hash key of the registered service.
 * @param update_handler Handler function to invoke.
 */
void module_register_update(hash_t service_key, const module_invoke_handler_t& update_handler);

/*! Loop through all services to invoke the registered handler.
 *
 * @param handler_key Hash key of the service handler to invoke.
 */
void module_foreach(hash_t handler_key);

/*! Loop through all services to invoke the registered menu handler. */
void module_foreach_menu();

/*! Loop through all services to invoke the registered menu status handler. */
void module_foreach_menu_status();

/*! Loop through all services to invoke the registered tabs handler. */
void module_foreach_tabs();

/*! Loop through all services to invoke the registered window handler. */
void module_foreach_window();

/*! Loop through all services to invoke the registered update handler. */
void module_update();
