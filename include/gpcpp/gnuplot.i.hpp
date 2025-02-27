/// @file gnuplot.i.hpp
/// @brief

#pragma once

#include "gnuplot.hpp"

#include <cmath>
#include <cstring>
#include <type_traits>

namespace gpcpp
{

/// @brief Checks if the specified style is a line style.
/// @param style The plot style to check.
/// @return true if the style is a line style, false otherwise.
static auto is_line_type(plot_type_t style) -> bool
{
    return (
        style == plot_type_t::lines || style == plot_type_t::lines_points || style == plot_type_t::steps ||
        style == plot_type_t::fsteps || style == plot_type_t::histeps || style == plot_type_t::filled_curves ||
        style == plot_type_t::impulses);
}

/// @brief Checks if the specified style is a point style.
/// @param style The plot style to check.
/// @return true if the style is a point style, false otherwise.
static auto is_point_type(plot_type_t style) -> bool
{
    return (style == plot_type_t::points || style == plot_type_t::lines_points);
}

/// @brief Checks if the difference between two floating-point values exceeds a tolerance.
/// @param a The first value.
/// @param b The second value.
/// @param tolerance The allowable difference (default is 1e-6).
/// @returns True if the difference exceeds the tolerance, false otherwise.
template <typename T>
inline auto are_equal(T a, T b, T tolerance = 1e-6) -> bool
{
    static_assert(std::is_floating_point<T>::value, "exceeds_tolerance requires floating-point types.");
    return std::abs(a - b) > tolerance;
}

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__TOS_WIN__)
// Windows-specific static variable initializations
std::string Gnuplot::m_gnuplot_filename  = "pgnuplot.exe";
std::string Gnuplot::m_gnuplot_path      = "C:/program files/gnuplot/bin/";
std::size_t Gnuplot::m_tmpfile_num       = 0;
const std::size_t Gnuplot::m_tmpfile_max = 27;
#elif defined(unix) || defined(__unix) || defined(__unix__) || defined(__APPLE__)
// UNIX-like system static variable initializations
std::string Gnuplot::m_gnuplot_filename  = "gnuplot";
std::string Gnuplot::m_gnuplot_path      = "/usr/local/bin/";
std::size_t Gnuplot::m_tmpfile_num       = 0;
const std::size_t Gnuplot::m_tmpfile_max = 64;
#endif

Gnuplot::Gnuplot(bool _debug)
    : debug(_debug)                       // Debug is disabled.
    , gnuplot_pipe(nullptr)               // No active pipe initially
    , terminal_type(terminal_type_t::wxt) // Default terminal type is wxt
    , valid(false)                        // Invalid session by default
    , two_dim(true)                       // 2D plotting by default
    , nplots(0)                           // No plots initially
    , line_width(1.0)                     // Default line width
    , plot_type(plot_type_t::lines)       // Default plot style is lines
    , smooth_type(smooth_type_t::none)    // No smoothing by default
    , line_type("")                       // No custom line style
    , line_color()                        // Default line color is unspecified
    , point_type(point_type_t::none)      // Default point style is none
    , point_size(-1.0)                    // Default point size is unspecified
    , grid_major_style_id(-1)             // Default is disabled.
    , grid_minor_style_id(-1)             // Default is disabled.
{
#if (defined(unix) || defined(__unix) || defined(__unix__)) && !defined(__APPLE__)
    // Ensure DISPLAY is set for Unix systems.
    if (getenv("DISPLAY") == nullptr) {
        std::cerr << "Error: DISPLAY variable not set.\n";
        valid = false;
        return;
    }
#endif

    // Check if gnuplot is available.
    if (!Gnuplot::get_program_path()) {
        std::cerr << "Error: Gnuplot executable not found.\n";
        valid = false;
        return;
    }

// Try to open a pipe to Gnuplot.
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__TOS_WIN__)
    gnuplot_pipe = _popen((Gnuplot::m_gnuplot_path + "/" + Gnuplot::m_gnuplot_filename).c_str(), "w");
#elif defined(unix) || defined(__unix) || defined(__unix__) || defined(__APPLE__)
    gnuplot_pipe = popen((Gnuplot::m_gnuplot_path + "/" + Gnuplot::m_gnuplot_filename).c_str(), "w");
#else
    std::cerr << "Error: Unsupported platform for opening a pipe to Gnuplot.\n";
    valid = false;
    return;
#endif
    if (gnuplot_pipe == nullptr) {
        std::cerr << "Error: Unable to open pipe to Gnuplot.\n";
        valid = false;
        return;
    }

    // Initialize plotting state.
    valid   = true;
    nplots  = 0;
    two_dim = false;

    // Initialize styles.
    plot_type   = plot_type_t::none;
    smooth_type = smooth_type_t::none;
    line_type.clear();
    line_color.unset();
    point_type = point_type_t::plus;
    point_size = -1.0;
    line_width = -1.0;

    // Initialize contour settings.
    contour.type  = contour_type_t::none;
    contour.param = contour_param_t::levels;
    contour.discrete_levels.clear();
    contour.increment_start = 0.0;
    contour.increment_step  = 0.1;
    contour.increment_end   = 1.0;
    contour.levels          = 10;

    // Clear temporary file list.
    tmpfile_list.clear();
}

Gnuplot::~Gnuplot()
{

    // Close the communication pipe to Gnuplot if it's open
    if (gnuplot_pipe != nullptr) {
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__TOS_WIN__)
        if (_pclose(gnuplot_pipe) == -1) {
            std::cerr << "Warning: Problem closing communication to Gnuplot.\n";
        }
#elif defined(unix) || defined(__unix) || defined(__unix__) || defined(__APPLE__)
        if (pclose(gnuplot_pipe) == -1) {
            std::cerr << "Warning: Problem closing communication to Gnuplot.\n";
        }
#else
        std::cerr << "Error: Unsupported platform for closing Gnuplot pipe.\n";
#endif
        // Avoid dangling pointer.
        gnuplot_pipe = nullptr;
    }

    // Remove all temporary files created during the session
    remove_tmpfiles();
}

auto Gnuplot::send_cmd(const std::string &cmdstr) -> Gnuplot &
{
    // Check if the Gnuplot session is ready.
    if (!this->is_ready()) {
        std::cerr << "Error: Invalid Gnuplot session not ready.\n";
        return *this;
    }

    if (debug) {
        std::cout << cmdstr.c_str() << "\n";
    }

    // Write the command to the Gnuplot pipe.
    fprintf(gnuplot_pipe, "%s\n", cmdstr.c_str());

    // Check and update state based on the command type.
    if (cmdstr.find("replot") != std::string::npos) {
        // Do not increment plot count or change dimensionality.
    }
    // Command starts with "splot".
    else if (cmdstr.find("splot") == 0) {
        two_dim = false;
        nplots++;
    }
    // Command starts with "plot".
    else if (cmdstr.find("plot") == 0) {
        two_dim = true;
        nplots++;
    }

    return *this;
}

auto Gnuplot::plot_vertical_line(double x) -> Gnuplot &
{
    // Check if the Gnuplot session is ready.
    if (!this->is_ready()) {
        std::cerr << "Error: Invalid Gnuplot session not ready.\n";
        return *this;
    }

    std::ostringstream oss;

    // Construct the arrow command for the vertical line
    oss << "set arrow from " << x << ", graph 0 to " << x << ", graph 1 nohead ";

    // Include line color if it is specified.
    if (line_color.is_set()) {
        oss << " lc rgbcolor \"" << line_color.to_string() << "\"";
    } else {
        oss << " lc rgbcolor \"black\"";
    }

    // Add line width if specified.
    if (line_width > 0) {
        oss << " lw " << line_width;
    }
    // Add line style if specified.
    if (!line_type.empty()) {
        oss << " " << line_type;
    }

    // Send the constructed command to Gnuplot for execution
    this->send_cmd(oss.str());

    return *this;
}

