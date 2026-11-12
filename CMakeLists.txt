cmake_minimum_required(VERSION 3.18)
project(ImGuiX LANGUAGES C CXX VERSION 0.1.0)

# ===== Root CMakeLists.txt =====
# Purpose: Configure and build the ImGuiX library and optional SDK.
# Inputs:  IMGUIX_* options and IMGUIX_DEPS_* modes, IMGUI_DIR
# Outputs: target imguix and export ImGuiXTargets
# Notes:   Select backend via IMGUIX_USE_*_BACKEND options.

# Normalize REAL_PATH behavior so MSYS/Windows paths resolve to native form first
if(POLICY CMP0152)
    cmake_policy(SET CMP0152 NEW)
endif()

# ===== Options =====
option(IMGUIX_HEADER_ONLY      "Header-only mode (no .cpp compiled)" OFF)
option(IMGUIX_BUILD_SHARED     "Build ImGuiX as shared library" OFF)
option(IMGUIX_BUILD_TESTS      "Build tests in tests/ folder"   ON)
option(IMGUIX_BUILD_EXAMPLES   "Build examples in examples/ folder" ON)

# Backend selection
option(IMGUIX_USE_SFML_BACKEND "Use SFML backend via ImGui-SFML" ON)
option(IMGUIX_USE_GLFW_BACKEND "Use GLFW backend" OFF)
option(IMGUIX_USE_SDL2_BACKEND "Use SDL2 backend" OFF)

# ImGui / FreeType
option(IMGUIX_IMGUI_FREETYPE   "Enable ImGui FreeType rasterizer" ON)

# ImGui / ImPlot, ImPlot3D
option(IMGUIX_USE_IMPLOT       "Build ImPlot and link it in" ON)
option(IMGUIX_USE_IMPLOT3D     "Build ImPlot3D and link it in" ON)

# ImGui / ImNodeFlow
option(IMGUIX_USE_IMNODEFLOW   "Build and link ImNodeFlow" ON)

# ImGui / ImGuiFileDialog
option(IMGUIX_USE_IMGUIFILEDIALOG        "Build and link ImGuiFileDialog" ON)
option(IMGUIX_IMGUIFILEDIALOG_USE_STD_FS "Use std::filesystem in ImGuiFileDialog" ON)

# ImGui / ImGuiColorTextEdit
option(IMGUIX_USE_IMTEXTEDITOR "Build and link ImGuiColorTextEdit" ON)

# portable-file-dialog
option(IMGUIX_USE_PFD          "Build and link portable-file-dialogs (header-only)" ON)

# ImGui / imgui-command-palette
option(IMGUIX_USE_IMCMD        "Build and link imgui-command-palette" ON)

# ImGui / ImCoolBar
option(IMGUIX_USE_IMCOOLBAR    "Build and link ImCoolBar" ON)

# ImGui / imspinner
option(IMGUIX_USE_IMSPINNER    "Build and link imspinner (header-only)" ON)

# ImGui / imgui_md
option(IMGUIX_USE_IMGUI_MD      "Build and link imgui_md (Markdown)" ON)
# Собирать md4c через add_subdirectory (их CMake), или напрямую из md4c.c
option(IMGUIX_MD4C_USE_SUBDIR   "Use md4c's own CMake (add_subdirectory) instead of compiling md4c.c directly" OFF)

# JSON vendoring (default OFF to prefer system package)
option(IMGUIX_VENDOR_JSON      "Install vendored nlohmann_json headers with ImGuiX" OFF)

option(IMGUIX_SDK_FLATTEN_MISC_HEADERS
    "Also copy imgui_stdlib.h and imgui_freetype.h into include/ root" ON)

# SDK
# Install SDK extras (quickstart, templates, assets)
option(IMGUIX_SDK_INSTALL      "Install SDK extras (quickstart, templates, assets)" ON)
option(IMGUIX_SDK_BUNDLE_DEPS  "Install 3rd-party deps into SDK stage" ON)
# Place quickstart template into SDK
option(IMGUIX_SDK_INSTALL_QUICKSTART "Install quickstart template into SDK" ON)

# ===== Deps mode =====
set(IMGUIX_DEPS_MODE "AUTO" CACHE STRING "AUTO|SYSTEM|BUNDLED")
set_property(CACHE IMGUIX_DEPS_MODE PROPERTY STRINGS AUTO SYSTEM BUNDLED)
set(IMGUIX_DEPS_FMT_MODE        "INHERIT" CACHE STRING "INHERIT|AUTO|SYSTEM|BUNDLED")
set(IMGUIX_DEPS_SFML_MODE       "INHERIT" CACHE STRING "INHERIT|AUTO|SYSTEM|BUNDLED")
set(IMGUIX_DEPS_IMGUI_MODE      "INHERIT" CACHE STRING "INHERIT|AUTO|SYSTEM|BUNDLED")
set(IMGUIX_DEPS_IMGUI_SFML_MODE "INHERIT" CACHE STRING "INHERIT|AUTO|SYSTEM|BUNDLED")
set(IMGUIX_DEPS_FREETYPE_MODE   "INHERIT" CACHE STRING "INHERIT|AUTO|SYSTEM|BUNDLED")
set(IMGUIX_DEPS_JSON_MODE       "INHERIT" CACHE STRING "INHERIT|AUTO|SYSTEM|BUNDLED")

# ===== C++ =====
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

if(EMSCRIPTEN)
    add_link_options(-sASYNCIFY)
    add_compile_definitions(IMGUIX_ASYNCIFY)
endif()

set(IMGUIX_USER_CONFIG_DIR  "${PROJECT_SOURCE_DIR}/include")
set(IMGUIX_USER_CONFIG_NAME "imguix/config/imconfig-imguix.hpp")

