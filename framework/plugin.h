/*
 * Copyright 2023 - All rights reserved.
 * License: https://equals-forty-two.com/LICENSE
 * 
 * Plugin interface
 */

#pragma once

#include <foundation/types.h>

/*! Define an invalid plugin handle value. */
constexpr const uint32_t PLUGIN_HANDLE_INVALID = UINT32_MAX;

/*! Plugin handle. */
struct plugin_handle_t { uint32_t index{ PLUGIN_HANDLE_INVALID }; }; 

/*! Load a plugin shared library. 
 *
 *  @param path   Path to plugin.
 *  @param length Length of path.
 *
 *  @return Plugin handle if loaded successfully, otherwise PLUGIN_HANDLE_INVALID.
 */
plugin_handle_t plugin_load(const char* path, size_t length);

/*! Unload plugin shared library.
 *
 *  @param plugin Plugin handle.
 */
void plugin_unload(plugin_handle_t plugin);

/*! Returns how many plugins are loaded.
 *
 *  @return Number of plugins.
 */
unsigned plugin_count();

/*! Application entry point to lets plugin render any ImGui windows.
 */
void plugin_render();

/*! Application entry point to let plugins update their internal state.
 */
void plugin_update();
