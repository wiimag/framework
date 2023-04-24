/*
 * Copyright 2022-2023 - All rights reserved.
 * License: https://equals-forty-two.com/LICENSE
 */


#include "settings.h"

#include <framework/app.h>
#include <framework/glfw.h>
#include <framework/query.h>
#include <framework/version.h>

#include <foundation/foundation.h>

/*! Main entry point used to report the application title.
 *
 *  @return The application title.
 */
extern const char* app_title()
{
    return PRODUCT_NAME;
}

/*! Main entry point to configure the application.
 *
 *  This function is called once when the application is starting up.
 *
 *  @remark This function is called before #app_initialize.
 *
 *  @param config     The application configuration.
 *  @param application The application information.
 */
extern void app_configure(foundation_config_t& config, application_t& application)
{
    #if BUILD_APPLICATION
    application.flags = APPLICATION_GUI;
    #else
    application.flags = APPLICATION_DAEMON;
    #endif

    application.name = string_const(PRODUCT_NAME, string_length(PRODUCT_NAME));
    application.short_name = string_const(PRODUCT_CODE_NAME, string_length(PRODUCT_CODE_NAME));
    application.company = string_const(STRING_CONST(PRODUCT_COMPANY));
    application.version = version_make(VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH, VERSION_BUILD, 0);
    application.exception_handler = app_exception_handler;
}

/*! Main entry point to initialize the application.
 *
 *  In this function you should initialize all the services that you intend to use in the application.
 *
 *  @param window The GLFW window handle.
 *
 *  @return 0 if initialization was successful, otherwise an error code.
 */
extern int app_initialize(GLFWwindow* window)
{
    // Framework systems
    string_table_initialize();
    progress_initialize();    
    jobs_initialize();
    session_setup(nullptr);
    query_initialize();
    
    // App systems
    settings_initialize();
    module_initialize();

    return 0;
}

/*! Main entry point to shutdown the application.
 *
 *  This function is called once when the application is shutting down, and should be used to
 *  shutdown all initialized services and modules.
 */
extern void app_shutdown()
{
    dispatcher_update();
    dispatcher_poll(nullptr);
        
    // Lets make sure all requests are finished 
    // before exiting shutting down other services.
    jobs_shutdown();
    query_shutdown();
    
    // App systems
    module_shutdown();
    settings_shutdown();
    
    // Framework systems
    tabs_shutdown();
    progress_finalize();
    session_shutdown();
    string_table_shutdown();
}

/*! Main entry point to update the application state.
 *
 *  This function is called once per frame, and should be used to update
 *  the application state. This includes updating the application logic,
 *  updating the UI, and updating the application state.
 *
 *  @param window The GLFW window handle.
 */
extern void app_update(GLFWwindow* window)
{
    app_update_default(window);
}

/*! Main entry point to render the application state.
 *
 *  This function is called once per frame, and should be used to render using IMGUI
 *  the application state.
 *
 *  @remark By default no IMGUI window is created, and the application is expected to
 *          create its embedded window using ImGui::Begin/End.
 *
 *  @param window       The main GLFW window handle.
 *  @param frame_width  The width of the frame buffer.
 *  @param frame_height The height of the frame buffer.
 */
extern void app_render(GLFWwindow* window, int frame_width, int frame_height)
{
    constexpr auto default_tab_render = []()
    {
        ImGui::TrTextUnformatted("Hello World!");
    };

    app_render_default(window, frame_width, frame_height, SETTINGS.current_tab, default_tab_render, settings_draw);
}