# ===== Module path =====
list(PREPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

# --- std::string helpers for ImGui (imgui_stdlib) ---
# If user does not set the option, choose default:
# - with SFML backend: OFF (ImGui-SFML already provides imgui_stdlib.cpp)
# - otherwise: ON
if(NOT DEFINED IMGUIX_IMGUI_STDLIB)
    set(_imgui_stdlib_default ON)
    if(IMGUIX_USE_SFML_BACKEND)
        set(_imgui_stdlib_default OFF)
    endif()
    set(IMGUIX_IMGUI_STDLIB ${_imgui_stdlib_default}
        CACHE BOOL "Build ImGui std::string helpers (imgui_stdlib.cpp) for non-SFML backends")
endif()

if(IMGUIX_USE_SFML_BACKEND AND IMGUIX_IMGUI_STDLIB)
    message(STATUS "IMGUIX_USE_SFML_BACKEND=ON: forcing IMGUIX_IMGUI_STDLIB=OFF to avoid duplicate imgui_stdlib.cpp")
    set(IMGUIX_IMGUI_STDLIB OFF CACHE BOOL "" FORCE)
endif()

# ===== Deps (modular) =====
include(cmake/deps/fmt.cmake)
include(cmake/deps/freetype.cmake)
include(cmake/deps/sfml.cmake)
include(cmake/deps/imgui.cmake)
include(cmake/deps/imgui_sfml.cmake)
include(cmake/deps/implot.cmake)
include(cmake/deps/implot3d.cmake)
include(cmake/deps/imnodeflow.cmake)
include(cmake/deps/imgui_file_dialog.cmake)
include(cmake/deps/imtexteditor.cmake)
include(cmake/deps/portable_file_dialogs.cmake)
include(cmake/deps/imcmd.cmake)
include(cmake/deps/imcoolbar.cmake)
include(cmake/deps/imspinner.cmake)
include(cmake/deps/imgui_md.cmake)
include(cmake/deps/nlohmann_json.cmake)
include(cmake/deps/assets.cmake)
include(cmake/deps/app_icon.cmake)
include(cmake/ImGuiXDemos.cmake)

# ===== Resolve core deps =====
imguix_use_or_fetch_fmt(FMT_TARGET)                 # Expect fmt::fmt
imguix_use_or_fetch_nlohmann_json(JSON_TARGET)      # Always nlohmann_json::nlohmann_json or alias

# ===== Resolve GUI/backend deps =====
set(BACKEND_LIBS "")
set(IMGUI_LIB imgui::imgui)

if(IMGUIX_USE_SFML_BACKEND)
    imguix_use_or_fetch_sfml(SFML_TARGETS)          # SFML::Graphics;SFML::Window;SFML::System
    imguix_use_or_fetch_imgui_sfml(IMGUI_SFML_TARGET IMGUI_TARGET_OUT)
    set(IMGUI_LIB "${IMGUI_TARGET_OUT}")            # ImGui-SFML
    list(APPEND BACKEND_LIBS ${SFML_TARGETS})
elseif(IMGUIX_USE_GLFW_BACKEND)
    message(STATUS "GLFW backend selected (add deps/glfw.cmake if needed)")
elseif(IMGUIX_USE_SDL2_BACKEND)
    message(STATUS "SDL2 backend selected (add deps/sdl2.cmake if needed)")
else()
    message(STATUS "No GUI backend selected; using pure Dear ImGui")
    imguix_use_or_fetch_imgui(IMGUI_TARGET)
    set(IMGUI_LIB "${IMGUI_TARGET}")
endif()

# --- Select backend define ---
set(_IMGUIX_BACKEND_DEFINE "")
if(IMGUIX_USE_SFML_BACKEND)
    set(_IMGUIX_BACKEND_DEFINE IMGUIX_USE_SFML_BACKEND)
elseif(IMGUIX_USE_GLFW_BACKEND)
    set(_IMGUIX_BACKEND_DEFINE IMGUIX_USE_GLFW_BACKEND)
elseif(IMGUIX_USE_SDL2_BACKEND)
    set(_IMGUIX_BACKEND_DEFINE IMGUIX_USE_SDL2_BACKEND)
endif()

if(IMGUIX_USE_IMPLOT)
    imguix_use_or_fetch_implot(IMPLOT_TARGET)
endif()

if(IMGUIX_USE_IMPLOT3D)
    imguix_use_or_fetch_implot3d(IMPLOT3D_TARGET)
endif()

if(IMGUIX_USE_IMNODEFLOW)
    imguix_use_or_fetch_imnodeflow(IMNODEFLOW_TARGET)
endif()

if(IMGUIX_USE_IMGUIFILEDIALOG)
    imguix_use_or_fetch_imguifiledialog(IGFD_TARGET)
endif()

if(IMGUIX_USE_IMTEXTEDITOR)
    imguix_use_or_fetch_imtexteditor(IMTEXTEDITOR_TARGET)
endif()

if(IMGUIX_USE_PFD)
    imguix_use_or_fetch_pfd(PFD_TARGET)
endif()

if(IMGUIX_USE_IMCMD)
    imguix_use_or_fetch_imcmd(IMCMD_TARGET)
endif()

if(IMGUIX_USE_IMCOOLBAR)
    imguix_use_or_fetch_imcoolbar(IMCOOLBAR_TARGET)
endif()

if(IMGUIX_USE_IMSPINNER)
    imguix_use_or_fetch_imspinner(IMSPINNER_TARGET)
endif()

if(IMGUIX_USE_IMGUI_MD)
    imguix_use_or_fetch_imgui_md(IMGUI_MD_TARGET)
endif()

# ===== Library / Header-only switch =====
if(IMGUIX_HEADER_ONLY)
    message(STATUS "ImGuiX: header-only mode ON")
    add_library(imguix INTERFACE)
    add_library(ImGuiX::imguix ALIAS imguix)

    # BUILD_INTERFACE: headers from source tree during build
    # INSTALL_INTERFACE: headers exposed after installation
    target_include_directories(imguix INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>
        $<INSTALL_INTERFACE:include>
    )

    # Allow user configuration overrides via IMGUI_USER_CONFIG
    target_include_directories(imguix INTERFACE "${IMGUIX_USER_CONFIG_DIR}")
    target_compile_definitions(imguix INTERFACE IMGUI_USER_CONFIG="${IMGUIX_USER_CONFIG_NAME}")

    # Variant B: when vendoring, add json headers to build include path
    if(IMGUIX_VENDOR_JSON)
        target_include_directories(imguix INTERFACE
            $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/libs/json/include>
        )
    endif()

    target_compile_definitions(imguix INTERFACE IMGUIX_HEADER_ONLY)

    if(_IMGUIX_BACKEND_DEFINE)
        target_compile_definitions(imguix INTERFACE ${_IMGUIX_BACKEND_DEFINE})
    endif()

    if(IMGUIX_IMGUI_FREETYPE)
        target_compile_definitions(imguix INTERFACE IMGUI_ENABLE_FREETYPE)
        # Do not link freetype/SFML/ImGui-SFML in INTERFACE; consumers link them
    endif()

    if(IMGUIX_USE_IMPLOT)
        target_compile_definitions(imguix INTERFACE IMGUIX_ENABLE_IMPLOT)
        target_link_libraries(imguix INTERFACE ${IMPLOT_TARGET})
    endif()

    if(IMGUIX_USE_IMPLOT3D)
        target_compile_definitions(imguix INTERFACE IMGUIX_ENABLE_IMPLOT3D)
        target_link_libraries(imguix INTERFACE ${IMPLOT3D_TARGET})
    endif()
    
    if(IMGUIX_USE_IMNODEFLOW)
        target_compile_definitions(imguix INTERFACE IMGUIX_ENABLE_IMNODEFLOW)
    endif()
    
    if(IMGUIX_USE_IMGUIFILEDIALOG)
        target_compile_definitions(imguix INTERFACE IMGUIX_ENABLE_IMGUIFILEDIALOG)
    endif()
    
    if(IMGUIX_USE_IMTEXTEDITOR)
        target_compile_definitions(imguix INTERFACE IMGUIX_ENABLE_IMTEXTEDITOR)
    endif()
    
    if(IMGUIX_USE_PFD)
        target_compile_definitions(imguix INTERFACE IMGUIX_ENABLE_PFD)
        # include-dir пойдёт транзитивно у потребителей, если они залинкуются на pfd::pfd
    endif()
    
    if(IMGUIX_USE_IMCMD)
        target_compile_definitions(imguix INTERFACE IMGUIX_ENABLE_IMCMD)
    endif()
    
    if(IMGUIX_USE_IMCOOLBAR)
        target_compile_definitions(imguix INTERFACE IMGUIX_ENABLE_IMCOOLBAR)
    endif()
    
    if(IMGUIX_USE_IMSPINNER)
        target_compile_definitions(imguix INTERFACE IMGUIX_ENABLE_IMSPINNER)
        # include-dir пойдёт транзитивно, если потребитель линкуется на imspinner::imspinner
    endif()
    
    if(IMGUIX_USE_IMGUI_MD)
        target_compile_definitions(imguix INTERFACE IMGUIX_ENABLE_IMGUI_MD)
    endif()


else()
    set(_imguix_type STATIC)
    if(IMGUIX_BUILD_SHARED)
        set(_imguix_type SHARED)
    endif()

    add_library(imguix ${_imguix_type}
        src/imguix.cpp
    )
    add_library(ImGuiX::imguix ALIAS imguix)

    target_include_directories(imguix
        PUBLIC
            $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>
            $<INSTALL_INTERFACE:include>
    )

    if(WIN32 AND IMGUIX_BUILD_SHARED)
        set_target_properties(imguix PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS ON)
    endif()

    # Link external dependencies; expose fmt and json for public headers
    target_link_libraries(imguix
        PUBLIC fmt::fmt nlohmann_json::nlohmann_json
        PRIVATE ${IMGUI_LIB} ${BACKEND_LIBS}
    )

    # Ensure backend define participates in this target's compilation
    if(_IMGUIX_BACKEND_DEFINE)
        target_compile_definitions(imguix PUBLIC ${_IMGUIX_BACKEND_DEFINE})
    endif()

    if(IMGUIX_IMGUI_FREETYPE)
        target_compile_definitions(imguix PUBLIC IMGUI_ENABLE_FREETYPE)

        # Require include path for imgui_freetype.h
        set(_IMGUI_DIR_LOCAL "${IMGUI_DIR}")
        if(NOT _IMGUI_DIR_LOCAL)
            set(_IMGUI_DIR_LOCAL "${PROJECT_SOURCE_DIR}/libs/imgui")
        endif()
        target_include_directories(imguix
            PUBLIC
                $<BUILD_INTERFACE:${_IMGUI_DIR_LOCAL}/misc/freetype>
                $<INSTALL_INTERFACE:include>
        )

        # Resolve Dear ImGui root (same approach as for FreeType)
        set(_IMGUI_DIR_LOCAL "${IMGUI_DIR}")
        if(NOT _IMGUI_DIR_LOCAL)
            set(_IMGUI_DIR_LOCAL "${PROJECT_SOURCE_DIR}/libs/imgui")
        endif()

        # Require include path for imgui_stdlib.h (misc/cpp)
        target_include_directories(imguix
            PUBLIC
                $<BUILD_INTERFACE:${_IMGUI_DIR_LOCAL}/misc/cpp>
                $<INSTALL_INTERFACE:include>
        )

        imguix_use_or_find_freetype(FREETYPE_TARGET)
        target_link_libraries(imguix PRIVATE ${FREETYPE_TARGET})
    endif()

    # Variant B: when vendoring, add build include for json submodule
    if(IMGUIX_VENDOR_JSON)
        target_include_directories(imguix PRIVATE
            $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/libs/json/include>
        )
    endif()

    if(IMGUIX_USE_IMPLOT)
        target_compile_definitions(imguix PUBLIC IMGUIX_ENABLE_IMPLOT)
        target_link_libraries(imguix PUBLIC ${IMPLOT_TARGET})
    endif()

    if(IMGUIX_USE_IMPLOT3D)
        target_compile_definitions(imguix PUBLIC IMGUIX_ENABLE_IMPLOT3D)
        target_link_libraries(imguix PUBLIC ${IMPLOT3D_TARGET})
    endif()
    
    if(IMGUIX_USE_IMNODEFLOW)
        target_compile_definitions(imguix PUBLIC IMGUIX_ENABLE_IMNODEFLOW)
        target_link_libraries(imguix PRIVATE ${IMNODEFLOW_TARGET})
    endif()
    
    if(IMGUIX_USE_IMGUIFILEDIALOG)
        target_compile_definitions(imguix PUBLIC IMGUIX_ENABLE_IMGUIFILEDIALOG)
        target_link_libraries(imguix PRIVATE ${IGFD_TARGET})
    endif()
    
    if(IMGUIX_USE_IMTEXTEDITOR)
        target_compile_definitions(imguix PUBLIC IMGUIX_ENABLE_IMTEXTEDITOR)
        target_link_libraries(imguix PRIVATE ${IMTEXTEDITOR_TARGET})
    endif()
    
    if(IMGUIX_USE_PFD)
        target_compile_definitions(imguix PUBLIC IMGUIX_ENABLE_PFD)
        target_link_libraries(imguix PRIVATE ${PFD_TARGET})
    endif()
    
    if(IMGUIX_USE_IMCMD)
        target_compile_definitions(imguix PUBLIC IMGUIX_ENABLE_IMCMD)
        target_link_libraries(imguix PRIVATE ${IMCMD_TARGET})
    endif()
    
    if(IMGUIX_USE_IMCOOLBAR)
        target_compile_definitions(imguix PUBLIC IMGUIX_ENABLE_IMCOOLBAR)
        target_link_libraries(imguix PRIVATE ${IMCOOLBAR_TARGET})
    endif()
    
    if(IMGUIX_USE_IMSPINNER)
        target_compile_definitions(imguix PUBLIC IMGUIX_ENABLE_IMSPINNER)
        target_link_libraries(imguix PRIVATE ${IMSPINNER_TARGET})
    endif()
    
    if(IMGUIX_USE_IMGUI_MD)
        target_compile_definitions(imguix PUBLIC IMGUIX_ENABLE_IMGUI_MD)
        target_link_libraries(imguix PRIVATE ${IMGUI_MD_TARGET})
    endif()

endif()

# ===== Tests =====
if(IMGUIX_BUILD_TESTS)
    enable_testing()
    file(GLOB TESTS CONFIGURE_DEPENDS "${PROJECT_SOURCE_DIR}/tests/*.cpp")
    list(REMOVE_ITEM TESTS
        "${PROJECT_SOURCE_DIR}/tests/test_layout_primitives_multi_tu_support.cpp")

    imguix_stage_assets_once(
        NAME imguix_tests_assets
        DEST "${CMAKE_BINARY_DIR}/tests/data"
        DIRS "${PROJECT_SOURCE_DIR}/assets/data"
        EXCLUDE_DIRS web
    )

    set(_IMGUI_DIR "${IMGUI_DIR}")
    if(NOT _IMGUI_DIR)
        set(_IMGUI_DIR "${PROJECT_SOURCE_DIR}/libs/imgui")
    endif()

    foreach(TEST_FILE ${TESTS})
        get_filename_component(TEST_NAME "${TEST_FILE}" NAME_WE)
        add_executable(${TEST_NAME} "${TEST_FILE}")

        if(TEST_NAME STREQUAL "test_layout_primitives_header_only")
            target_compile_definitions(${TEST_NAME} PRIVATE IMGUIX_HEADER_ONLY)
            target_sources(${TEST_NAME} PRIVATE
                "${PROJECT_SOURCE_DIR}/tests/test_layout_primitives_multi_tu_support.cpp")
        endif()

        target_compile_definitions(${TEST_NAME} PRIVATE IMGUIX_DEMO)

        if(IMGUIX_USE_SFML_BACKEND)
            target_compile_definitions(${TEST_NAME} PRIVATE IMGUIX_USE_SFML_BACKEND)
        elseif(IMGUIX_USE_GLFW_BACKEND)
            target_compile_definitions(${TEST_NAME} PRIVATE IMGUIX_USE_GLFW_BACKEND)
        elseif(IMGUIX_USE_SDL2_BACKEND)
            target_compile_definitions(${TEST_NAME} PRIVATE IMGUIX_USE_SDL2_BACKEND)
        endif()

        target_include_directories(${TEST_NAME} PRIVATE
            "${CMAKE_CURRENT_LIST_DIR}/include"
        )
        # Provide access to imgui_stdlib.h
        target_include_directories(${TEST_NAME} PRIVATE 
            "${_IMGUI_DIR}/misc/cpp"
        )

        if(IMGUIX_IMGUI_FREETYPE)
            target_compile_definitions(${TEST_NAME} PRIVATE IMGUI_ENABLE_FREETYPE)
            target_include_directories(${TEST_NAME} PRIVATE
                "${_IMGUI_DIR}/misc/freetype"
            )
            imguix_use_or_find_freetype(FREETYPE_TARGET)
            target_link_libraries(${TEST_NAME} PRIVATE ${FREETYPE_TARGET})
        endif()

        target_link_libraries(${TEST_NAME} PRIVATE ImGuiX::imguix)
        target_link_libraries(${TEST_NAME} PRIVATE ${IMGUI_LIB} ${BACKEND_LIBS})
        target_link_libraries(${TEST_NAME} PRIVATE fmt::fmt nlohmann_json::nlohmann_json)

        if(IMGUIX_USE_IMPLOT)
            target_compile_definitions(${TEST_NAME} PRIVATE IMGUIX_ENABLE_IMPLOT)
            target_link_libraries(${TEST_NAME} PRIVATE implot::implot)
            imguix_add_implot_demo(${TEST_NAME})
        endif()
        
        if(IMGUIX_USE_IMPLOT3D)
            target_compile_definitions(${TEST_NAME} PRIVATE IMGUIX_ENABLE_IMPLOT3D)
            target_link_libraries(${TEST_NAME} PRIVATE implot3d::implot3d)
            imguix_add_implot3d_demo(${TEST_NAME})
        endif()
        
        if(IMGUIX_USE_IMNODEFLOW)
            target_compile_definitions(${TEST_NAME} PRIVATE IMGUIX_ENABLE_IMNODEFLOW)
            target_link_libraries(${TEST_NAME} PRIVATE imnodeflow::imnodeflow)
        endif()
        
        if(IMGUIX_USE_PFD)
            target_compile_definitions(${TEST_NAME} PRIVATE IMGUIX_ENABLE_PFD)
            target_link_libraries(${TEST_NAME} PRIVATE pfd::pfd)
        endif()
        
        if(IMGUIX_USE_IMGUIFILEDIALOG)
            target_compile_definitions(${TEST_NAME} PRIVATE IMGUIX_ENABLE_IMGUIFILEDIALOG)
            target_link_libraries(${TEST_NAME} PRIVATE imguifiledialog::imguifiledialog)
        endif()

        if(IMGUIX_USE_IMTEXTEDITOR)
            target_compile_definitions(${TEST_NAME} PRIVATE IMGUIX_ENABLE_IMTEXTEDITOR)
            target_link_libraries(${TEST_NAME} PRIVATE imtexteditor::imtexteditor)
        endif()
        
        if(IMGUIX_USE_IMCMD)
            target_compile_definitions(${TEST_NAME} PRIVATE IMGUIX_ENABLE_IMCMD)
            target_link_libraries(${TEST_NAME} PRIVATE imcmd::imcmd)
        endif()
        
        if(IMGUIX_USE_IMCOOLBAR)
            target_compile_definitions(${TEST_NAME} PRIVATE IMGUIX_ENABLE_IMCOOLBAR)
            target_link_libraries(${TEST_NAME} PRIVATE imcoolbar::imcoolbar)
        endif()

        if(IMGUIX_USE_IMSPINNER)
            target_compile_definitions(${TEST_NAME} PRIVATE IMGUIX_ENABLE_IMSPINNER)
            target_link_libraries(${TEST_NAME} PRIVATE imspinner::imspinner)
            # Включить встроенную демку из хедера (см. README):
            target_compile_definitions(${TEST_NAME} PRIVATE IMSPINNER_DEMO)
        endif()
        
        if(IMGUIX_USE_IMGUI_MD)
            target_compile_definitions(${TEST_NAME} PRIVATE IMGUIX_ENABLE_IMGUI_MD)
            target_link_libraries(${TEST_NAME} PRIVATE imgui_md::imgui_md)
        endif()

        imguix_add_imgui_demo(${TEST_NAME})

        if(WIN32)
            target_link_libraries(${TEST_NAME} PRIVATE gdi32 user32 comctl32 dwmapi)
        endif()

        add_dependencies(${TEST_NAME} imguix_tests_assets)

        imguix_copy_and_embed_app_icon(${TEST_NAME})

        set_target_properties(${TEST_NAME} PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/tests"
        )

        add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})
    endforeach()
