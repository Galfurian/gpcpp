# -----------------------------------------------------------------------------
# @brief  : Root cmake file.
# @author : Enrico Fraccaroli
# -----------------------------------------------------------------------------

# Set the minimum CMake version, the project name and default build type.
cmake_minimum_required(VERSION 3.1...3.18)

# Set the project name.
project(gpcpp CXX)

# Set the default build type to Debug.
if(NOT CMAKE_BUILD_TYPE)
    message(STATUS "Setting build type to 'Debug' as none was specified.")
    set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Choose the type of build." FORCE)
endif()

# -----------------------------------------------------------------------------
# ENABLE FETCH CONTENT
# -----------------------------------------------------------------------------

# We need this in order to import external projects.
include(FetchContent)
# Hide fetchcontent variables.
mark_as_advanced(FORCE
    FETCHCONTENT_QUIET
    FETCHCONTENT_BASE_DIR
    FETCHCONTENT_FULLY_DISCONNECTED
    FETCHCONTENT_UPDATES_DISCONNECTED
)

# -----------------------------------------------------------------------------
# OPTIONS
# -----------------------------------------------------------------------------

option(BUILD_TESTS "Build tests" OFF)
option(BUILD_EXAMPLES "Build examples" ON)
option(STRICT_WARNINGS "Enable strict compiler warnings" ON)
option(WARNINGS_AS_ERRORS "Treat all warnings as errors" OFF)

# -----------------------------------------------------------------------------
# DEPENDENCY (SYSTEM LIBRARIES)
# -----------------------------------------------------------------------------

find_package(Doxygen)

# -----------------------------------------------------------------------------
# LIBRARY
# -----------------------------------------------------------------------------

# Add the C++ Library.
add_library(gpcpp INTERFACE)
add_library(gpcpp::gpcpp ALIAS gpcpp)
# Inlcude header directories.
target_include_directories(gpcpp INTERFACE ${PROJECT_SOURCE_DIR}/include)
# Set the library to use c++-20
target_compile_features(gpcpp INTERFACE cxx_std_11)

# -----------------------------------------------------------------------------
# COMPILATION FLAGS
# -----------------------------------------------------------------------------

if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    # Disable warnings that suggest using MSVC-specific safe functions
    target_compile_definitions(gpcpp INTERFACE _CRT_SECURE_NO_WARNINGS)
    if(WARNINGS_AS_ERRORS)
        target_compile_options(gpcpp INTERFACE /WX)
    endif()
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    if(WARNINGS_AS_ERRORS)
        target_compile_options(gpcpp INTERFACE -Werror)
    endif()
endif()

if(STRICT_WARNINGS)
    if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
        # Mark system headers as external for MSVC explicitly
        # https://devblogs.microsoft.com/cppblog/broken-warnings-theory
        target_compile_options(gpcpp INTERFACE /experimental:external)
        target_compile_options(gpcpp INTERFACE /external:I ${CMAKE_BINARY_DIR})
        target_compile_options(gpcpp INTERFACE /external:anglebrackets)
        target_compile_options(gpcpp INTERFACE /external:W0)

        target_compile_options(gpcpp INTERFACE /W4)
    elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        target_compile_options(gpcpp INTERFACE -Wall -Wextra -Wconversion -pedantic)
    endif()
endif()

# -----------------------------------------------------------------------------
# EXAMPLES
# -----------------------------------------------------------------------------

