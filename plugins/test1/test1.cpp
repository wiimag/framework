/*
 * Copyright 2023 - All rights reserved.
 * License: https://equals-forty-two.com/LICENSE
 * 
 * Test1 plugin example implementation.
 *
 * Plugins can use the foundation_lib and/or IMGUI exported functions, but that is not mandatory.
 */

#include <api.h>

#include <foundation/hash.h>

#include <imgui/imgui.h>

static int _update_counter = 0;
static bool _window_opened = true;
static api_interface_t* api = nullptr;

#define HASH_PLUGIN_TEST1 static_hash_string("plugin_test1", 12, 0xc2f1f9448a0499bdULL)

FOUNDATION_STATIC void test1_plugin_menu(void* context)
{
    FOUNDATION_UNUSED(context);
    api->log->info(HASH_PLUGIN_TEST1, STRING_CONST("test1_plugin_menu"));
    api->log->warning(HASH_PLUGIN_TEST1, STRING_CONST("test1_plugin_menu"));
    api->log->error(HASH_PLUGIN_TEST1, ERROR_NONE, STRING_CONST("test1_plugin_menu"));
}

FOUNDATION_STATIC void test1_plugin_crash(void* context)
{
    int* p = (int*)context;
    *p = 0;
}

FOUNDATION_STATIC void test1_plugin_update()
{
    ++_update_counter;
}

FOUNDATION_STATIC void test1_plugin_render()
{
    if (!_window_opened)
        return;

    if (!ImGui::Begin("Plugin test1", &_window_opened))
        return ImGui::End();
    
    ImGui::Text("Hello App, I am a plugin! (%d)", _update_counter);

    if (ImGui::Button("Crash"))
        test1_plugin_crash(nullptr);
    ImGui::End();
}

FRAMEWORK_API_EXPORT int load_plugin(api_plugin_context_t* plugin)
{
    // Initialize plugin info
    plugin->context = HASH_PLUGIN_TEST1; plugin->version_major = 1; plugin->version_minor = 0;
    string_copy(STRING_BUFFER(plugin->name), STRING_CONST("test1"));

    // Load API interface
    plugin->initialize_apis(plugin, &api);

    // Set some callbacks that the application will invoke on the plugin
    plugin->callbacks.render = test1_plugin_render;
    plugin->callbacks.update = test1_plugin_update;

    // Use the API to do things
    api->app->register_menu(STRING_CONST("Plugins/test1"), nullptr, 0, APP_MENU_NONE, test1_plugin_menu, nullptr);
    api->app->register_menu(STRING_CONST("Plugins/crash"), nullptr, 0, APP_MENU_NONE, test1_plugin_crash, nullptr);

    return 0;
}

FRAMEWORK_API_EXPORT void unload_plugin()
{
    api = nullptr;
}
