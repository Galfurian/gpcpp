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
    # Disable warnings for MSVC-specific "safe" functions like strcpy_s, etc.,
    # which are not portable and may clutter warning logs.
    target_compile_definitions(gpcpp INTERFACE _CRT_SECURE_NO_WARNINGS)

    # Disable warning C4702: unreachable code.
    add_compile_options(/wd4702)

    if(WARNINGS_AS_ERRORS)
        # Treat all warnings as errors to enforce stricter code quality.
        target_compile_options(gpcpp INTERFACE /WX)
    endif()

    if(STRICT_WARNINGS)
        # Enable external header management to suppress warnings in system and
        # external headers, making it easier to focus on project-specific issues.
        target_compile_options(gpcpp INTERFACE /experimental:external)
        target_compile_options(gpcpp INTERFACE /external:I ${CMAKE_BINARY_DIR})
        target_compile_options(gpcpp INTERFACE /external:anglebrackets)
        target_compile_options(gpcpp INTERFACE /external:W0)

        # Use a high warning level to catch as many potential issues as possible.
        target_compile_options(gpcpp INTERFACE /W4)

        # Enforce standards-compliant behavior to avoid relying on MSVC-specific extensions.
        target_compile_options(gpcpp INTERFACE /permissive-)
    endif()

elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    if(WARNINGS_AS_ERRORS)
        # Treat all warnings as errors to enforce stricter code quality.
        target_compile_options(gpcpp INTERFACE -Werror)
    endif()

    if(STRICT_WARNINGS)
        # Enable a broad set of warnings to catch common and subtle issues:
        target_compile_options(gpcpp INTERFACE
            -Wall                # Enable most general-purpose warnings.
            -Wextra              # Enable extra warnings not included in -Wall.
            -Wconversion         # Warn about implicit type conversions that may lose data.
            -pedantic            # Enforce strict compliance with the C++ standard.
            -Wshadow             # Warn about variable shadowing, which can cause subtle bugs.
            -Wnon-virtual-dtor   # Warn when a class with virtual functions lacks a virtual destructor.
            -Wnull-dereference   # Warn about potential null pointer dereferences.
            -Wformat=2           # Enable strict checks for printf/scanf format strings.
            -Woverloaded-virtual # Warn when a derived class function hides a base class virtual function.
            -Wfloat-equal        # Warn about direct comparisons of floating-point values, which can be imprecise.
        )

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

    add_executable(gpcpp_example_custom_line_type_color examples/example_custom_line_type_color.cpp)
    target_include_directories(gpcpp_example_custom_line_type_color PUBLIC ${PROJECT_SOURCE_DIR}/examples ${PROJECT_SOURCE_DIR}/include)
    target_link_libraries(gpcpp_example_custom_line_type_color PUBLIC gpcpp)

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

    add_executable(gpcpp_example_custom_grid examples/example_custom_grid.cpp)
    target_include_directories(gpcpp_example_custom_grid PUBLIC ${PROJECT_SOURCE_DIR}/examples ${PROJECT_SOURCE_DIR}/include)
    target_link_libraries(gpcpp_example_custom_grid PUBLIC gpcpp)

    add_executable(gpcpp_example_labels examples/example_labels.cpp)
    target_include_directories(gpcpp_example_labels PUBLIC ${PROJECT_SOURCE_DIR}/examples ${PROJECT_SOURCE_DIR}/include)
    target_link_libraries(gpcpp_example_labels PUBLIC gpcpp)

endif()

# -----------------------------------------------------------------------------
# DOCUMENTATION
# -----------------------------------------------------------------------------

