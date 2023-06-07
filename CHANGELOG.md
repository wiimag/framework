# Changelog

## [1.2.0] - 2023-06-07
- Add `*.expr` source files to generated solution.
- Add `config.h/cpp` tests
- Add `dispatcher_thread_signal` to signal a thread in order to exit or to resume.
- Add `environment_username` to get the current user name.
- Add `FETCH(...)` expression to fetch arbitrary data from the backend. (i.e. `COUNT(MAP(FETCH("exchange-symbol-list", "TO"), INDEX($1, 1)))`)
- Add `FORMAT` expression to format expression strings using string templates.
- Add `LPAD(...)` and `RPAD(...)` functions to pad a string with a given character.
- Add `NextPowerOf2` to get the compile time next power of 2 of a given number.
- Add `string_table_decode` to decode a string symbol into a user buffer.
- Add `string_to_version_short` to convert a version string to a short version string.
- Add `string_utf8_from_code_point` to convert a code point to a UTF-8 string.
- Add `TABLE(...)` custom type drawers (i.e. `TABLE('Title', $symbols, ['Symbol', $1, symbol])`) 
- Add basic shared pointer support.
- Add better exception handling when executing client code.
- Add build switch `BUILD_ENABLE_DEVELOPMENT`
- Add build switch `BUILD_MAX_JOB_THREADS`
- Add link to web site in the help menu.
- Add many new built-in expression functions (see <https://wallet.wiimag.com/manual/en/expressions.md#basic-functions>)
- Add new string template formatters (see <https://wallet.wiimag.com/manual/en/expressions.md#formatters>)
- Add support for `YEAR(...)` to get the year of a date. This was useful to plot annual earnings using `PLOT(...)`.
- Add support for application input text dialogs.
- Add support support to set custom HTTP headers when executing a query.
- Add support to delete old search documents using a timestamp.
- Add support to evaluation expression files from the console using `@` followed by a valid file path, i.e. `@C:\...\expressions\report_slope_down.expr`.
- Add support to export `table` content to CSV.
- Add support to have a special row in tables to insert new rows (see `TABLE_ADD_NEW_ROW`)
- Add support to localize long date format.
- Add support to open dialog windows for the current active floating window (instead of always the main one).
- Add support to re-use deleted document slots for a search database.
- Fix `MAP(...)` to set the default comparison index to the last index of the array.
- Fix `url_encode` for some characters.
- Fix crash when closing a floating window.
- Fix day name french translations.
- Fix infinite loop with rare cases in `string_table_rebuild_hash_table`.
- Fix many minor issues with the `config` module API.
- Fix MSI package launching when looking for updates.
- Fix opening URL links on MacOS.
- Fix table currency value rounding.
- Fix timestamp parsing on MacOS.
- Fix window opened outside of monitor work area.
- Improve `TABLE(...)` and `PLOT(...)` expressions.
- Improve installation and update process using WiX Toolset.
- Remove comments in console before evaluating the expression.
- Update BGFX build options (see `external\bgfx\CMakeLists.txt`)

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

- [ ] 2023-04-17: Fix issue where minimizing the app and restoring it change it from monitor.
- [ ] 2023-04-16: Add support for Linux