auto Gnuplot::plot_horizontal_line(double y) -> Gnuplot &
{
    // Check if the Gnuplot session is ready.
    if (!this->is_ready()) {
        std::cerr << "Error: Invalid Gnuplot session not ready.\n";
        return *this;
    }

    std::ostringstream oss;

    // Construct the arrow command for the horizontal line
    oss << "set arrow from graph 0, first " << y << " to graph 1, first " << y << " nohead ";

    // Include line color if it is specified.
    if (line_color.is_set()) {
        oss << " lc rgbcolor \"" << line_color.to_string() << "\"";
    } else {
        oss << " lc rgbcolor \"black\"";
    }

    // Add line width if specified.
    if (line_width > 0) {
        oss << " lw " << line_width;
    }
    // Add line style if specified.
    if (!line_type.empty()) {
        oss << " " << line_type;
    }

    // Send the constructed command to Gnuplot for execution
    this->send_cmd(oss.str());

    return *this;
}

auto Gnuplot::plot_vertical_range(double x, double y_min, double y_max) -> Gnuplot &
{
    // Check if the Gnuplot session is ready.
    if (!this->is_ready()) {
        std::cerr << "Error: Invalid Gnuplot session not ready.\n";
        return *this;
    }

    std::ostringstream oss;

    // Construct the command for the vertical line over a range
    oss << "set arrow from " << x << ", first " << y_min << " to " << x << ", first " << y_max << " nohead ";

    // Include line color if it is specified.
    if (line_color.is_set()) {
        oss << " lc rgbcolor \"" << line_color.to_string() << "\"";
    } else {
        oss << " lc rgbcolor \"black\"";
    }

    // Add line style options if specified
    if (line_width > 0) {
        oss << " lw " << line_width;
    }
    if (!line_type.empty()) {
        oss << " " << line_type;
    }

    // Send the constructed command to Gnuplot for execution
    this->send_cmd(oss.str());

    return *this;
}

auto Gnuplot::plot_horizontal_range(double y, double x_min, double x_max) -> Gnuplot &
{
    // Check if the Gnuplot session is ready.
    if (!this->is_ready()) {
        std::cerr << "Error: Invalid Gnuplot session not ready.\n";
        return *this;
    }

    std::ostringstream oss;

    // Construct the command for the horizontal line over a range
    oss << "set arrow from " << x_min << ", first " << y << " to " << x_max << ", first " << y << " nohead ";

    // Include line color if it is specified.
    if (line_color.is_set()) {
        oss << " lc rgbcolor \"" << line_color.to_string() << "\"";
    } else {
        oss << " lc rgbcolor \"black\"";
    }

    // Add line style options if specified
    if (line_width > 0) {
        oss << " lw " << line_width;
    }
    if (!line_type.empty()) {
        oss << " " << line_type;
    }

    // Send the constructed command to Gnuplot for execution
    this->send_cmd(oss.str());

    return *this;
}

auto Gnuplot::add_label(
    double x,
    double y,
    const std::string &label,
    double font_size,
    const std::string &color,
    double offset_x,
    double offset_y,
    halign_t alignment,
    double rotation,
    bool _point_type,
    const box_style_t &box_style) -> Gnuplot &
{
    // Construct the label command string
    std::ostringstream oss;

    int box_style_id = -1;

    // Optionally add a box style (enclose label in a box).
    if (box_style.show) {
        // Generate the box style id.
        box_style_id = id_manager_textbox_style.generate_unique_id();
        // Generate the style.
        this->send_cmd(box_style.get_declaration(box_style_id));
    }

    oss << "set label \"" << label << "\" at " << x << "," << y;

    // Add horizontal alignment
    if (alignment == halign_t::left) {
        oss << " left";
    } else if (alignment == halign_t::right) {
        oss << " right";
    } else {
        oss << " center"; // default center
    }

    // Add rotation if specified.
    if (!gpcpp::are_equal(rotation, 0.0)) {
        oss << " rotate by " << rotation;
    }

    // Add font size
    oss << " font \", " << font_size << "\"";

    // Add color
    oss << " textcolor rgb \"" << color << "\"";

    // Optionally add a point style (showing a point at the label)
    if (_point_type) {
        oss << " point";
    } else {
        oss << " nopoint";
    }

    // Add offset if specified
    if (!gpcpp::are_equal(offset_x, 0.0) || !gpcpp::are_equal(offset_y, 0.0)) {
        oss << " offset " << offset_x << "," << offset_y;
    }

    if (box_style.show) {
        oss << " boxed bs " << box_style_id;
    }

    // Send the constructed command to Gnuplot.
    this->send_cmd(oss.str());

    return *this;
}

template <typename X>
auto Gnuplot::plot_x(const X &x, const std::string &title) -> Gnuplot &
{
    // Check if the Gnuplot session is ready.
    if (!this->is_ready()) {
        std::cerr << "Error: Invalid Gnuplot session not ready.\n";
        return *this;
    }

    // Check if the input vector is empty.
    if (x.empty()) {
        std::cerr << "Error: Input vector is empty. Cannot plot data." << '\n';
        return *this;
    }

    // Create a temporary file for storing the data
    std::ofstream file;
    std::string filename = this->create_tmpfile(file);
    if (filename.empty()) {
        std::cerr << "Error: Temporary file filename is empty. File creation failed." << '\n';
        return *this;
    }

    // Write the data to the temporary file
    for (size_t i = 0; i < x.size(); ++i) {
        if (!(file << x[i] << '\n')) {
            file.close();
            std::cerr << "Error: Failed to write data to the temporary file: " << filename << '\n';
            return *this;
        }
    }

    // Ensure the file buffer is flushed
    file.flush();
    if (file.fail()) {
        file.close();
        std::cerr << "Error: Failed to flush data to the temporary file: " << filename << '\n';
        return *this;
    }
    file.close();

    // Check if the file is available for reading.
    if (!gpcpp::Gnuplot::file_ready(filename)) {
        std::cerr << "Error: File " << filename << " is not available for reading." << '\n';
        return *this;
    }

    std::ostringstream oss;
    // Determine whether to use 'plot' or 'replot' based on the current plot state.
    oss << ((nplots > 0 && two_dim) ? "replot" : "plot");
    // Specify the file and columns for the Gnuplot command.
    oss << " \"" << filename << "\" using 1";
    // Add a title or specify 'notitle' if no title is provided.
    oss << (title.empty() ? " notitle " : " title \"" + title + "\"");
    // Specify the plot style or smoothing option.
    if (smooth_type == smooth_type_t::none) {
        oss << " with " << plot_type_to_string(plot_type);
    } else {
        oss << " smooth " << smooth_type_to_string(smooth_type);
    }
    // Include line color if it is specified.
    if (line_color.is_set()) {
        oss << " lc rgbcolor \"" << line_color.to_string() << "\"";
    }
    // Add line style options only if the plot style supports lines.
    if (is_line_type(plot_type)) {
        // Add line width if specified.
        if (line_width > 0) {
            oss << " lw " << line_width;
        }
        // Add line style if specified.
        if (!line_type.empty()) {
            oss << " " << line_type;
        }
    }
    // Add point style and size only if the plot style supports points.
    if (is_point_type(plot_type)) {
        // Add point style if specified.
        oss << " pt " << point_type_to_string(point_type);
        // Add point size if specified.
        if (point_size > 0) {
            oss << " ps " << point_size;
        }
    }
    // Send the constructed command to Gnuplot for execution
    this->send_cmd(oss.str());

    return *this;
}

