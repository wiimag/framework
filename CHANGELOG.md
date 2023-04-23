# Changelog

## [1.1.2] - 2023-04-23
- Add `system_save_file_dialog` on OSX
- Fix `build_setting` bash function on OSX
- Fix `MEM_DELETE` when pointer is null
- Fix OSX compilation issues
- Improve `./run` script
- Remove `FOUNDATION_CONSTCALL` usage which is causing issues on OSX

## [1.1.1] - 2023-04-22
- Add `./run package` (through `build-package.sh`) script to build and package the library for distribution.
- Add `BUILD_ENABLE_BACKEND` build switch
- Add `BUILD_ENABLE_TESTS` build switch
- Add `ImGui::CenteredButton` helper function to have a button centered in a window.
- Add `memory_allocate` and `memory_temporary` template helpers to allocate memory on the stack or in the heap.
- Add `system_save_file_dialog` to open a save file dialog (only on Windows for now).
- Add `window_valid` to check if a window handle is valid (but not necessarily the same expected window).
- Add ImPlot translation support
- Add installer skeleton code (not finished yet).
- Add missing PDBs for release builds.
- Add support to render custom table headers (See `table_render_column_header`).
- Fix some expression parsing issues and error reporting.
- Fix tabbing after changing the active tab the first time.
- Use `BUILD_TESTS` to enable/disable test code

## [1.1.0] - 2023-04-17
- Add markdown files to generate solution under extras/docs
- Add PRODUCT_* and VERSION_* variables in build.settings to consumed by CMake at build time
- Fail if user try to run tests in deploy mode

## [1.0.0] - 2023-04-16
- First release

# TODO

- [ ] 2023-04-22: Implement `system_save_file_dialog` on OSX
- [ ] 2023-04-17: Fix issue where minimizing the app and restoring it change it from monitor.
- [ ] 2023-04-16: Add support for Linux