if(DOXYGEN_FOUND)
        
    # FetchContent: Doxygen Awesome CSS
    FetchContent_Declare(doxygenawesome
        GIT_REPOSITORY https://github.com/jothepro/doxygen-awesome-css
        GIT_TAG main
    )
    FetchContent_MakeAvailable(doxygenawesome)

    # Hide FetchContent variables to avoid clutter in ccmake.
    mark_as_advanced(FORCE
        FETCHCONTENT_UPDATES_DISCONNECTED_DOXYGENAWESOME
        FETCHCONTENT_SOURCE_DIR_DOXYGENAWESOME
    )

    # Read the file with the version.
    file(READ ${PROJECT_SOURCE_DIR}/include/gpcpp/gnuplot.hpp version_file)
    # Extract the version.
    string(REGEX MATCH "GPCPP_MAJOR_VERSION ([0-9]*)" _ ${version_file})
    set(GPCPP_MAJOR_VERSION ${CMAKE_MATCH_1})
    string(REGEX MATCH "GPCPP_MINOR_VERSION ([0-9]*)" _ ${version_file})
    set(GPCPP_MINOR_VERSION ${CMAKE_MATCH_1})
    string(REGEX MATCH "GPCPP_MICRO_VERSION ([0-9]*)" _ ${version_file})
    set(GPCPP_MICRO_VERSION ${CMAKE_MATCH_1})

    # Customization: Doxygen Configuration
    set(DOXYGEN_WARN_FORMAT "$file:$line: $text")
    set(DOXYGEN_PROJECT_NAME GPCPP)
    set(DOXYGEN_PROJECT_BRIEF "Simple wrapper for gnuplot for C++")
    set(DOXYGEN_PROJECT_NUMBER "${GPCPP_MAJOR_VERSION}.${GPCPP_MINOR_VERSION}.${GPCPP_MICRO_VERSION}")
    set(DOXYGEN_USE_MDFILE_AS_MAINPAGE ${PROJECT_SOURCE_DIR}/README.md)
    set(DOXYGEN_SHOW_INCLUDE_FILES NO)
    set(DOXYGEN_GENERATE_TREEVIEW YES)
    set(DOXYGEN_GENERATE_LATEX NO)
    set(DOXYGEN_GENERATE_MAN NO)

    # Styling and UX enhancements using Doxygen Awesome
    set(DOXYGEN_HTML_HEADER ${doxygenawesome_SOURCE_DIR}/doxygen-custom/header.html)
    set(DOXYGEN_HTML_EXTRA_STYLESHEET ${doxygenawesome_SOURCE_DIR}/doxygen-awesome.css)
    set(DOXYGEN_HTML_EXTRA_FILES
        ${doxygenawesome_SOURCE_DIR}/doxygen-awesome-fragment-copy-button.js
        ${doxygenawesome_SOURCE_DIR}/doxygen-awesome-paragraph-link.js
        ${doxygenawesome_SOURCE_DIR}/doxygen-awesome-darkmode-toggle.js
    )

    # Set stricter warnings for better documentation quality
    set(DOXYGEN_WARN_IF_UNDOCUMENTED YES)
    set(DOXYGEN_WARN_IF_DOC_ERROR YES)
    set(DOXYGEN_WARN_NO_PARAMDOC YES)
    set(DOXYGEN_WARN_AS_ERROR YES)

    # Exclude certain files or directories from documentation (if needed)
    set(DOXYGEN_EXCLUDE_PATTERNS
        "${PROJECT_SOURCE_DIR}/tests/*"
        "${PROJECT_SOURCE_DIR}/examples/*"
    )

    # Add Doxygen documentation target
    doxygen_add_docs(
        gpcpp_documentation
        ${PROJECT_SOURCE_DIR}/README.md
        ${PROJECT_SOURCE_DIR}/LICENSE.md
        ${PROJECT_SOURCE_DIR}/include/gpcpp/box_style.hpp
        ${PROJECT_SOURCE_DIR}/include/gpcpp/color.hpp
        ${PROJECT_SOURCE_DIR}/include/gpcpp/defines.hpp
        ${PROJECT_SOURCE_DIR}/include/gpcpp/gnuplot.hpp
        ${PROJECT_SOURCE_DIR}/include/gpcpp/gnuplot.i.hpp
        ${PROJECT_SOURCE_DIR}/include/gpcpp/id_manager.hpp
        COMMENT "Generating Doxygen documentation for the library"
    )

endif()