endif()

# ===== Package Config (build tree) =====
include(CMakePackageConfigHelpers)
configure_package_config_file(
    cmake/ImGuiXConfig.cmake.in
    ${CMAKE_BINARY_DIR}/ImGuiXConfig.cmake
    INSTALL_DESTINATION lib/cmake/ImGuiX
)
install(TARGETS imguix
    EXPORT ImGuiXTargets
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
    INCLUDES DESTINATION include
)

# ===== Examples =====
if(IMGUIX_BUILD_EXAMPLES)
    file(GLOB SMOKE_EXAMPLES CONFIGURE_DEPENDS "${PROJECT_SOURCE_DIR}/examples/smoke/*.cpp")

    imguix_stage_assets_once(
        NAME imguix_examples_assets
        DEST "${CMAKE_BINARY_DIR}/examples/data"
        DIRS "${PROJECT_SOURCE_DIR}/assets/data"
        EXCLUDE_DIRS web
    )

    set(_IMGUI_DIR "${IMGUI_DIR}")
    if(NOT _IMGUI_DIR)
        set(_IMGUI_DIR "${PROJECT_SOURCE_DIR}/libs/imgui")
    endif()

    foreach(EXAMPLE_FILE ${SMOKE_EXAMPLES})
        get_filename_component(EXAMPLE_NAME "${EXAMPLE_FILE}" NAME_WE)
        add_executable(${EXAMPLE_NAME} "${EXAMPLE_FILE}")

        target_compile_definitions(${EXAMPLE_NAME} PRIVATE IMGUIX_DEMO)

        if(IMGUIX_USE_SFML_BACKEND)
            target_compile_definitions(${EXAMPLE_NAME} PRIVATE IMGUIX_USE_SFML_BACKEND)
        elseif(IMGUIX_USE_GLFW_BACKEND)
            target_compile_definitions(${EXAMPLE_NAME} PRIVATE IMGUIX_USE_GLFW_BACKEND)
        elseif(IMGUIX_USE_SDL2_BACKEND)
            target_compile_definitions(${EXAMPLE_NAME} PRIVATE IMGUIX_USE_SDL2_BACKEND)
        endif()

        target_include_directories(${EXAMPLE_NAME} PRIVATE
            "${CMAKE_CURRENT_LIST_DIR}/include"
        )
        target_include_directories(${EXAMPLE_NAME} PRIVATE
            "${_IMGUI_DIR}/misc/cpp"
        )

        if(IMGUIX_IMGUI_FREETYPE)
            target_compile_definitions(${EXAMPLE_NAME} PRIVATE IMGUI_ENABLE_FREETYPE)
            target_include_directories(${EXAMPLE_NAME} PRIVATE
                "${_IMGUI_DIR}/misc/freetype"
            )
            imguix_use_or_find_freetype(FREETYPE_TARGET)
            target_link_libraries(${EXAMPLE_NAME} PRIVATE ${FREETYPE_TARGET})
        endif()

        target_link_libraries(${EXAMPLE_NAME} PRIVATE ImGuiX::imguix)
        target_link_libraries(${EXAMPLE_NAME} PRIVATE ${IMGUI_LIB} ${BACKEND_LIBS})
        target_link_libraries(${EXAMPLE_NAME} PRIVATE fmt::fmt nlohmann_json::nlohmann_json)

        if(IMGUIX_USE_IMPLOT)
            target_compile_definitions(${EXAMPLE_NAME} PRIVATE IMGUIX_ENABLE_IMPLOT)
            target_link_libraries(${EXAMPLE_NAME} PRIVATE implot::implot)
            imguix_add_implot_demo(${EXAMPLE_NAME})
        endif()

        if(IMGUIX_USE_IMPLOT3D)
            target_compile_definitions(${EXAMPLE_NAME} PRIVATE IMGUIX_ENABLE_IMPLOT3D)
            target_link_libraries(${EXAMPLE_NAME} PRIVATE implot3d::implot3d)
            imguix_add_implot3d_demo(${EXAMPLE_NAME})
        endif()

        if(IMGUIX_USE_IMNODEFLOW)
            target_compile_definitions(${EXAMPLE_NAME} PRIVATE IMGUIX_ENABLE_IMNODEFLOW)
            target_link_libraries(${EXAMPLE_NAME} PRIVATE imnodeflow::imnodeflow)
        endif()

        if(IMGUIX_USE_PFD)
            target_compile_definitions(${EXAMPLE_NAME} PRIVATE IMGUIX_ENABLE_PFD)
            target_link_libraries(${EXAMPLE_NAME} PRIVATE pfd::pfd)
        endif()

        if(IMGUIX_USE_IMGUIFILEDIALOG)
            target_compile_definitions(${EXAMPLE_NAME} PRIVATE IMGUIX_ENABLE_IMGUIFILEDIALOG)
            target_link_libraries(${EXAMPLE_NAME} PRIVATE imguifiledialog::imguifiledialog)
        endif()

        if(IMGUIX_USE_IMTEXTEDITOR)
            target_compile_definitions(${EXAMPLE_NAME} PRIVATE IMGUIX_ENABLE_IMTEXTEDITOR)
            target_link_libraries(${EXAMPLE_NAME} PRIVATE imtexteditor::imtexteditor)
        endif()

        if(IMGUIX_USE_IMCMD)
            target_compile_definitions(${EXAMPLE_NAME} PRIVATE IMGUIX_ENABLE_IMCMD)
            target_link_libraries(${EXAMPLE_NAME} PRIVATE imcmd::imcmd)
        endif()

        if(IMGUIX_USE_IMCOOLBAR)
            target_compile_definitions(${EXAMPLE_NAME} PRIVATE IMGUIX_ENABLE_IMCOOLBAR)
            target_link_libraries(${EXAMPLE_NAME} PRIVATE imcoolbar::imcoolbar)
        endif()

        if(IMGUIX_USE_IMSPINNER)
            target_compile_definitions(${EXAMPLE_NAME} PRIVATE IMGUIX_ENABLE_IMSPINNER)
            target_link_libraries(${EXAMPLE_NAME} PRIVATE imspinner::imspinner)
            target_compile_definitions(${EXAMPLE_NAME} PRIVATE IMSPINNER_DEMO)
        endif()

        if(IMGUIX_USE_IMGUI_MD)
            target_compile_definitions(${EXAMPLE_NAME} PRIVATE IMGUIX_ENABLE_IMGUI_MD)
            target_link_libraries(${EXAMPLE_NAME} PRIVATE imgui_md::imgui_md)
        endif()

        imguix_add_imgui_demo(${EXAMPLE_NAME})

        if(WIN32)
            target_link_libraries(${EXAMPLE_NAME} PRIVATE gdi32 user32 comctl32 dwmapi)
        endif()

        add_dependencies(${EXAMPLE_NAME} imguix_examples_assets)

        imguix_copy_and_embed_app_icon(${EXAMPLE_NAME})

        set_target_properties(${EXAMPLE_NAME} PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/examples"
        )
    endforeach()