if(BUILD_EXAMPLES)

    add_executable(gpcpp_example_line_plot examples/example_line_plot.cpp)
    target_include_directories(gpcpp_example_line_plot PUBLIC ${PROJECT_SOURCE_DIR}/examples ${PROJECT_SOURCE_DIR}/include)
    target_link_libraries(gpcpp_example_line_plot PUBLIC gpcpp)

    add_executable(gpcpp_example_scatter_plot examples/example_scatter_plot.cpp)
    target_include_directories(gpcpp_example_scatter_plot PUBLIC ${PROJECT_SOURCE_DIR}/examples ${PROJECT_SOURCE_DIR}/include)
    target_link_libraries(gpcpp_example_scatter_plot PUBLIC gpcpp)

    add_executable(gpcpp_example_line_plot_with_error_bars examples/example_line_plot_with_error_bars.cpp)
    target_include_directories(gpcpp_example_line_plot_with_error_bars PUBLIC ${PROJECT_SOURCE_DIR}/examples ${PROJECT_SOURCE_DIR}/include)
    target_link_libraries(gpcpp_example_line_plot_with_error_bars PUBLIC gpcpp)

    add_executable(gpcpp_example_3d_surface_plot examples/example_3d_surface_plot.cpp)
    target_include_directories(gpcpp_example_3d_surface_plot PUBLIC ${PROJECT_SOURCE_DIR}/examples ${PROJECT_SOURCE_DIR}/include)
    target_link_libraries(gpcpp_example_3d_surface_plot PUBLIC gpcpp)

    add_executable(gpcpp_example_contour_plot examples/example_contour_plot.cpp)
    target_include_directories(gpcpp_example_contour_plot PUBLIC ${PROJECT_SOURCE_DIR}/examples ${PROJECT_SOURCE_DIR}/include)
    target_link_libraries(gpcpp_example_contour_plot PUBLIC gpcpp)

    add_executable(gpcpp_example_custom_line_style_color examples/example_custom_line_style_color.cpp)
    target_include_directories(gpcpp_example_custom_line_style_color PUBLIC ${PROJECT_SOURCE_DIR}/examples ${PROJECT_SOURCE_DIR}/include)
    target_link_libraries(gpcpp_example_custom_line_style_color PUBLIC gpcpp)

    add_executable(gpcpp_example_logarithmic_scaling examples/example_logarithmic_scaling.cpp)
    target_include_directories(gpcpp_example_logarithmic_scaling PUBLIC ${PROJECT_SOURCE_DIR}/examples ${PROJECT_SOURCE_DIR}/include)
    target_link_libraries(gpcpp_example_logarithmic_scaling PUBLIC gpcpp)

    add_executable(gpcpp_example_multiple_plots_one_window examples/example_multiple_plots_one_window.cpp)
    target_include_directories(gpcpp_example_multiple_plots_one_window PUBLIC ${PROJECT_SOURCE_DIR}/examples ${PROJECT_SOURCE_DIR}/include)
    target_link_libraries(gpcpp_example_multiple_plots_one_window PUBLIC gpcpp)

    add_executable(gpcpp_example_multiple_styles examples/example_multiple_styles.cpp)
    target_include_directories(gpcpp_example_multiple_styles PUBLIC ${PROJECT_SOURCE_DIR}/examples ${PROJECT_SOURCE_DIR}/include)
    target_link_libraries(gpcpp_example_multiple_styles PUBLIC gpcpp)

    add_executable(gpcpp_example_save_plot_to_file examples/example_save_plot_to_file.cpp)
    target_include_directories(gpcpp_example_save_plot_to_file PUBLIC ${PROJECT_SOURCE_DIR}/examples ${PROJECT_SOURCE_DIR}/include)
    target_link_libraries(gpcpp_example_save_plot_to_file PUBLIC gpcpp)

endif()

# -----------------------------------------------------------------------------
# TESTS
# -----------------------------------------------------------------------------

if(BUILD_TESTS)

    # CMake has support for adding tests to a project.
    enable_testing()

endif()

# -----------------------------------------------------------------------------
# DOCUMENTATION
# -----------------------------------------------------------------------------

if(DOXYGEN_FOUND)
    
    # Record the options that describe how to populate the specified content.
    FetchContent_Declare(doxygenawesome
        GIT_REPOSITORY https://github.com/jothepro/doxygen-awesome-css
        GIT_TAG main
    )
    # Retrieve the properties related to the content.
    FetchContent_MakeAvailable(doxygenawesome)
    # Hide fetchcontent variables, otherwise with ccmake it's a mess.
    mark_as_advanced(FORCE
        FETCHCONTENT_UPDATES_DISCONNECTED_DOXYGENAWESOME
        FETCHCONTENT_SOURCE_DIR_DOXYGENAWESOME
    )

    # = CUSTOMIZATION =========================================================
    set(DOXYGEN_WARN_FORMAT "$file:$line: $text")
    set(DOXYGEN_PROJECT_NAME gpcpp)
    set(DOXYGEN_USE_MDFILE_AS_MAINPAGE ${PROJECT_SOURCE_DIR}/README.md)
    set(DOXYGEN_SHOW_INCLUDE_FILES NO)
    set(DOXYGEN_GENERATE_TREEVIEW YES)
    set(DOXYGEN_HTML_HEADER ${doxygenawesome_SOURCE_DIR}/doxygen-custom/header.html)
    set(DOXYGEN_HTML_EXTRA_STYLESHEET ${doxygenawesome_SOURCE_DIR}/doxygen-awesome.css)
    set(DOXYGEN_HTML_EXTRA_FILES
        ${doxygenawesome_SOURCE_DIR}/doxygen-awesome-fragment-copy-button.js
        ${doxygenawesome_SOURCE_DIR}/doxygen-awesome-paragraph-link.js
        ${doxygenawesome_SOURCE_DIR}/doxygen-awesome-darkmode-toggle.js
    )
    doxygen_add_docs(
        gpcpp_documentation
        ${PROJECT_SOURCE_DIR}/README.md
        ${PROJECT_SOURCE_DIR}/LICENSE.md
        ${PROJECT_SOURCE_DIR}/include/gpcpp/color.hpp
        ${PROJECT_SOURCE_DIR}/include/gpcpp/gnuplot.hpp
        ${PROJECT_SOURCE_DIR}/include/gpcpp/gnuplot.i.hpp
    )
endif()