template <typename X>
auto Gnuplot::plot_x(const std::vector<X> &datasets, const std::vector<std::string> &titles) -> Gnuplot &
{
    // Check if the Gnuplot session is ready
    if (!this->is_ready()) {
        std::cerr << "Error: Invalid Gnuplot session not ready.\n";
        return *this;
    }

    // Validate input data
    if (datasets.empty()) {
        std::cerr << "Error: Input datasets are empty. Cannot plot.\n";
        return *this;
    }

    if (!titles.empty() && titles.size() != datasets.size()) {
        std::cerr << "Error: Mismatch between the number of datasets and titles.\n";
        return *this;
    }

    std::vector<std::string> filenames;
    filenames.reserve(datasets.size());

    // Create temporary files for each dataset
    for (size_t i = 0; i < datasets.size(); ++i) {
        if (datasets[i].empty()) {
            std::cerr << "Error: Dataset " << i + 1 << " is empty. Skipping.\n";
            continue;
        }

        std::ofstream file;
        std::string filename = this->create_tmpfile(file);
        if (filename.empty()) {
            std::cerr << "Error: Temporary file creation failed for dataset " << i + 1 << ". Skipping.\n";
            continue;
        }

        for (const auto &value : datasets[i]) {
            if (!(file << value << '\n')) {
                std::cerr << "Error: Failed to write data to temporary file: " << filename << ". Skipping dataset.\n";
                file.close();
                continue;
            }
        }

        file.flush();
        if (file.fail()) {
            std::cerr << "Error: Failed to flush data to temporary file: " << filename << ". Skipping dataset.\n";
            file.close();
            continue;
        }
        file.close();

        filenames.push_back(filename);
    }

    if (filenames.empty()) {
        std::cerr << "Error: No valid datasets to plot.\n";
        return *this;
    }

    // Check if the file is available for reading.
    for (const auto &filename : filenames) {
        if (!gpcpp::Gnuplot::file_ready(filename)) {
            std::cerr << "Error: File " << filename << " is not available for reading.\n";
            return *this;
        }
    }

    std::ostringstream oss;

    // Determine the command ('plot' or 'replot')
    oss << ((nplots > 0 && two_dim) ? "replot " : "plot ");

    // Construct the plotting command for each dataset
    for (size_t i = 0; i < filenames.size(); ++i) {
        oss << "\"" << filenames[i] << "\" using 1";

        // Add title
        if (titles.empty() || titles[i].empty()) {
            oss << " notitle ";
        } else {
            oss << " title \"" << titles[i] << "\" ";
        }

        // Specify plot style or smoothing
        if (smooth_type == smooth_type_t::none) {
            oss << " with " << plot_type_to_string(plot_type);
        } else {
            oss << " smooth " << smooth_type_to_string(smooth_type);
        }

        // Add line options if applicable
        if (is_line_type(plot_type)) {
            if (line_width > 0) {
                oss << " lw " << line_width;
            }
            if (line_color.is_set()) {
                oss << " lc rgbcolor \"" << line_color.to_string() << "\"";
            }
        }

        // Add point options if applicable
        if (is_point_type(plot_type)) {
            oss << " pt " << point_type_to_string(point_type);
            if (point_size > 0) {
                oss << " ps " << point_size;
            }
        }

        // Add a comma unless it's the last dataset
        if (i != filenames.size() - 1) {
            oss << ", ";
        }
    }

    // Send the constructed command to Gnuplot
    this->send_cmd(oss.str());

    return *this;
}

template <typename X, typename Y>
auto Gnuplot::plot_xy(const X &x, const Y &y, const std::string &title) -> Gnuplot &
{
    // Check if the Gnuplot session is ready
    if (!this->is_ready()) {
        std::cerr << "Error: Invalid Gnuplot session. Cannot plot.\n";
        return *this;
    }

    // Validate input vectors
    if (x.empty() || y.empty()) {
        std::cerr << "Error: Input vectors are empty. Cannot plot.\n";
        return *this;
    }

    if (x.size() != y.size()) {
        std::cerr << "Error: Mismatch between the lengths of x and y vectors.\n";
        return *this;
    }

    // Create a temporary file for storing the data
    std::ofstream file;
    std::string filename = this->create_tmpfile(file);
    if (filename.empty()) {
        std::cerr << "Error: Failed to create a temporary file.\n";
        return *this;
    }

    // Write the data to the temporary file
    for (size_t i = 0; i < x.size(); ++i) {
        if (!(file << x[i] << " " << y[i] << '\n')) {
            std::cerr << "Error: Failed to write data to the temporary file: " << filename << '\n';
            file.close();
            return *this;
        }
    }

    // Flush the file buffer and close the file
    file.flush();
    if (file.fail()) {
        std::cerr << "Error: Failed to flush data to the temporary file: " << filename << '\n';
        file.close();
        return *this;
    }
    file.close();

    // Check if the file is available for reading
    if (!gpcpp::Gnuplot::file_ready(filename)) {
        std::cerr << "Error: File " << filename << " is not available for reading.\n";
        return *this;
    }
    std::ostringstream oss;
    // Determine whether to use 'plot' or 'replot' based on the current plot state
    oss << ((nplots > 0 && two_dim) ? "replot" : "plot");
    // Specify the file and columns for the Gnuplot command
    oss << " \"" << filename << "\" using 1:2";
    // Add a title or specify 'notitle' if no title is provided
    oss << (title.empty() ? " notitle" : " title \"" + title + "\"");
    // Specify the plot style or smoothing option.
    if (smooth_type == smooth_type_t::none) {
        oss << " with " << plot_type_to_string(plot_type);
    } else {
        oss << " smooth " << smooth_type_to_string(smooth_type);
    }
    // Include line color if it is specified.
    if (line_color.is_set()) {
        oss << " lc rgbcolor \"" << line_color.to_string() << "\"";
    }
    // Add line style options only if the plot style supports lines.
    if (is_line_type(plot_type)) {
        // Add line width if specified.
        if (line_width > 0) {
            oss << " lw " << line_width;
        }
        // Add line style if specified.
        if (!line_type.empty()) {
            oss << " " << line_type;
        }
    }
    // Add point style and size only if the plot style supports points.
    if (is_point_type(plot_type)) {
        // Add point style if specified.
        oss << " pt " << point_type_to_string(point_type);
        // Add point size if specified.
        if (point_size > 0) {
            oss << " ps " << point_size;
        }
    }
    // Send the constructed command to Gnuplot for execution
    this->send_cmd(oss.str());

    return *this;
}