endif()

# ===== Install / Export =====
# Public headers of ImGuiX
install(DIRECTORY ${PROJECT_SOURCE_DIR}/include/imguix
        DESTINATION include)

# Variant B: vendor json headers when enabled
if(IMGUIX_VENDOR_JSON)
    # Place headers so <nlohmann/json.hpp> works out of the box
    install(DIRECTORY ${PROJECT_SOURCE_DIR}/libs/json/include/nlohmann
            DESTINATION include/nlohmann)
endif()

install(EXPORT ImGuiXTargets
    NAMESPACE ImGuiX::
    DESTINATION lib/cmake/ImGuiX
)
install(FILES ${CMAKE_BINARY_DIR}/ImGuiXConfig.cmake
        DESTINATION lib/cmake/ImGuiX)

# ===== SDK bundle of 3rd-party deps =====

# Install target and optionally its PUBLIC_HEADER files
# Params:
# - tgt: target name to install
# Behavior:
# - Installs library and headers if PUBLIC_HEADER is defined
# Usage:
#   imguix_install_target_with_headers(imgui)
# Idempotent: no-op if target is missing
function(imguix_install_target_with_headers tgt)
    if(NOT TARGET ${tgt})
        return()
    endif()
    get_target_property(_type ${tgt} TYPE)
    get_target_property(_pub ${tgt} PUBLIC_HEADER)
    get_target_property(_imported ${tgt} IMPORTED)
    if(_imported)
        return()
    endif()

    if(_type STREQUAL "INTERFACE_LIBRARY")
        if(_pub)
            install(TARGETS ${tgt}
                EXPORT ImGuiXTargets
                PUBLIC_HEADER DESTINATION include
            )
        else()
            install(TARGETS ${tgt}
                EXPORT ImGuiXTargets
            )
        endif()
    else()
        if(_pub)
            install(TARGETS ${tgt}
                EXPORT ImGuiXTargets
                ARCHIVE DESTINATION lib
                LIBRARY DESTINATION lib
                RUNTIME DESTINATION bin
                PUBLIC_HEADER DESTINATION include
            )
        else()
            install(TARGETS ${tgt}
                EXPORT ImGuiXTargets
                ARCHIVE DESTINATION lib
                LIBRARY DESTINATION lib
                RUNTIME DESTINATION bin
            )
        endif()
    endif()
