/*
 * Copyright 2022-2023 - All rights reserved.
 * License: https://equals-forty-two.com/LICENSE
 */

#include "version.h"
#include "settings.h"

#include <framework/app.h>
#include <framework/glfw.h>
#include <framework/query.h>

#include <foundation/foundation.h>

/*! Render leading menus. 
 *
 *  @param window The GLFW window.
 */
FOUNDATION_STATIC void app_main_menu_begin(GLFWwindow* window)
{
    if (!ImGui::BeginMenuBar())
        return;

    if (ImGui::TrBeginMenu("File"))
    {
        if (ImGui::TrBeginMenu("Create"))
        {
            ImGui::EndMenu();
        }

        if (ImGui::TrBeginMenu("Open"))
        {
            ImGui::EndMenu();
        }

        ImGui::Separator();
        if (ImGui::TrMenuItem(ICON_MD_EXIT_TO_APP " Exit", "Alt+F4"))
            glfw_request_close_window(window);
            
        ImGui::EndMenu();
    }

    ImGui::EndMenuBar();

    // Let the framework inject some menus.
    app_menu_begin(window);
}

/*! Render trailing menus. 
 *
 *  In between leading and trailing menus the framework usually 
 *  injects additional menus through registered modules.
 *
 *  @param window The GLFW window.
 */
FOUNDATION_STATIC void app_main_menu_end(GLFWwindow* window)
{
    // Let registered module inject some menus.
    module_foreach_menu();

    if (ImGui::BeginMenuBar())
    {
        if (ImGui::TrBeginMenu("Windows"))
            ImGui::EndMenu();
            
        app_menu_help(window);

        // Update special application menu status.
        // Usually controls are displayed at the far right of the menu.
        profiler_menu_timer();
        module_foreach_menu_status();

        ImGui::EndMenuBar();
    }

    app_menu_end(window);
}

/*! Main entry point used to report the application title.
 *
 *  @return The application title.
 */
extern const char* app_title()
{
    return PRODUCT_NAME;
}

/*! Main entry point to setup the application exception handler.
 *
 *  You can use this function to setup a custom exception handler to log and report crashes.
 *
 *  @param dump_file The name of the dump file.
 *  @param length    The length of the dump file name.
 *
 *  @note The dump file is a binary file containing the application state at the time of the crash.
 */
extern void app_exception_handler(const char* dump_file, size_t length)
{
    FOUNDATION_UNUSED(dump_file);
    FOUNDATION_UNUSED(length);
    log_error(0, ERROR_EXCEPTION, STRING_CONST("Unhandled exception"));
    process_exit(-1);
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
    module_update();
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
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImVec2((float)frame_width, (float)frame_height));

    if (ImGui::Begin(app_title(), nullptr,
        ImGuiWindowFlags_NoBringToFrontOnFocus |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_MenuBar))
    {
        // Render main menus
        app_main_menu_begin(window);

        // Render document tabs
        static ImGuiTabBarFlags tabs_init_flags = ImGuiTabBarFlags_Reorderable;
        if (tabs_begin("Tabs", SETTINGS.current_tab, tabs_init_flags, nullptr))
        {
            // Render the settings tab
            tab_set_color(TAB_COLOR_APP_3D);
            tab_draw(tr(ICON_MD_HEXAGON " Example "), nullptr, 0, []()
            {
                ImGui::TrTextUnformatted("Hello World!");
            });

            // Render module registered tabs
            module_foreach_tabs();

            // Render the settings tab
            tab_set_color(TAB_COLOR_SETTINGS);
            tab_draw(tr(ICON_MD_SETTINGS " Settings ##Settings"), nullptr,
                ImGuiTabItemFlags_NoPushId | ImGuiTabItemFlags_Trailing | ImGuiTabItemFlags_NoReorder, settings_draw);

            // We differ setting ImGuiTabBarFlags_AutoSelectNewTabs until after the first frame,
            // since we manually select the first tab in the list using the user session data.
            if ((tabs_init_flags & ImGuiTabBarFlags_AutoSelectNewTabs) == 0)
                tabs_init_flags |= ImGuiTabBarFlags_AutoSelectNewTabs;

            tabs_end();
        }

        // Render trailing menus
        app_main_menu_end(window);

        // Render main dialog and floating windows
        module_foreach_window();

    } ImGui::End();
}

/*! Main entry point to additional 3rdparty library information 
 *  displayed in the default about dialog.
 */
extern void app_render_3rdparty_libs()
{
    // HERE: Invoke here any function that wants to render 3rd party libs information
}