template <typename X, typename Y, typename E>
auto Gnuplot::plot_xy_erorrbar(const X &x, const Y &y, const E &dy, erorrbar_type_t style, const std::string &title)
    -> Gnuplot &
{
    // Check if the Gnuplot session is ready
    if (!this->is_ready()) {
        std::cerr << "Error: Invalid Gnuplot session. Cannot plot.\n";
        return *this;
    }

    // Validate input vectors
    if (x.empty() || y.empty() || dy.empty()) {
        std::cerr << "Error: Input vectors are empty. Cannot plot.\n";
        return *this;
    }

    if (x.size() != y.size() || x.size() != dy.size()) {
        std::cerr << "Error: Mismatch between the lengths of x, y, and dy vectors.\n";
        return *this;
    }

    // Create a temporary file for storing the data
    std::ofstream file;
    std::string filename = this->create_tmpfile(file);
    if (filename.empty()) {
        std::cerr << "Error: Failed to create a temporary file.\n";
        return *this;
    }

    // Write the data to the temporary file
    for (size_t i = 0; i < x.size(); ++i) {
        if (!(file << x[i] << " " << y[i] << " " << dy[i] << '\n')) {
            std::cerr << "Error: Failed to write data to the temporary file: " << filename << '\n';
            file.close();
            return *this;
        }
    }

    // Flush the file buffer and close the file
    file.flush();
    if (file.fail()) {
        std::cerr << "Error: Failed to flush data to the temporary file: " << filename << '\n';
        file.close();
        return *this;
    }
    file.close();

    // Check if the file is available for reading
    if (!gpcpp::Gnuplot::file_ready(filename)) {
        std::cerr << "Error: File " << filename << " is not available for reading.\n";
        return *this;
    }

    std::ostringstream oss;

    // Determine whether to use 'plot' or 'replot' based on the current plot state
    oss << ((nplots > 0 && two_dim) ? "replot " : "plot ");

    // Specify the file and columns for the Gnuplot command
    oss << "\"" << filename << "\" using 1:2:3 with " << errorbars_to_string(style) << " ";

    // Add a title or specify 'notitle' if no title is provided
    oss << (title.empty() ? " notitle " : " title \"" + title + "\" ");

    // Include line color if it is specified.
    if (line_color.is_set()) {
        oss << " lc rgbcolor \"" << line_color.to_string() << "\"";
    }

    // Add line width if specified.
    if (line_width > 0) {
        oss << " lw " << line_width;
    }
    // Add line style if specified.
    if (!line_type.empty()) {
        oss << " " << line_type;
    }

    // Add point style if specified.
    oss << " pt " << point_type_to_string(point_type);
    // Add point size if specified.
    if (point_size > 0) {
        oss << " ps " << point_size;
    }

    // Send the constructed command to Gnuplot for execution
    this->send_cmd(oss.str());

    return *this;
}

template <typename X, typename Y, typename Z>
auto Gnuplot::plot_xyz(const X &x, const Y &y, const Z &z, const std::string &title) -> Gnuplot &
{
    // Check if the Gnuplot session is ready
    if (!this->is_ready()) {
        std::cerr << "Error: Invalid Gnuplot session. Cannot plot.\n";
        return *this;
    }

    // Validate input vectors
    if (x.empty() || y.empty() || z.empty()) {
        std::cerr << "Error: Input vectors are empty. Cannot plot.\n";
        return *this;
    }

    if (x.size() != y.size() || x.size() != z.size()) {
        std::cerr << "Error: Mismatch between the lengths of x, y, and z vectors.\n";
        return *this;
    }

    // Create a temporary file for storing the data
    std::ofstream file;
    std::string filename = this->create_tmpfile(file);
    if (filename.empty()) {
        std::cerr << "Error: Failed to create a temporary file.\n";
        return *this;
    }

    // Write the data to the temporary file
    for (size_t i = 0; i < x.size(); ++i) {
        if (!(file << x[i] << " " << y[i] << " " << z[i] << '\n')) {
            std::cerr << "Error: Failed to write data to the temporary file: " << filename << '\n';
            file.close();
            return *this;
        }
    }

    // Flush the file buffer and close the file
    file.flush();
    if (file.fail()) {
        std::cerr << "Error: Failed to flush data to the temporary file: " << filename << '\n';
        file.close();
        return *this;
    }
    file.close();

    // Check if the file is available for reading
    if (!gpcpp::Gnuplot::file_ready(filename)) {
        std::cerr << "Error: File " << filename << " is not available for reading.\n";
        return *this;
    }

    std::ostringstream oss;

    // Determine whether to use 'splot' or 'replot' based on the current plot state
    oss << ((nplots > 0 && !two_dim) ? "replot" : "splot");

    // Specify the file and columns for the Gnuplot command
    oss << " \"" << filename << "\" using 1:2:3";

    // Add a title or specify 'notitle' if no title is provided
    oss << (title.empty() ? " notitle" : " title \"" + title + "\"");

    // Specify the plot style or smoothing option.
    if (smooth_type == smooth_type_t::none) {
        oss << " with " << plot_type_to_string(plot_type);
    } else {
        oss << " smooth " << smooth_type_to_string(smooth_type);
    }

    // Include line color if it is specified.
    if (line_color.is_set()) {
        oss << " lc rgbcolor \"" << line_color.to_string() << "\"";
    }

    // Add line style options only if the plot style supports lines.
    if (is_line_type(plot_type)) {
        // Add line width if specified.
        if (line_width > 0) {
            oss << " lw " << line_width;
        }
        // Add line style if specified.
        if (!line_type.empty()) {
            oss << " " << line_type;
        }
    }

    // Add point style and size only if the plot style supports points.
    if (is_point_type(plot_type)) {
        // Add point style if specified.
        oss << " pt " << point_type_to_string(point_type);
        // Add point size if specified.
        if (point_size > 0) {
            oss << " ps " << point_size;
        }
    }

    // Send the constructed command to Gnuplot for execution
    this->send_cmd(oss.str());

    return *this;
}

template <typename X, typename Y, typename Z>
auto Gnuplot::plot_3d_grid(const X &x, const Y &y, const Z &z, const std::string &title) -> Gnuplot &
{
    // Check if the Gnuplot session is ready
    if (!this->is_ready()) {
        std::cerr << "Error: Invalid Gnuplot session. Cannot plot.\n";
        return *this;
    }

    // Validate input dimensions
    if (x.empty() || y.empty() || z.empty()) {
        std::cerr << "Error: Input vectors must not be empty.\n";
        return *this;
    }
    if (z.size() != x.size() || z[0].size() != y.size()) {
        std::cerr << "Error: Dimensions of z must match sizes of x and y.\n";
        return *this;
    }

    // Create a temporary file for storing the grid data
    std::ofstream file;
    std::string filename = this->create_tmpfile(file);
    if (filename.empty()) {
        std::cerr << "Error: Failed to create a temporary file.\n";
        return *this;
    }

    // Write the grid data to the temporary file
    for (size_t i = 0; i < x.size(); ++i) {
        for (size_t j = 0; j < y.size(); ++j) {
            if (!(file << x[i] << " " << y[j] << " " << z[i][j] << '\n')) {
                std::cerr << "Error: Failed to write data to the temporary file: " << filename << '\n';
                file.close();
                return *this;
            }
        }
        file << "\n"; // Separate rows for Gnuplot
    }

    // Flush and close the file
    file.flush();
    if (file.fail()) {
        std::cerr << "Error: Failed to flush data to the temporary file: " << filename << '\n';
        file.close();
        return *this;
    }
    file.close();

    // Check if the file is available for reading
    if (!gpcpp::Gnuplot::file_ready(filename)) {
        std::cerr << "Error: File " << filename << " is not available for reading.\n";
        return *this;
    }

    std::ostringstream oss;

    // Determine whether to use 'splot' or 'replot' based on the current plot state
    oss << ((nplots > 0 && !two_dim) ? "replot" : "splot");

    // Specify the file and columns for the Gnuplot command
    oss << " \"" << filename << "\" using 1:2:3";

    // Add a title or specify 'notitle' if no title is provided
    oss << (title.empty() ? " notitle" : " title \"" + title + "\"");

    // Specify the plot style or smoothing option.
    if (smooth_type == smooth_type_t::none) {
        oss << " with " << plot_type_to_string(plot_type);
    } else {
        oss << " smooth " << smooth_type_to_string(smooth_type);
    }

    // Include line color if it is specified.
    if (line_color.is_set()) {
        oss << " lc rgbcolor \"" << line_color.to_string() << "\"";
    }

    // Add line style options only if the plot style supports lines.
    if (is_line_type(plot_type)) {
        // Add line width if specified.
        if (line_width > 0) {
            oss << " lw " << line_width;
        }
        // Add line style if specified.
        if (!line_type.empty()) {
            oss << " " << line_type;
        }
    }

    // Add point style and size only if the plot style supports points.
    if (is_point_type(plot_type)) {
        // Add point style if specified.
        oss << " pt " << point_type_to_string(point_type);
        // Add point size if specified.
        if (point_size > 0) {
            oss << " ps " << point_size;
        }
    }

    // Send the constructed command to Gnuplot for execution
    this->send_cmd(oss.str());
    return *this;
}