endfunction()


if(IMGUIX_SDK_BUNDLE_DEPS)
    # --- Dear ImGui: install only required headers, skip aux folders ---
    set(_IMGUI_DIR "${IMGUI_DIR}")
    if(NOT _IMGUI_DIR)
        set(_IMGUI_DIR "${PROJECT_SOURCE_DIR}/libs/imgui")
    endif()

    if(EXISTS "${_IMGUI_DIR}/imgui.h")
        install(
            DIRECTORY "${_IMGUI_DIR}/"
            DESTINATION "include"
            FILES_MATCHING
                # Include headers only
                PATTERN "*.h"
                # Also take headers from misc/cpp and misc/freetype
                PATTERN "misc/cpp/*.h"
                PATTERN "misc/freetype/*.h"
            # Exclude irrelevant directories
            PATTERN ".git*"     EXCLUDE
            PATTERN ".github*"  EXCLUDE
            PATTERN "backends"  EXCLUDE
            PATTERN "docs"      EXCLUDE
            PATTERN "examples*" EXCLUDE
            PATTERN "scripts"   EXCLUDE
            PATTERN "bindings"  EXCLUDE
        )

        if(IMGUIX_SDK_FLATTEN_MISC_HEADERS)
            # Duplicate misc headers into root include for simple includes
            if(EXISTS "${_IMGUI_DIR}/misc/cpp/imgui_stdlib.h")
                install(FILES "${_IMGUI_DIR}/misc/cpp/imgui_stdlib.h"
                        DESTINATION "include")
            endif()
            if(EXISTS "${_IMGUI_DIR}/misc/freetype/imgui_freetype.h")
                install(FILES "${_IMGUI_DIR}/misc/freetype/imgui_freetype.h"
                        DESTINATION "include")
            endif()
        endif()
    endif()

    imguix_install_target_with_headers(imgui)


    # --- ImGui-SFML: static library and headers ---
    if(TARGET ImGui-SFML)
        imguix_install_target_with_headers(ImGui-SFML)
        if(NOT DEFINED IMGUI_SFML_DIR OR IMGUI_SFML_DIR STREQUAL "")
            set(IMGUI_SFML_DIR "${PROJECT_SOURCE_DIR}/libs/imgui-sfml")
        endif()
        if(EXISTS "${IMGUI_SFML_DIR}/imgui-SFML.h")
            install(FILES
                    "${IMGUI_SFML_DIR}/imgui-SFML.h"
                    "${IMGUI_SFML_DIR}/imconfig-SFML.h"
                    DESTINATION include)
        endif()
    endif()

    # --- fmt: library and headers if submodule ---
    if(TARGET fmt)
        if(DEFINED FMT_INSTALL AND FMT_INSTALL)
            message(STATUS "SDK: fmt installs itself (FMT_INSTALL=ON) — skipping manual header install")
        else()
            imguix_install_target_with_headers(fmt)
            if(EXISTS "${PROJECT_SOURCE_DIR}/libs/fmt/include/fmt")
                install(DIRECTORY "${PROJECT_SOURCE_DIR}/libs/fmt/include/fmt"
                        DESTINATION include)
            endif()
        endif()
    endif()

    # --- nlohmann-json (header-only) when vendored ---
    if(IMGUIX_VENDOR_JSON)
        imguix_install_target_with_headers(nlohmann_json)
        if(EXISTS "${PROJECT_SOURCE_DIR}/libs/json/include/nlohmann")
            install(DIRECTORY "${PROJECT_SOURCE_DIR}/libs/json/include/nlohmann"
                    DESTINATION include)
        endif()
    endif()

    # --- ImPlot: headers ---
    imguix_install_target_with_headers(implot)
    if(EXISTS "${PROJECT_SOURCE_DIR}/libs/implot/implot.h")
        install(FILES
            "${PROJECT_SOURCE_DIR}/libs/implot/implot.h"
            "${PROJECT_SOURCE_DIR}/libs/implot/implot_internal.h"
            DESTINATION include
        )
    endif()

    # --- ImPlot3D: headers ---
    imguix_install_target_with_headers(implot3d)
    if(EXISTS "${PROJECT_SOURCE_DIR}/libs/implot3d/implot3d.h")
        install(FILES
            "${PROJECT_SOURCE_DIR}/libs/implot3d/implot3d.h"
            "${PROJECT_SOURCE_DIR}/libs/implot3d/implot3d_internal.h"
            DESTINATION include
        )
    endif()
    
    
    # --- ImNodeFlow: headers ---
    imguix_install_target_with_headers(imnodeflow)
    if(EXISTS "${PROJECT_SOURCE_DIR}/libs/ImNodeFlow/include/ImNodeFlow.h")
        install(FILES
            "${PROJECT_SOURCE_DIR}/libs/ImNodeFlow/include/ImNodeFlow.h"
            DESTINATION include
        )
    endif()

    # --- ImGuiFileDialog: headers ---
    imguix_install_target_with_headers(imguifiledialog)
    if(EXISTS "${PROJECT_SOURCE_DIR}/libs/ImGuiFileDialog/ImGuiFileDialog.h")
        install(FILES
            "${PROJECT_SOURCE_DIR}/libs/ImGuiFileDialog/ImGuiFileDialog.h"
            "${PROJECT_SOURCE_DIR}/libs/ImGuiFileDialog/ImGuiFileDialogConfig.h"
            DESTINATION include)
    endif()

    # --- ImGuiColorTextEdit: headers ---
    imguix_install_target_with_headers(imtexteditor)
    if(EXISTS "${PROJECT_SOURCE_DIR}/libs/ImGuiColorTextEdit/ImGuiColorTextEdit.h")
        install(FILES
            "${PROJECT_SOURCE_DIR}/libs/ImGuiColorTextEdit/ImGuiColorTextEdit.h"
            DESTINATION include)
    endif()
    
    # --- portable-file-dialogs: headers ---
    imguix_install_target_with_headers(pfd)
    if(EXISTS "${PROJECT_SOURCE_DIR}/libs/portable-file-dialogs/portable-file-dialogs.h")
        install(FILES
            "${PROJECT_SOURCE_DIR}/libs/portable-file-dialogs/portable-file-dialogs.h"
            DESTINATION include
        )
    endif()

    # --- imspinner: headers ---
    imguix_install_target_with_headers(imspinner)
    if(EXISTS "${PROJECT_SOURCE_DIR}/libs/imspinner/imspinner.h")
        install(FILES "${PROJECT_SOURCE_DIR}/libs/imspinner/imspinner.h"
                DESTINATION include)
    endif()

    # --- imgui-command-palette: headers ---
    imguix_install_target_with_headers(imcmd)
    if(EXISTS "${PROJECT_SOURCE_DIR}/libs/imgui-command-palette/imcmd_command_palette.h")
        install(FILES
            "${PROJECT_SOURCE_DIR}/libs/imgui-command-palette/imcmd_command_palette.h"
            "${PROJECT_SOURCE_DIR}/libs/imgui-command-palette/imcmd_fuzzy_search.h"
            DESTINATION include)
    endif()

    # --- ImCoolBar: headers ---
    imguix_install_target_with_headers(imcoolbar)
    if(EXISTS "${PROJECT_SOURCE_DIR}/libs/ImCoolBar/ImCoolBar.h")
        install(FILES "${PROJECT_SOURCE_DIR}/libs/ImCoolBar/ImCoolBar.h"
                DESTINATION include)
    endif()

    # --- imgui_md: headers ---
    imguix_install_target_with_headers(imgui_md)
    imguix_install_target_with_headers(md4c)
    if(EXISTS "${PROJECT_SOURCE_DIR}/libs/imgui_md/imgui_md.h")
        install(FILES "${PROJECT_SOURCE_DIR}/libs/imgui_md/imgui_md.h"
                DESTINATION include)
    endif()
    # md4c header (root or src)
    if(EXISTS "${PROJECT_SOURCE_DIR}/libs/md4c/src/md4c.h")
        install(FILES "${PROJECT_SOURCE_DIR}/libs/md4c/src/md4c.h"
                DESTINATION include)
    elseif(EXISTS "${PROJECT_SOURCE_DIR}/libs/md4c/md4c.h")
        install(FILES "${PROJECT_SOURCE_DIR}/libs/md4c/md4c.h"
                DESTINATION include)
    endif()

endif()

# --- Quickstart and resources (guarded by IMGUIX_SDK_INSTALL) ---
if(IMGUIX_SDK_INSTALL AND IMGUIX_SDK_INSTALL_QUICKSTART)
    # Copy resources required by quickstart
    if(EXISTS "${PROJECT_SOURCE_DIR}/assets/data/resources")
        install(DIRECTORY "${PROJECT_SOURCE_DIR}/assets/data/resources/"
                DESTINATION quickstart/data/resources)
    endif()

    # Install quickstart code if present in repository
    if(EXISTS "${PROJECT_SOURCE_DIR}/quickstart/CMakeLists.txt")
        install(DIRECTORY "${PROJECT_SOURCE_DIR}/quickstart/"
                DESTINATION quickstart
                PATTERN "build*" EXCLUDE
                PATTERN ".vs" EXCLUDE
                PATTERN ".idea" EXCLUDE)
    endif()
endif()