auto Gnuplot::plot_slope(const double a, const double b, const std::string &title) -> Gnuplot &
{
    std::ostringstream oss;

    // Determine whether to use 'plot' or 'replot' based on the current plot state.
    oss << ((nplots > 0 && two_dim) ? "replot " : "plot ");

    // Add the equation for the slope to the plot command.
    oss << " " << a << " * x + " << b << " ";

    // Specify the equation and title for the plot.
    if (title.empty()) {
        oss << "title \"f(x) = " << a << " * x + " << b << "\"";
    } else {
        oss << "title \"" << title << "\"";
    }

    // Specify the plot style or smoothing option.
    if (smooth_type == smooth_type_t::none) {
        oss << " with " << plot_type_to_string(plot_type);
    } else {
        oss << " smooth " << smooth_type_to_string(smooth_type);
    }

    // Include line color if it is specified.
    if (line_color.is_set()) {
        oss << " lc rgbcolor \"" << line_color.to_string() << "\"";
    }

    // Add line style options only if the plot style supports lines.
    if (is_line_type(plot_type)) {
        // Add line width if specified.
        if (line_width > 0) {
            oss << " lw " << line_width;
        }
        // Add line style if specified.
        if (!line_type.empty()) {
            oss << " " << line_type;
        }
    }

    // Add point style and size only if the plot style supports points.
    if (is_point_type(plot_type)) {
        // Add point style if specified.
        oss << " pt " << point_type_to_string(point_type);
        // Add point size if specified.
        if (point_size > 0) {
            oss << " ps " << point_size;
        }
    }

    // Send the constructed command to Gnuplot for execution
    this->send_cmd(oss.str());

    return *this;
}

auto Gnuplot::plot_equation(const std::string &equation, const std::string &title) -> Gnuplot &
{
    std::ostringstream oss;

    // Determine whether to use 'plot' or 'replot' based on the current state.
    oss << ((nplots > 0 && two_dim) ? "replot " : "plot ");

    // Add the equation to be plotted.
    oss << equation;

    // Set the title or use 'notitle' if no title is provided.
    oss << (title.empty() ? " notitle" : " title \"" + title + "\"");

    // Specify the plot style or smoothing option.
    if (smooth_type == smooth_type_t::none) {
        oss << " with " << plot_type_to_string(plot_type);
    } else {
        oss << " smooth " << smooth_type_to_string(smooth_type);
    }

    // Include line color if it is specified.
    if (line_color.is_set()) {
        oss << " lc rgbcolor \"" << line_color.to_string() << "\"";
    }

    // Add line style options only if the plot style supports lines.
    if (is_line_type(plot_type)) {
        // Add line width if specified.
        if (line_width > 0) {
            oss << " lw " << line_width;
        }
        // Add line style if specified.
        if (!line_type.empty()) {
            oss << " " << line_type;
        }
    }

    // Add point style and size only if the plot style supports points.
    if (is_point_type(plot_type)) {
        // Add point style if specified.
        oss << " pt " << point_type_to_string(point_type);
        // Add point size if specified.
        if (point_size > 0) {
            oss << " ps " << point_size;
        }
    }

    // Send the constructed command to Gnuplot for execution.
    this->send_cmd(oss.str());
    return *this;
}

auto Gnuplot::plot_equation3d(const std::string &equation, const std::string &title) -> Gnuplot &
{
    std::ostringstream oss;

    // Determine whether to use 'splot' or 'replot' based on the current state.
    oss << ((nplots > 0 && !two_dim) ? "replot " : "splot ");

    // Add the equation.
    oss << equation;

    // Add the title or use a default title format.
    if (title.empty()) {
        oss << " title \"f(x, y) = " << equation << "\"";
    } else {
        oss << " title \"" << title << "\"";
    }

    // Specify the plot style or smoothing option.
    if (smooth_type == smooth_type_t::none) {
        oss << " with " << plot_type_to_string(plot_type);
    } else {
        oss << " smooth " << smooth_type_to_string(smooth_type);
    }

    // Include line color if it is specified.
    if (line_color.is_set()) {
        oss << " lc rgbcolor \"" << line_color.to_string() << "\"";
    }

    // Add line style options only if the plot style supports lines.
    if (is_line_type(plot_type)) {
        // Add line width if specified.
        if (line_width > 0) {
            oss << " lw " << line_width;
        }
        // Add line style if specified.
        if (!line_type.empty()) {
            oss << " " << line_type;
        }
    }

    // Add point style and size only if the plot style supports points.
    if (is_point_type(plot_type)) {
        // Add point style if specified.
        oss << " pt " << point_type_to_string(point_type);
        // Add point size if specified.
        if (point_size > 0) {
            oss << " ps " << point_size;
        }
    }

    // Send the constructed command to Gnuplot for execution.
    this->send_cmd(oss.str());
    return *this;
}

auto Gnuplot::plot_image(
    const unsigned char *ucPicBuf,
    const unsigned int iWidth,
    const unsigned int iHeight,
    const std::string &title) -> Gnuplot &
{
    // Create a temporary file to store image data
    std::ofstream file;
    std::string filename = create_tmpfile(file);
    if (filename.empty()) {
        std::cerr << "Error: Failed to create a temporary file for image plotting." << '\n';
        return *this; // Early return on failure
    }

    // Write the image data (width, height, pixel value) to the temporary file
    int iIndex         = 0;
    bool write_success = true;
    for (unsigned int iRow = 0; iRow < iHeight && write_success; ++iRow) {
        for (unsigned int iColumn = 0; iColumn < iWidth; ++iColumn) {
            if (!(file << iColumn << " " << iRow << " " << static_cast<float>(ucPicBuf[iIndex++]) << '\n')) {
                std::cerr << "Error: Failed to write image data to temporary file: " << filename << '\n';
                write_success = false;
                break;
            }
        }
    }

    // Ensure all data is written to the file and the file is closed properly
    file.flush();
    if (file.fail() || !write_success) {
        file.close();
        std::cerr << "Error: Failed to flush data to temporary file: " << filename << '\n';
        return *this; // Early return on failure
    }
    file.close();

    // Check if the file is available for reading
    if (gpcpp::Gnuplot::file_ready(filename)) {
        // Construct the Gnuplot command for plotting the image
        std::ostringstream oss;
        // Determine whether to use 'plot' or 'replot' based on the current plot state
        oss << ((nplots > 0 && two_dim) ? "replot " : "plot ");
        // Specify the file and plotting options
        oss << "\"" << filename << "\" with image";
        if (!title.empty()) {
            oss << " title \"" << title << "\"";
        }
        // Send the constructed command to Gnuplot for execution
        this->send_cmd(oss.str());
    }
    return *this;
}

auto Gnuplot::set_gnuplot_path(const std::string &path) -> bool
{
    std::string tmp = path + "/" + Gnuplot::m_gnuplot_filename;

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__TOS_WIN__)
    if (Gnuplot::file_exists(tmp, 0)) // check existence
#elif defined(unix) || defined(__unix) || defined(__unix__) || defined(__APPLE__)
    if (Gnuplot::file_exists(tmp, 1)) // check existence and execution permission
#endif
    {
        Gnuplot::m_gnuplot_path = path;
        return true;
    }
    Gnuplot::m_gnuplot_path.clear();
    return false;
}

auto Gnuplot::set_terminal(terminal_type_t type) -> Gnuplot &
{
    // For Unix-like systems, ensure the DISPLAY variable is set when using X11.
#if defined(unix) || defined(__unix) || defined(__unix__) || defined(__APPLE__)
    if ((type == terminal_type_t::x11) && (getenv("DISPLAY") == nullptr)) {
        std::cerr << "Error: Can't find DISPLAY environment variable. Ensure an active X11 session.\n";
        return *this;
    }
#endif

    // Update the standard terminal type
    terminal_type = type;
    return *this;
}

/// Tokenizes a string into a container based on the specified delimiters.
///
/// @tparam Container The type of the container to store tokens (e.g., std::vector<std::string>).
/// @param container Reference to the container where tokens will be stored.
/// @param in The input string to tokenize.
/// @param delimiters A string containing delimiter characters (default is whitespace).
template <typename Container>
static inline void tokenize(Container &container, const std::string &in, const std::string &delimiters = " \t\n")
{
    std::stringstream ss(in);
    std::string token;

    // Use a custom delimiter iterator if delimiters are more than whitespace.
    if (delimiters == " \t\n") {
        while (ss >> token) {
            container.push_back(token);
        }
    } else {
        std::string::size_type start = 0;
        while ((start = in.find_first_not_of(delimiters, start)) != std::string::npos) {
            auto end = in.find_first_of(delimiters, start);
            container.push_back(in.substr(start, end - start));
            start = end;
        }
    }
}

auto Gnuplot::replot() -> Gnuplot &
{
    if (nplots > 0) {
        this->send_cmd("replot");
    }
    return *this;
}

auto Gnuplot::is_ready() const -> bool { return valid && (gnuplot_pipe != nullptr); }

auto Gnuplot::reset_plot() -> Gnuplot &
{
    nplots = 0;
    return *this;
}

auto Gnuplot::reset_all() -> Gnuplot &
{
    nplots = 0;
    this->send_cmd("reset");
    this->send_cmd("clear");
    plot_type   = plot_type_t::none;
    smooth_type = smooth_type_t::none;
    id_manager_textbox_style.clear();
    id_manager_line_style.clear();
    grid_major_style_id = -1;
    grid_minor_style_id = -1;
    return *this;
}

auto Gnuplot::set_plot_type(plot_type_t style) -> Gnuplot &
{
    plot_type = style;
    return *this;
}

auto Gnuplot::set_smooth_type(smooth_type_t style) -> Gnuplot &
{
    smooth_type = style;
    return *this;
}

auto Gnuplot::set_line_type(line_type_t style, const std::string &custom_pattern) -> Gnuplot &
{
    line_type = line_type_to_string(style, custom_pattern);
    return *this;
}

auto Gnuplot::set_line_color(const std::string &color) -> Gnuplot &
{
    line_color = gpcpp::Color(color);
    return *this;
}

auto Gnuplot::set_line_color(int r, int g, int b) -> Gnuplot &
{
    line_color = gpcpp::Color(r, g, b);
    return *this;
}

auto Gnuplot::set_point_type(point_type_t style) -> Gnuplot &
{
    point_type = style;
    return *this;
}

auto Gnuplot::set_point_size(double size) -> Gnuplot &
{
    if (size > 0) {
        point_size = size;
    }
    return *this;
}

auto Gnuplot::show() -> Gnuplot &
{
    this->send_cmd("set output");
    this->send_cmd("set terminal " + terminal_type_to_string(terminal_type));

    fflush(gnuplot_pipe);

    // Wait for user input before closing.
    std::cout << "Press Enter to continue..." << '\n';
    std::cin.get();

    return *this;
}

auto Gnuplot::set_output(const std::string &filename) -> Gnuplot &
{
    // Set the output file where the plot will be saved
    this->send_cmd("set output \"" + filename + "\"");
    this->send_cmd("set terminal " + terminal_type_to_string(terminal_type));
    return *this;
}

auto Gnuplot::set_legend(
    const std::string &position,
    const std::string &font,
    const std::string &title,
    bool with_box,
    double spacing,
    double width) -> Gnuplot &
{
    if (position.empty()) {
        this->send_cmd("unset key");
        return *this;
    }

    std::ostringstream oss;

    // Set the legend position.
    oss << "set key " << position;

    // Set the legend title, if provided.
    if (!title.empty()) {
        oss << " title \"" << title << "\"";
    }

    // Set the font, if provided.
    if (!font.empty()) {
        oss << " font \"" << font << "\"";
    }

    // Add box around the legend if specified.
    if (with_box) {
        oss << " box";
    } else {
        oss << " nobox";
    }

    // Set the spacing between legend items if specified.
    if (spacing > 0) {
        oss << " spacing " << spacing;
    }

    // Set the width of the legend box.
    if (width > 0) {
        oss << " width " << width;
    }

    // Send the command to Gnuplot
    this->send_cmd(oss.str());

    return *this;
}

auto Gnuplot::set_title(const std::string &title) -> Gnuplot &
{
    // Send the command to Gnuplot
    this->send_cmd("set title \"" + title + "\"");
    return *this;
}

auto Gnuplot::unset_title() -> Gnuplot &
{
    this->set_title();
    return *this;
}

auto Gnuplot::set_xlogscale(const double base) -> Gnuplot &
{
    this->send_cmd("set logscale x " + std::to_string(base));
    return *this;
}

auto Gnuplot::set_ylogscale(const double base) -> Gnuplot &
{
    std::ostringstream cmdstr;

    cmdstr << "set logscale y " << base;
    this->send_cmd(cmdstr.str());

    return *this;
}

auto Gnuplot::set_zlogscale(const double base) -> Gnuplot &
{
    std::ostringstream cmdstr;

    cmdstr << "set logscale z " << base;
    this->send_cmd(cmdstr.str());

    return *this;
}

auto Gnuplot::unset_xlogscale() -> Gnuplot &
{
    this->send_cmd("unset logscale x");
    return *this;
}

auto Gnuplot::unset_ylogscale() -> Gnuplot &
{
    this->send_cmd("unset logscale y");
    return *this;
}

auto Gnuplot::unset_zlogscale() -> Gnuplot &
{
    this->send_cmd("unset logscale z");
    return *this;
}

auto Gnuplot::set_line_width(double width) -> Gnuplot &
{
    if (width > 0) {
        line_width = width;
    }
    return *this;
}

/// turns grid on/off
auto Gnuplot::set_grid() -> Gnuplot &
{
    this->send_cmd("set grid");
    return *this;
}

/// @brief Sets the major tics for the x-axis.
auto Gnuplot::set_xtics_major(double major_step) -> Gnuplot &
{
    if (major_step <= 0) {
        throw std::invalid_argument("Major step size for x-axis must be positive.");
    }

    std::string cmd = "set xtics " + std::to_string(major_step);
    this->send_cmd(cmd);
    return *this;
}

/// @brief Sets the minor tics for the x-axis.
auto Gnuplot::set_xtics_minor(int minor_intervals) -> Gnuplot &
{
    if (minor_intervals <= 0) {
        throw std::invalid_argument("Number of minor intervals for x-axis must be positive.");
    }

    std::string cmd = "set mxtics " + std::to_string(minor_intervals);
    this->send_cmd(cmd);
    return *this;
}

/// @brief Sets the major tics for the y-axis.
auto Gnuplot::set_ytics_major(double major_step) -> Gnuplot &
{
    if (major_step <= 0) {
        throw std::invalid_argument("Major step size for y-axis must be positive.");
    }

    std::string cmd = "set ytics " + std::to_string(major_step);
    this->send_cmd(cmd);
    return *this;
}

/// @brief Sets the minor tics for the y-axis.
auto Gnuplot::set_ytics_minor(int minor_intervals) -> Gnuplot &
{
    if (minor_intervals <= 0) {
        throw std::invalid_argument("Number of minor intervals for y-axis must be positive.");
    }

    std::string cmd = "set mytics " + std::to_string(minor_intervals);
    this->send_cmd(cmd);
    return *this;
}

auto Gnuplot::set_grid_line_type(
    grid_type_t grid_type,
    line_type_t style,
    const Color &color,
    double width,
    const std::string &custom_dash) -> Gnuplot &
{
    if ((grid_type == grid_type_t::major) && (grid_major_style_id < 0)) {
        grid_major_style_id = id_manager_line_style.generate_unique_id();
    }
    if ((grid_type == grid_type_t::minor) && (grid_minor_style_id < 0)) {
        grid_minor_style_id = id_manager_line_style.generate_unique_id();
    }

    // Define default linetype (solid)
    int linetype = 1;

    // Define custom dashtype based on the line style
    std::string dashtype;
    switch (style) {
    case line_type_t::solid:
        linetype = 1;
        break;
    case line_type_t::dashed:
        dashtype = "50, 25";
        break;
    case line_type_t::dotted:
        dashtype = "1, 1";
        break;
    case line_type_t::dash_dot:
        dashtype = "10, 5, 1, 5";
        break;
    case line_type_t::dash_dot_dot:
        dashtype = "10, 5, 1, 5, 1, 5";
        break;
    case line_type_t::custom:
        if (!custom_dash.empty()) {
            dashtype = custom_dash; // Use the provided custom dash pattern
        }
        break;
    default:
        break;
    }

    // Build the Gnuplot command
    std::string cmd = "set style line ";
    if (grid_type == grid_type_t::major) {
        cmd += std::to_string(grid_major_style_id);
    } else if (grid_type == grid_type_t::minor) {
        cmd += std::to_string(grid_minor_style_id);
    }
    cmd += " lt " + std::to_string(linetype);
    if (!dashtype.empty()) {
        cmd += " dt (" + dashtype + ")";
    }
    if (color.is_set()) {
        cmd += " lc rgb \"" + color.to_string() + "\"";
    }
    cmd += " lw " + std::to_string(width);

    // Send the command to Gnuplot
    this->send_cmd(cmd);

    return *this;
}

auto Gnuplot::apply_grid(const std::string &tics, const std::string &layer, bool vertical_lines) -> Gnuplot &
{
    std::string cmd = "set grid " + tics;
    if (layer == "front" || layer == "back") {
        cmd += " " + layer;
    }
    // Apply major grid style.
    if (grid_major_style_id > 0) {
        cmd += " ls " + std::to_string(grid_major_style_id);
    }
    // Apply minor grid style.
    if (grid_minor_style_id > 0) {
        cmd += " , ls " + std::to_string(grid_minor_style_id);
    }
    // Vertical lines option.
    if (!vertical_lines) {
        cmd += " novertical";
    }
    this->send_cmd(cmd);
    return *this;
}

auto Gnuplot::unset_grid() -> Gnuplot &
{
    this->send_cmd("unset grid");
    return *this;
}

auto Gnuplot::set_multiplot() -> Gnuplot &
{
    this->send_cmd("set multiplot");
    return *this;
}

auto Gnuplot::unset_multiplot() -> Gnuplot &
{
    this->send_cmd("unset multiplot");
    return *this;
}

auto Gnuplot::set_origin_and_size(double x_origin, double y_origin, double width, double height) -> Gnuplot &
{
    // Set the origin (position) of the plot in the window
    this->send_cmd("set origin " + std::to_string(x_origin) + "," + std::to_string(y_origin));

    // Set the size of the plot area (width and height relative to the window)
    this->send_cmd("set size " + std::to_string(width) + "," + std::to_string(height));

    // Return the current object for method chaining
    return *this;
}

auto Gnuplot::set_samples(const int samples) -> Gnuplot &
{
    std::ostringstream cmdstr;
    cmdstr << "set samples " << samples;
    this->send_cmd(cmdstr.str());

    return *this;
}

auto Gnuplot::set_isosamples(const int isolines) -> Gnuplot &
{
    std::ostringstream cmdstr;
    cmdstr << "set isosamples " << isolines;
    this->send_cmd(cmdstr.str());

    return *this;
}

auto Gnuplot::set_contour_type(contour_type_t type) -> Gnuplot &
{
    contour.type = type;
    return *this;
}

auto Gnuplot::set_contour_param(contour_param_t param) -> Gnuplot &
{
    contour.param = param;
    return *this;
}

auto Gnuplot::set_contour_levels(int levels) -> Gnuplot &
{
    if (levels > 0) {
        contour.levels = levels;
    }
    return *this;
}

auto Gnuplot::set_contour_increment(double start, double step, double end) -> Gnuplot &
{
    contour.increment_start = start;
    contour.increment_step  = step;
    contour.increment_end   = end;
    return *this;
}

auto Gnuplot::set_contour_discrete_levels(const std::vector<double> &levels) -> Gnuplot &
{
    contour.discrete_levels = levels;
    return *this;
}

auto Gnuplot::set_hidden3d() -> Gnuplot &
{
    this->send_cmd("set hidden3d");
    return *this;
}

auto Gnuplot::unset_hidden3d() -> Gnuplot &
{
    this->send_cmd("unset hidden3d");
    return *this;
}

auto Gnuplot::unset_contour() -> Gnuplot &
{
    this->send_cmd("unset contour");
    return *this;
}

auto Gnuplot::set_surface() -> Gnuplot &
{
    this->send_cmd("set surface");
    return *this;
}

auto Gnuplot::unset_surface() -> Gnuplot &
{
    this->send_cmd("unset surface");
    return *this;
}

auto Gnuplot::set_xautoscale() -> Gnuplot &
{
    this->send_cmd("set xrange restore");
    this->send_cmd("set autoscale x");
    return *this;
}

auto Gnuplot::set_yautoscale() -> Gnuplot &
{
    this->send_cmd("set yrange restore");
    this->send_cmd("set autoscale y");
    return *this;
}

auto Gnuplot::set_zautoscale() -> Gnuplot &
{
    this->send_cmd("set zrange restore");
    this->send_cmd("set autoscale z");
    return *this;
}

auto Gnuplot::set_xlabel(const std::string &label) -> Gnuplot &
{
    std::ostringstream cmdstr;

    cmdstr << "set xlabel \"" << label << "\"";
    this->send_cmd(cmdstr.str());

    return *this;
}

auto Gnuplot::set_ylabel(const std::string &label) -> Gnuplot &
{
    std::ostringstream cmdstr;

    cmdstr << "set ylabel \"" << label << "\"";
    this->send_cmd(cmdstr.str());

    return *this;
}

auto Gnuplot::set_zlabel(const std::string &label) -> Gnuplot &
{
    std::ostringstream cmdstr;

    cmdstr << "set zlabel \"" << label << "\"";
    this->send_cmd(cmdstr.str());

    return *this;
}

auto Gnuplot::set_xrange(const double iFrom, const double iTo) -> Gnuplot &
{
    std::ostringstream cmdstr;

    cmdstr << "set xrange[" << iFrom << ":" << iTo << "]";
    this->send_cmd(cmdstr.str());

    return *this;
}

auto Gnuplot::set_yrange(const double iFrom, const double iTo) -> Gnuplot &
{
    std::ostringstream cmdstr;

    cmdstr << "set yrange[" << iFrom << ":" << iTo << "]";
    this->send_cmd(cmdstr.str());

    return *this;
}

auto Gnuplot::set_zrange(const double iFrom, const double iTo) -> Gnuplot &
{
    std::ostringstream cmdstr;

    cmdstr << "set zrange[" << iFrom << ":" << iTo << "]";
    this->send_cmd(cmdstr.str());

    return *this;
}

auto Gnuplot::set_cbrange(const double iFrom, const double iTo) -> Gnuplot &
{
    std::ostringstream cmdstr;

    cmdstr << "set cbrange[" << iFrom << ":" << iTo << "]";
    this->send_cmd(cmdstr.str());

    return *this;
}

auto Gnuplot::get_program_path() -> bool
{
    // Check the first location: `Gnuplot::m_gnuplot_path`
    std::string tmp = Gnuplot::m_gnuplot_path + "/" + Gnuplot::m_gnuplot_filename;

    if (Gnuplot::file_exists(tmp, 1)) { // Check for existence and execution permission
        return true;
    }

    // Check the second location: System PATH
    char *path = getenv("PATH");
    if (path == nullptr) {
        std::cerr << "Error: PATH environment variable is not set.\n";
        return false;
    }

    // Tokenize the PATH variable into directories
    std::list<std::string> paths;
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__TOS_WIN__)
    tokenize(paths, path, ";");
#elif defined(unix) || defined(__unix) || defined(__unix__) || defined(__APPLE__)
    tokenize(paths, path, ":");
#endif

    // Search for the Gnuplot executable in each directory
    for (const auto &dir : paths) {
        tmp = dir + "/" + Gnuplot::m_gnuplot_filename;
        if (Gnuplot::file_exists(tmp, 1)) { // Check for existence and execution permission
            Gnuplot::m_gnuplot_path = dir;  // Set `Gnuplot::m_gnuplot_path`
            return true;
        }
    }

    // Gnuplot was not found
    std::cerr << "Error: Gnuplot not found in PATH or in \"" << Gnuplot::m_gnuplot_path << "\".\n";
    return false;
}

auto Gnuplot::file_exists(const std::string &filename, int mode) -> bool
{
    // Validate mode argument
    if (mode < 0 || mode > 7) {
        std::cerr << "Error: Invalid mode in Gnuplot::file_exists. Mode must be between 0 and 7.\n";
        return false;
    }

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__TOS_WIN__)
    // Windows: Use _access
    if (_access(filename.c_str(), mode) == 0) {
        return true; // File exists with required access
    }
#elif defined(unix) || defined(__unix) || defined(__unix__) || defined(__APPLE__)
    // Unix/Linux/macOS: Use access
    if (access(filename.c_str(), mode) == 0) {
        return true; // File exists with required access
    }
#else
    std::cerr << "Error: Unsupported platform in Gnuplot::file_exists.\n";
    return false;
#endif

    // File does not exist or is inaccessible
    return false;
}

auto Gnuplot::file_ready(const std::string &filename) -> bool
{
    // Check if the file exists.
    if (Gnuplot::file_exists(filename, 0)) {
        // Check if the file has read permissions.
        if (Gnuplot::file_exists(filename, 4)) {
            return true;
        }
        std::cerr << "No read permission for file \"" << filename << "\".";
        return false;
    }
    // File does not exist.
    std::cerr << "File \"" << filename << "\" does not exist.";
    return true;
}

auto Gnuplot::create_tmpfile(std::ofstream &tmp) -> std::string
{
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__TOS_WIN__)
    std::string filename = "gnuplotiXXXXXX.tmp";

    if (Gnuplot::m_tmpfile_num >= Gnuplot::m_tmpfile_max) {
        std::cerr << "Error: Maximum number of temporary files reached (" << Gnuplot::m_tmpfile_max
                  << "). Cannot create more files.\n";
        return std::string();
    }

    // _mktemp_s modifies the string in-place, so we need a mutable buffer
    if (_mktemp_s(&filename[0], filename.size() + 1) != 0) {
        std::cerr << "Error: Cannot create temporary file \"" << filename << "\".\n";
        return std::string();
    }

    // Open the temporary file for writing
    tmp.open(filename, std::ios::out | std::ios::trunc);
    if (!tmp.is_open() || tmp.bad()) {
        std::cerr << "Error: Cannot open temporary file \"" << filename << "\" for writing.\n";
        return std::string();
    }

#elif defined(unix) || defined(__unix) || defined(__unix__) || defined(__APPLE__)
    std::string filename = "/tmp/gnuplotiXXXXXX";

    if (Gnuplot::m_tmpfile_num >= Gnuplot::m_tmpfile_max) {
        std::cerr << "Error: Maximum number of temporary files reached (" << Gnuplot::m_tmpfile_max
                  << "). Cannot create more files.\n";
        return std::string();
    }

    // mkstemp requires a mutable C-string
    int fd = mkstemp(&filename[0]);
    if (fd == -1) {
        std::cerr << "Error: Cannot create temporary file \"" << filename << "\".\n";
        return std::string();
    }

    // Associate the file descriptor with ofstream
    tmp.open(filename);
    if (!tmp.is_open() || tmp.bad()) {
        std::cerr << "Error: Cannot open temporary file \"" << filename << "\" for writing.\n";
        close(fd); // Close file descriptor to prevent leaks
        return std::string();
    }

    close(fd); // Close the file descriptor after associating with ofstream
#else
    std::cerr << "Error: Unsupported platform for temporary file creation.\n";
    return std::string();
#endif

    // Store the temporary file name for cleanup and increment the counter
    tmpfile_list.emplace_back(filename);
    Gnuplot::m_tmpfile_num++;

    // Return the name of the successfully created temporary file
    return filename;
}

auto Gnuplot::apply_contour_settings() -> Gnuplot &
{
    // Set contour type.
    switch (contour.type) {
    case contour_type_t::base:
        this->send_cmd("set contour base");
        break;
    case contour_type_t::surface:
        this->send_cmd("set contour surface");
        break;
    case contour_type_t::both:
        this->send_cmd("set contour both");
        break;
    case contour_type_t::none:
        this->send_cmd("unset contour");
        break;
    }
    // Return early if no contour settings are specified.
    if (contour.type == contour_type_t::none) {
        return *this;
    }
    // Set contour parameters.
    switch (contour.param) {
    case contour_param_t::levels:
        this->send_cmd("set cntrparam levels " + std::to_string(contour.levels));
        break;
    case contour_param_t::increment: {
        std::ostringstream oss;
        oss << "set cntrparam increment " << contour.increment_start << "," << contour.increment_step << ","
            << contour.increment_end;
        this->send_cmd(oss.str());
        break;
    }
    case contour_param_t::discrete: {
        std::ostringstream oss;
        oss << "set cntrparam level discrete";
        for (std::size_t i = 0; i < contour.discrete_levels.size(); ++i) {
            oss << " " << contour.discrete_levels[i];
            if (i < contour.discrete_levels.size() - 1) {
                oss << ",";
            }
        }
        this->send_cmd(oss.str());
        break;
    }
    }
    return *this;
}

void Gnuplot::remove_tmpfiles()
{
    if (tmpfile_list.empty()) {
        return; // No temporary files to remove
    }
    for (const auto &tmpfile : tmpfile_list) {
        if (std::remove(tmpfile.c_str()) != 0) {
            std::cerr << "Warning: Unable to remove temporary file \"" << tmpfile << "\".\n";
        }
    }
    // Adjust the global temporary file counter.
    if (Gnuplot::m_tmpfile_num < tmpfile_list.size()) {
        std::cerr << "We are trying to remove more tmp files than expected (curret: " << Gnuplot::m_tmpfile_num
                  << ", to close: " << tmpfile_list.size() << ").\n";
        Gnuplot::m_tmpfile_num = 0;
    } else {
        Gnuplot::m_tmpfile_num -= tmpfile_list.size();
    }
    // Clear the list of temporary files
    tmpfile_list.clear();
}

} // namespace gpcpp
