/// @file gnuplot.i.hpp
/// @brief

#pragma once

namespace gpcpp
{

/// @brief Maximum number of temporary files allowed.
/// @details This value is platform-dependent:
/// - Windows: 27 files (due to OS restrictions).
/// - UNIX-like systems: 64 files.
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__TOS_WIN__)
#define GP_MAX_TMP_FILES 27 ///< Maximum temporary files for Windows.
#else
#define GP_MAX_TMP_FILES 64 ///< Maximum temporary files for UNIX-like systems.
#endif

// Initialize the static variables
int Gnuplot::m_tmpfile_num = 0;

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__TOS_WIN__)
// Windows-specific static variable initializations
std::string Gnuplot::m_gnuplot_filename = "pgnuplot.exe";
std::string Gnuplot::m_gnuplot_path     = "C:/program files/gnuplot/bin/";
#elif defined(unix) || defined(__unix) || defined(__unix__) || defined(__APPLE__)
// UNIX-like system static variable initializations
std::string Gnuplot::m_gnuplot_filename = "gnuplot";
std::string Gnuplot::m_gnuplot_path     = "/usr/local/bin/";
#endif

/// Macro to create a temporary file using platform-specific functions
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__TOS_WIN__)
#define CREATE_TEMP_FILE(name) (_mktemp(name) == NULL)
#elif defined(unix) || defined(__unix) || defined(__unix__) || defined(__APPLE__)
#define CREATE_TEMP_FILE(name) (mkstemp(name) == -1)
#endif

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__TOS_WIN__)
#define CLOSE_PIPE(pipe) _pclose(pipe)
#elif defined(unix) || defined(__unix) || defined(__unix__) || defined(__APPLE__)
#define CLOSE_PIPE(pipe) pclose(pipe)
#endif

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__TOS_WIN__)
#define OPEN_PIPE(cmd, mode) _popen(cmd, mode)
#elif defined(unix) || defined(__unix) || defined(__unix__) || defined(__APPLE__)
#define OPEN_PIPE(cmd, mode) popen(cmd, mode)
#endif

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__TOS_WIN__)
#define FILE_ACCESS(file, mode) _access(file, mode)
#elif defined(unix) || defined(__unix) || defined(__unix__) || defined(__APPLE__)
#define FILE_ACCESS(file, mode) access(file, mode)
#endif

Gnuplot::Gnuplot()
    : gnuplot_pipe(nullptr),               // No active pipe initially
      terminal_type(terminal_type_t::wxt), // Default terminal type is wxt
      valid(false),                        // Invalid session by default
      two_dim(true),                       // 2D plotting by default
      nplots(0),                           // No plots initially
      line_width(1.0),                     // Default line width
      plot_style(plot_style_t::lines),     // Default plot style is lines
      smooth_style(smooth_style_t::none),  // No smoothing by default
      line_style(""),                      // No custom line style
      line_color(),                        // Default line color is unspecified
      point_style(point_style_t::none),    // Default point style is none
      point_size(-1.0)                     // Default point size is unspecified

{
#if (defined(unix) || defined(__unix) || defined(__unix__)) && !defined(__APPLE__)
    // Ensure DISPLAY is set for Unix systems.
    if (!getenv("DISPLAY")) {
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
    gnuplot_pipe = OPEN_PIPE((Gnuplot::m_gnuplot_path + "/" + Gnuplot::m_gnuplot_filename).c_str(), "w");
    if (!gnuplot_pipe) {
        std::cerr << "Error: Unable to open pipe to Gnuplot.\n";
        valid = false;
        return;
    }

    // Initialize plotting state.
    valid   = true;
    nplots  = 0;
    two_dim = false;

    // Initialize styles.
    plot_style   = plot_style_t::none;
    smooth_style = smooth_style_t::none;
    line_style.clear();
    line_color.unset();
    point_style = point_style_t::plus;
    point_size  = -1.0;
    line_width  = -1.0;

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
    if (gnuplot_pipe) {
        if (CLOSE_PIPE(gnuplot_pipe) == -1) {
            std::cerr << "Warning: Problem closing communication to Gnuplot.\n";
        }
        // Avoid dangling pointer.
        gnuplot_pipe = nullptr;
    }

    // Remove all temporary files created during the session
    remove_tmpfiles();
}

Gnuplot &Gnuplot::send_cmd(const std::string &cmdstr)
{
    // Check if the Gnuplot session is ready.
    if (!this->is_ready()) {
        std::cerr << "Error: Invalid Gnuplot session not ready.\n";
        return *this;
    }

    // Write the command to the Gnuplot pipe.
    fprintf(gnuplot_pipe, "%s\n", cmdstr.c_str());
    fflush(gnuplot_pipe);

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

template <typename X>
Gnuplot &Gnuplot::plot_x(const X &x, const std::string &title)
{
    // Check if the Gnuplot session is ready.
    if (!this->is_ready()) {
        std::cerr << "Error: Invalid Gnuplot session not ready.\n";
        return *this;
    }

    // Check if the input vector is empty.
    if (x.empty()) {
        std::cerr << "Error: Input vector is empty. Cannot plot data." << std::endl;
        return *this;
    }

    // Create a temporary file for storing the data
    std::ofstream file;
    std::string filename = this->create_tmpfile(file);
    if (filename.empty()) {
        std::cerr << "Error: Temporary file filename is empty. File creation failed." << std::endl;
        return *this;
    }

    // Write the data to the temporary file
    for (size_t i = 0; i < x.size(); ++i) {
        if (!(file << x[i] << '\n')) {
            file.close();
            std::cerr << "Error: Failed to write data to the temporary file: " << filename << std::endl;
            return *this;
        }
    }

    // Ensure the file buffer is flushed
    file.flush();
    if (file.fail()) {
        file.close();
        std::cerr << "Error: Failed to flush data to the temporary file: " << filename << std::endl;
        return *this;
    }
    file.close();

    // Check if the file is available for reading.
    if (!this->file_ready(filename)) {
        std::cerr << "Error: File " << filename << " is not available for reading." << std::endl;
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
    if (smooth_style == smooth_style_t::none) {
        oss << " with " << this->plot_style_to_string(plot_style);
    } else {
        oss << " smooth " << this->smooth_style_to_string(smooth_style);
    }
    // Include line color if it is specified.
    if (line_color.is_set()) {
        oss << " lc rgbcolor \"" << line_color.to_string() << "\"";
    }
    // Add line style options only if the plot style supports lines.
    if (is_line_style(plot_style)) {
        // Add line width if specified.
        if (line_width > 0) {
            oss << " lw " << line_width;
        }
        // Add line style if specified.
        if (!line_style.empty()) {
            oss << " " << line_style;
        }
    }
    // Add point style and size only if the plot style supports points.
    if (is_point_style(plot_style)) {
        // Add point style if specified.
        oss << " pt " << this->point_style_to_string(point_style);
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
Gnuplot &Gnuplot::plot_x(const std::vector<X> &datasets, const std::vector<std::string> &titles)
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
    for (size_t i = 0; i < filenames.size(); ++i) {
        if (!this->file_ready(filenames[i])) {
            std::cerr << "Error: File " << filenames[i] << " is not available for reading.\n";
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
        if (smooth_style == smooth_style_t::none) {
            oss << " with " << this->plot_style_to_string(plot_style);
        } else {
            oss << " smooth " << this->smooth_style_to_string(smooth_style);
        }

        // Add line options if applicable
        if (is_line_style(plot_style)) {
            if (line_width > 0) {
                oss << " lw " << line_width;
            }
            if (line_color.is_set()) {
                oss << " lc rgbcolor \"" << line_color.to_string() << "\"";
            }
        }

        // Add point options if applicable
        if (is_point_style(plot_style)) {
            oss << " pt " << this->point_style_to_string(point_style);
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
Gnuplot &Gnuplot::plot_xy(const X &x, const Y &y, const std::string &title)
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
    if (!this->file_ready(filename)) {
        std::cerr << "Error: File " << filename << " is not available for reading.\n";
        return *this;
    }
    std::ostringstream oss;
    // Determine whether to use 'plot' or 'replot' based on the current plot state
    oss << ((nplots > 0 && two_dim) ? "replot" : "plot");
    // Specify the file and columns for the Gnuplot command
    oss << " \"" << filename << "\" using 1:2";
    // Add a title or specify 'notitle' if no title is provided
    oss << (title.empty() ? " notitle " : " title \"" + title + "\"");
    // Specify the plot style or smoothing option.
    if (smooth_style == smooth_style_t::none) {
        oss << " with " << this->plot_style_to_string(plot_style);
    } else {
        oss << " smooth " << this->smooth_style_to_string(smooth_style);
    }
    // Include line color if it is specified.
    if (line_color.is_set()) {
        oss << " lc rgbcolor \"" << line_color.to_string() << "\"";
    }
    // Add line style options only if the plot style supports lines.
    if (is_line_style(plot_style)) {
        // Add line width if specified.
        if (line_width > 0) {
            oss << " lw " << line_width;
        }
        // Add line style if specified.
        if (!line_style.empty()) {
            oss << " " << line_style;
        }
    }
    // Add point style and size only if the plot style supports points.
    if (is_point_style(plot_style)) {
        // Add point style if specified.
        oss << " pt " << this->point_style_to_string(point_style);
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
Gnuplot &
Gnuplot::plot_xy_erorrbar(const X &x, const Y &y, const E &dy, erorrbar_style_t style, const std::string &title)
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
    if (!this->file_ready(filename)) {
        std::cerr << "Error: File " << filename << " is not available for reading.\n";
        return *this;
    }

    std::ostringstream oss;

    // Determine whether to use 'plot' or 'replot' based on the current plot state
    oss << ((nplots > 0 && two_dim) ? "replot " : "plot ");

    // Specify the file and columns for the Gnuplot command
    oss << "\"" << filename << "\" using 1:2:3 with " << this->errorbars_to_string(style) << " ";

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
    if (!line_style.empty()) {
        oss << " " << line_style;
    }

    // Add point style if specified.
    oss << " pt " << this->point_style_to_string(point_style);
    // Add point size if specified.
    if (point_size > 0) {
        oss << " ps " << point_size;
    }

    // Send the constructed command to Gnuplot for execution
    this->send_cmd(oss.str());

    return *this;
}

template <typename X, typename Y, typename Z>
Gnuplot &Gnuplot::plot_xyz(const X &x, const Y &y, const Z &z, const std::string &title)
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
    if (!this->file_ready(filename)) {
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
    if (smooth_style == smooth_style_t::none) {
        oss << " with " << this->plot_style_to_string(plot_style);
    } else {
        oss << " smooth " << this->smooth_style_to_string(smooth_style);
    }

    // Include line color if it is specified.
    if (line_color.is_set()) {
        oss << " lc rgbcolor \"" << line_color.to_string() << "\"";
    }

    // Add line style options only if the plot style supports lines.
    if (is_line_style(plot_style)) {
        // Add line width if specified.
        if (line_width > 0) {
            oss << " lw " << line_width;
        }
        // Add line style if specified.
        if (!line_style.empty()) {
            oss << " " << line_style;
        }
    }

    // Add point style and size only if the plot style supports points.
    if (is_point_style(plot_style)) {
        // Add point style if specified.
        oss << " pt " << this->point_style_to_string(point_style);
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
Gnuplot &Gnuplot::plot_3d_grid(const X &x, const Y &y, const Z &z, const std::string &title)
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
    if (!this->file_ready(filename)) {
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
    if (smooth_style == smooth_style_t::none) {
        oss << " with " << this->plot_style_to_string(plot_style);
    } else {
        oss << " smooth " << this->smooth_style_to_string(smooth_style);
    }

    // Include line color if it is specified.
    if (line_color.is_set()) {
        oss << " lc rgbcolor \"" << line_color.to_string() << "\"";
    }

    // Add line style options only if the plot style supports lines.
    if (is_line_style(plot_style)) {
        // Add line width if specified.
        if (line_width > 0) {
            oss << " lw " << line_width;
        }
        // Add line style if specified.
        if (!line_style.empty()) {
            oss << " " << line_style;
        }
    }

    // Add point style and size only if the plot style supports points.
    if (is_point_style(plot_style)) {
        // Add point style if specified.
        oss << " pt " << this->point_style_to_string(point_style);
        // Add point size if specified.
        if (point_size > 0) {
            oss << " ps " << point_size;
        }
    }

    // Send the constructed command to Gnuplot for execution
    this->send_cmd(oss.str());
    return *this;
}

Gnuplot &Gnuplot::plot_slope(const double a, const double b, const std::string &title)
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
    if (smooth_style == smooth_style_t::none) {
        oss << " with " << this->plot_style_to_string(plot_style);
    } else {
        oss << " smooth " << this->smooth_style_to_string(smooth_style);
    }

    // Include line color if it is specified.
    if (line_color.is_set()) {
        oss << " lc rgbcolor \"" << line_color.to_string() << "\"";
    }

    // Add line style options only if the plot style supports lines.
    if (is_line_style(plot_style)) {
        // Add line width if specified.
        if (line_width > 0) {
            oss << " lw " << line_width;
        }
        // Add line style if specified.
        if (!line_style.empty()) {
            oss << " " << line_style;
        }
    }

    // Add point style and size only if the plot style supports points.
    if (is_point_style(plot_style)) {
        // Add point style if specified.
        oss << " pt " << this->point_style_to_string(point_style);
        // Add point size if specified.
        if (point_size > 0) {
            oss << " ps " << point_size;
        }
    }

    // Send the constructed command to Gnuplot for execution
    this->send_cmd(oss.str());

    return *this;
}

Gnuplot &Gnuplot::plot_equation(const std::string &equation, const std::string &title)
{
    std::ostringstream oss;

    // Determine whether to use 'plot' or 'replot' based on the current state.
    oss << ((nplots > 0 && two_dim) ? "replot " : "plot ");

    // Add the equation to be plotted.
    oss << equation;

    // Set the title or use 'notitle' if no title is provided.
    oss << (title.empty() ? " notitle" : " title \"" + title + "\"");

    // Specify the plot style or smoothing option.
    if (smooth_style == smooth_style_t::none) {
        oss << " with " << this->plot_style_to_string(plot_style);
    } else {
        oss << " smooth " << this->smooth_style_to_string(smooth_style);
    }

    // Include line color if it is specified.
    if (line_color.is_set()) {
        oss << " lc rgbcolor \"" << line_color.to_string() << "\"";
    }

    // Add line style options only if the plot style supports lines.
    if (is_line_style(plot_style)) {
        // Add line width if specified.
        if (line_width > 0) {
            oss << " lw " << line_width;
        }
        // Add line style if specified.
        if (!line_style.empty()) {
            oss << " " << line_style;
        }
    }

    // Add point style and size only if the plot style supports points.
    if (is_point_style(plot_style)) {
        // Add point style if specified.
        oss << " pt " << this->point_style_to_string(point_style);
        // Add point size if specified.
        if (point_size > 0) {
            oss << " ps " << point_size;
        }
    }

    // Send the constructed command to Gnuplot for execution.
    this->send_cmd(oss.str());
    return *this;
}

Gnuplot &Gnuplot::plot_equation3d(const std::string &equation, const std::string &title)
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
    if (smooth_style == smooth_style_t::none) {
        oss << " with " << this->plot_style_to_string(plot_style);
    } else {
        oss << " smooth " << this->smooth_style_to_string(smooth_style);
    }

    // Include line color if it is specified.
    if (line_color.is_set()) {
        oss << " lc rgbcolor \"" << line_color.to_string() << "\"";
    }

    // Add line style options only if the plot style supports lines.
    if (is_line_style(plot_style)) {
        // Add line width if specified.
        if (line_width > 0) {
            oss << " lw " << line_width;
        }
        // Add line style if specified.
        if (!line_style.empty()) {
            oss << " " << line_style;
        }
    }

    // Add point style and size only if the plot style supports points.
    if (is_point_style(plot_style)) {
        // Add point style if specified.
        oss << " pt " << this->point_style_to_string(point_style);
        // Add point size if specified.
        if (point_size > 0) {
            oss << " ps " << point_size;
        }
    }

    // Send the constructed command to Gnuplot for execution.
    this->send_cmd(oss.str());
    return *this;
}

Gnuplot &Gnuplot::plot_image(const unsigned char *ucPicBuf,
                             const unsigned int iWidth,
                             const unsigned int iHeight,
                             const std::string &title)
{
    // Create a temporary file to store image data
    std::ofstream file;
    std::string filename = create_tmpfile(file);
    if (filename.empty()) {
        std::cerr << "Error: Failed to create a temporary file for image plotting." << std::endl;
        return *this; // Early return on failure
    }

    // Write the image data (width, height, pixel value) to the temporary file
    int iIndex         = 0;
    bool write_success = true;
    for (unsigned int iRow = 0; iRow < iHeight && write_success; ++iRow) {
        for (unsigned int iColumn = 0; iColumn < iWidth; ++iColumn) {
            if (!(file << iColumn << " " << iRow << " " << static_cast<float>(ucPicBuf[iIndex++]) << std::endl)) {
                std::cerr << "Error: Failed to write image data to temporary file: " << filename << std::endl;
                write_success = false;
                break;
            }
        }
    }

    // Ensure all data is written to the file and the file is closed properly
    file.flush();
    if (file.fail() || !write_success) {
        file.close();
        std::cerr << "Error: Failed to flush data to temporary file: " << filename << std::endl;
        return *this; // Early return on failure
    }
    file.close();

    // Check if the file is available for reading
    if (this->file_ready(filename)) {
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

bool Gnuplot::set_gnuplot_path(const std::string &path)
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
    } else {
        Gnuplot::m_gnuplot_path.clear();
        return false;
    }
}

Gnuplot &Gnuplot::set_terminal(terminal_type_t type)
{
    // For Unix-like systems, ensure the DISPLAY variable is set when using X11.
#if defined(unix) || defined(__unix) || defined(__unix__) || defined(__APPLE__)
    if ((type == terminal_type_t::x11) && (getenv("DISPLAY") == NULL)) {
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

Gnuplot &Gnuplot::replot()
{
    if (nplots > 0) {
        this->send_cmd("replot");
    }
    return *this;
}

bool Gnuplot::is_ready() const
{
    return valid && gnuplot_pipe;
}

Gnuplot &Gnuplot::reset_plot()
{
    nplots = 0;
    return *this;
}

Gnuplot &Gnuplot::reset_all()
{
    nplots = 0;
    this->send_cmd("reset");
    this->send_cmd("clear");
    plot_style   = plot_style_t::none;
    smooth_style = smooth_style_t::none;
    return *this;
}

Gnuplot &Gnuplot::set_plot_style(plot_style_t style)
{
    plot_style = style;
    return *this;
}

Gnuplot &Gnuplot::set_smooth_style(smooth_style_t style)
{
    smooth_style = style;
    return *this;
}

Gnuplot &Gnuplot::set_line_style(line_style_t style, const std::string &custom_pattern)
{
    line_style = this->line_style_to_string(style, custom_pattern);
    return *this;
}

Gnuplot &Gnuplot::set_line_color(const std::string &color)
{
    line_color = gpcpp::Color(color);
    return *this;
}

Gnuplot &Gnuplot::set_line_color(int r, int g, int b)
{
    line_color = gpcpp::Color(r, g, b);
    return *this;
}

Gnuplot &Gnuplot::set_point_style(point_style_t style)
{
    point_style = style;
    return *this;
}

Gnuplot &Gnuplot::set_point_size(double size)
{
    if (size > 0) {
        point_size = size;
    }
    return *this;
}

Gnuplot &Gnuplot::show()
{
    this->send_cmd("set output");
    this->send_cmd("set terminal " + this->terminal_type_to_string(terminal_type));

    // Wait for user input before closing.
    std::cout << "Press Enter to continue..." << std::endl;
    std::cin.get();

    return *this;
}

Gnuplot &Gnuplot::set_output(const std::string filename)
{
    // Set the output file where the plot will be saved
    this->send_cmd("set output \"" + filename + "\"");
    this->send_cmd("set terminal " + this->terminal_type_to_string(terminal_type));
    return *this;
}

Gnuplot &Gnuplot::set_legend(const std::string &position,
                             const std::string &font,
                             const std::string &title,
                             bool with_box,
                             double spacing,
                             double width)
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

Gnuplot &Gnuplot::set_title(const std::string &title)
{
    // Send the command to Gnuplot
    this->send_cmd("set title \"" + title + "\"");
    return *this;
}

Gnuplot &Gnuplot::unset_title()
{
    this->set_title();
    return *this;
}

Gnuplot &Gnuplot::set_xlogscale(const double base)
{
    this->send_cmd("set logscale x " + std::to_string(base));
    return *this;
}

Gnuplot &Gnuplot::set_ylogscale(const double base)
{
    std::ostringstream cmdstr;

    cmdstr << "set logscale y " << base;
    this->send_cmd(cmdstr.str());

    return *this;
}

Gnuplot &Gnuplot::set_zlogscale(const double base)
{
    std::ostringstream cmdstr;

    cmdstr << "set logscale z " << base;
    this->send_cmd(cmdstr.str());

    return *this;
}

Gnuplot &Gnuplot::unset_xlogscale()
{
    this->send_cmd("unset logscale x");
    return *this;
}

Gnuplot &Gnuplot::unset_ylogscale()
{
    this->send_cmd("unset logscale y");
    return *this;
}

Gnuplot &Gnuplot::unset_zlogscale()
{
    this->send_cmd("unset logscale z");
    return *this;
}

Gnuplot &Gnuplot::set_line_width(double width)
{
    if (width > 0) {
        line_width = width;
    }
    return *this;
}

/// turns grid on/off
Gnuplot &Gnuplot::set_grid()
{
    this->send_cmd("set grid");
    return *this;
}

/// @brief Sets the major tics for the x-axis.
Gnuplot &Gnuplot::set_xtics_major(double major_step)
{
    if (major_step <= 0) {
        throw std::invalid_argument("Major step size for x-axis must be positive.");
    }

    std::string cmd = "set xtics " + std::to_string(major_step);
    this->send_cmd(cmd);
    return *this;
}

/// @brief Sets the minor tics for the x-axis.
Gnuplot &Gnuplot::set_xtics_minor(int minor_intervals)
{
    if (minor_intervals <= 0) {
        throw std::invalid_argument("Number of minor intervals for x-axis must be positive.");
    }

    std::string cmd = "set mxtics " + std::to_string(minor_intervals);
    this->send_cmd(cmd);
    return *this;
}

/// @brief Sets the major tics for the y-axis.
Gnuplot &Gnuplot::set_ytics_major(double major_step)
{
    if (major_step <= 0) {
        throw std::invalid_argument("Major step size for y-axis must be positive.");
    }

    std::string cmd = "set ytics " + std::to_string(major_step);
    this->send_cmd(cmd);
    return *this;
}

/// @brief Sets the minor tics for the y-axis.
Gnuplot &Gnuplot::set_ytics_minor(int minor_intervals)
{
    if (minor_intervals <= 0) {
        throw std::invalid_argument("Number of minor intervals for y-axis must be positive.");
    }

    std::string cmd = "set mytics " + std::to_string(minor_intervals);
    this->send_cmd(cmd);
    return *this;
}

Gnuplot &Gnuplot::set_grid_line_style(grid_type_t grid_type,
                                      line_style_t style,
                                      const Color &color,
                                      double width,
                                      const std::string &custom_dash)
{
    // Define the style line index based on grid type
    int line_index = (grid_type == grid_type_t::major) ? 1 : (grid_type == grid_type_t::minor) ? 2 : 3;

    // Define default linetype (solid)
    int linetype = 1;

    // Define custom dashtype based on the line style
    std::string dashtype;
    switch (style) {
    case line_style_t::solid:
        linetype = 1;
        break;
    case line_style_t::dashed:
        dashtype = "50, 25";
        break;
    case line_style_t::dotted:
        dashtype = "1, 1";
        break;
    case line_style_t::dash_dot:
        dashtype = "10, 5, 1, 5";
        break;
    case line_style_t::dash_dot_dot:
        dashtype = "10, 5, 1, 5, 1, 5";
        break;
    case line_style_t::custom:
        if (!custom_dash.empty()) {
            dashtype = custom_dash; // Use the provided custom dash pattern
        }
        break;
    default:
        break;
    }

    // Build the Gnuplot command
    std::string cmd = "set style line " + std::to_string(line_index); // Line style index
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

Gnuplot &Gnuplot::apply_grid(const std::string &tics, int angle, const std::string &layer)
{
    std::string cmd = "set grid " + tics;

    if (layer == "front" || layer == "back") {
        cmd += " " + grid.grid_layer;
    }

    // Apply major grid style
    cmd += " ls 1"; // Use line style 1 for major grid

    // Apply minor grid style
    cmd += ", ls 2"; // Use line style 2 for minor grid

    // Add polar grid if applicable
    if (angle >= 0) {
        cmd += " polar " + std::to_string(angle);
    }

    this->send_cmd(cmd);
    return *this;
}

Gnuplot &Gnuplot::unset_grid()
{
    this->send_cmd("unset grid");
    return *this;
}

Gnuplot &Gnuplot::set_multiplot()
{
    this->send_cmd("set multiplot");
    return *this;
}

Gnuplot &Gnuplot::unset_multiplot()
{
    this->send_cmd("unset multiplot");
    return *this;
}

Gnuplot &Gnuplot::set_origin_and_size(double x_origin, double y_origin, double width, double height)
{
    // Set the origin (position) of the plot in the window
    this->send_cmd("set origin " + std::to_string(x_origin) + "," + std::to_string(y_origin));

    // Set the size of the plot area (width and height relative to the window)
    this->send_cmd("set size " + std::to_string(width) + "," + std::to_string(height));

    // Return the current object for method chaining
    return *this;
}

Gnuplot &Gnuplot::set_samples(const int samples)
{
    std::ostringstream cmdstr;
    cmdstr << "set samples " << samples;
    this->send_cmd(cmdstr.str());

    return *this;
}

Gnuplot &Gnuplot::set_isosamples(const int isolines)
{
    std::ostringstream cmdstr;
    cmdstr << "set isosamples " << isolines;
    this->send_cmd(cmdstr.str());

    return *this;
}

Gnuplot &Gnuplot::set_contour_type(contour_type_t type)
{
    contour.type = type;
    return *this;
}

Gnuplot &Gnuplot::set_contour_param(contour_param_t param)
{
    contour.param = param;
    return *this;
}

Gnuplot &Gnuplot::set_contour_levels(int levels)
{
    if (levels > 0) {
        contour.levels = levels;
    }
    return *this;
}

Gnuplot &Gnuplot::set_contour_increment(double start, double step, double end)
{
    contour.increment_start = start;
    contour.increment_step  = step;
    contour.increment_end   = end;
    return *this;
}

Gnuplot &Gnuplot::set_contour_discrete_levels(const std::vector<double> &levels)
{
    contour.discrete_levels = levels;
    return *this;
}

Gnuplot &Gnuplot::set_hidden3d()
{
    this->send_cmd("set hidden3d");
    return *this;
}

Gnuplot &Gnuplot::unset_hidden3d()
{
    this->send_cmd("unset hidden3d");
    return *this;
}

Gnuplot &Gnuplot::unset_contour()
{
    this->send_cmd("unset contour");
    return *this;
}

Gnuplot &Gnuplot::set_surface()
{
    this->send_cmd("set surface");
    return *this;
}

Gnuplot &Gnuplot::unset_surface()
{
    this->send_cmd("unset surface");
    return *this;
}

Gnuplot &Gnuplot::set_xautoscale()
{
    this->send_cmd("set xrange restore");
    this->send_cmd("set autoscale x");
    return *this;
}

Gnuplot &Gnuplot::set_yautoscale()
{
    this->send_cmd("set yrange restore");
    this->send_cmd("set autoscale y");
    return *this;
}

Gnuplot &Gnuplot::set_zautoscale()
{
    this->send_cmd("set zrange restore");
    this->send_cmd("set autoscale z");
    return *this;
}

Gnuplot &Gnuplot::set_xlabel(const std::string &label)
{
    std::ostringstream cmdstr;

    cmdstr << "set xlabel \"" << label << "\"";
    this->send_cmd(cmdstr.str());

    return *this;
}

Gnuplot &Gnuplot::set_ylabel(const std::string &label)
{
    std::ostringstream cmdstr;

    cmdstr << "set ylabel \"" << label << "\"";
    this->send_cmd(cmdstr.str());

    return *this;
}

Gnuplot &Gnuplot::set_zlabel(const std::string &label)
{
    std::ostringstream cmdstr;

    cmdstr << "set zlabel \"" << label << "\"";
    this->send_cmd(cmdstr.str());

    return *this;
}

Gnuplot &Gnuplot::set_xrange(const double iFrom, const double iTo)
{
    std::ostringstream cmdstr;

    cmdstr << "set xrange[" << iFrom << ":" << iTo << "]";
    this->send_cmd(cmdstr.str());

    return *this;
}

Gnuplot &Gnuplot::set_yrange(const double iFrom, const double iTo)
{
    std::ostringstream cmdstr;

    cmdstr << "set yrange[" << iFrom << ":" << iTo << "]";
    this->send_cmd(cmdstr.str());

    return *this;
}

Gnuplot &Gnuplot::set_zrange(const double iFrom, const double iTo)
{
    std::ostringstream cmdstr;

    cmdstr << "set zrange[" << iFrom << ":" << iTo << "]";
    this->send_cmd(cmdstr.str());

    return *this;
}

Gnuplot &Gnuplot::set_cbrange(const double iFrom, const double iTo)
{
    std::ostringstream cmdstr;

    cmdstr << "set cbrange[" << iFrom << ":" << iTo << "]";
    this->send_cmd(cmdstr.str());

    return *this;
}

bool Gnuplot::get_program_path()
{
    // Check the first location: `m_gnuplot_path`
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
            Gnuplot::m_gnuplot_path = dir;  // Set `m_gnuplot_path`
            return true;
        }
    }

    // Gnuplot was not found
    std::cerr << "Error: Gnuplot not found in PATH or in \"" << Gnuplot::m_gnuplot_path << "\".\n";
    return false;
}

bool Gnuplot::file_exists(const std::string &filename, int mode)
{
    // Validate mode argument
    if (mode < 0 || mode > 7) {
        std::cerr << "Error: Invalid mode in Gnuplot::file_exists. Mode must be between 0 and 7.\n";
        return false;
    }

    // Check file access using platform-independent macro
    // mode = 0: Existence, 1: Execute, 2: Write, 4: Read
    if (FILE_ACCESS(filename.c_str(), mode) == 0) {
        return true; // File exists with the required mode
    }

    // File does not exist or is inaccessible
    return false;
}

bool Gnuplot::is_line_style(plot_style_t style)
{
    return (style == plot_style_t::lines || style == plot_style_t::lines_points || style == plot_style_t::steps ||
            style == plot_style_t::fsteps || style == plot_style_t::histeps || style == plot_style_t::filled_curves ||
            style == plot_style_t::impulses);
}

bool Gnuplot::is_point_style(plot_style_t style)
{
    return (style == plot_style_t::points || style == plot_style_t::lines_points);
}

bool Gnuplot::file_ready(const std::string &filename)
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
std::string Gnuplot::create_tmpfile(std::ofstream &tmp)
{
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__TOS_WIN__)
    char filename[] = "gnuplotiXXXXXX"; // Temporary file in working directory
#elif defined(unix) || defined(__unix) || defined(__unix__) || defined(__APPLE__)
    char filename[] = "/tmp/gnuplotiXXXXXX"; // Temporary file in /tmp
#endif

    // Check if the maximum number of temporary files has been reached.
    if (Gnuplot::m_tmpfile_num >= GP_MAX_TMP_FILES) {
        std::cerr << "Error: Maximum number of temporary files reached (" << GP_MAX_TMP_FILES
                  << "). Cannot create more files.\n";
        return std::string(); // Return an empty string to indicate failure
    }

    // Generate a unique temporary filename.
    if (CREATE_TEMP_FILE(filename)) {
        std::cerr << "Error: Cannot create temporary file \"" << filename << "\".\n";
        return std::string(); // Return an empty string to indicate failure
    }

    // Open the temporary file for writing.
    tmp.open(filename);
    if (!tmp.is_open() || tmp.bad()) {
        std::cerr << "Error: Cannot open temporary file \"" << filename << "\" for writing.\n";
        return std::string(); // Return an empty string to indicate failure
    }

    // Store the temporary file name for cleanup and increment the counter.
    tmpfile_list.push_back(filename);
    Gnuplot::m_tmpfile_num++;

    return filename; // Return the name of the successfully created temporary file
}

Gnuplot &Gnuplot::apply_contour_settings()
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

std::string Gnuplot::plot_style_to_string(plot_style_t style)
{
    switch (style) {
    case plot_style_t::lines:
        return "lines";
    case plot_style_t::points:
        return "points";
    case plot_style_t::lines_points:
        return "linespoints";
    case plot_style_t::impulses:
        return "impulses";
    case plot_style_t::dots:
        return "dots";
    case plot_style_t::steps:
        return "steps";
    case plot_style_t::fsteps:
        return "fsteps";
    case plot_style_t::histeps:
        return "histeps";
    case plot_style_t::boxes:
        return "boxes";
    case plot_style_t::filled_curves:
        return "filledcurves";
    case plot_style_t::histograms:
        return "histograms";
    default:
        return "lines";
    }
}

std::string Gnuplot::smooth_style_to_string(smooth_style_t style)
{
    switch (style) {
    case smooth_style_t::unique:
        return "unique";
    case smooth_style_t::frequency:
        return "frequency";
    case smooth_style_t::csplines:
        return "csplines";
    case smooth_style_t::acsplines:
        return "acsplines";
    case smooth_style_t::bezier:
        return "bezier";
    case smooth_style_t::sbezier:
        return "sbezier";
    default:
        return std::string(); // Default: No smoothing
    }
}

std::string Gnuplot::line_style_to_string(line_style_t style, const std::string &custom_pattern)
{
    switch (style) {
    case line_style_t::solid:
        return "dashtype 1"; // Solid
    case line_style_t::dashed:
        return "dashtype 2"; // Dashed
    case line_style_t::dotted:
        return "dashtype 3"; // Dotted
    case line_style_t::dash_dot:
        return "dashtype 4"; // Dash-dot
    case line_style_t::dash_dot_dot:
        return "dashtype 5"; // Dash-dot-dot
    case line_style_t::custom:
        return "dashtype (" + custom_pattern + ")";
    default:
        return "dashtype 1"; // Fallback to solid
    }
}

std::string Gnuplot::point_style_to_string(point_style_t style)
{
    return std::to_string(static_cast<int>(style));
}

std::string Gnuplot::errorbars_to_string(erorrbar_style_t style)
{
    switch (style) {
    case erorrbar_style_t::yerrorbars:
        return "yerrorbars";
    case erorrbar_style_t::xerrorbars:
        return "xerrorbars";
    default:
        return "yerrorbars";
    }
}

std::string Gnuplot::terminal_type_to_string(terminal_type_t type)
{
    switch (type) {
    case terminal_type_t::cairolatex:
        return "cairolatex";
    case terminal_type_t::canvas:
        return "canvas";
    case terminal_type_t::cgm:
        return "cgm";
    case terminal_type_t::context:
        return "context";
    case terminal_type_t::domterm:
        return "domterm";
    case terminal_type_t::dpu414:
        return "dpu414";
    case terminal_type_t::dumb:
        return "dumb";
    case terminal_type_t::dxf:
        return "dxf";
    case terminal_type_t::emf:
        return "emf";
    case terminal_type_t::epscairo:
        return "epscairo";
    case terminal_type_t::epslatex:
        return "epslatex";
    case terminal_type_t::epson_180dpi:
        return "epson_180dpi";
    case terminal_type_t::epson_60dpi:
        return "epson_60dpi";
    case terminal_type_t::epson_lx800:
        return "epson_lx800";
    case terminal_type_t::fig:
        return "fig";
    case terminal_type_t::gif:
        return "gif";
    case terminal_type_t::hp500c:
        return "hp500c";
    case terminal_type_t::hpdj:
        return "hpdj";
    case terminal_type_t::hpgl:
        return "hpgl";
    case terminal_type_t::hpljii:
        return "hpljii";
    case terminal_type_t::hppj:
        return "hppj";
    case terminal_type_t::jpeg:
        return "jpeg";
    case terminal_type_t::lua:
        return "lua";
    case terminal_type_t::mf:
        return "mf";
    case terminal_type_t::mp:
        return "mp";
    case terminal_type_t::nec_cp6:
        return "nec_cp6";
    case terminal_type_t::okidata:
        return "okidata";
    case terminal_type_t::pbm:
        return "pbm";
    case terminal_type_t::pcl5:
        return "pcl5";
    case terminal_type_t::pdfcairo:
        return "pdfcairo";
    case terminal_type_t::pict2e:
        return "pict2e";
    case terminal_type_t::png:
        return "png";
    case terminal_type_t::pngcairo:
        return "pngcairo";
    case terminal_type_t::postscript:
        return "postscript";
    case terminal_type_t::pslatex:
        return "pslatex";
    case terminal_type_t::pstex:
        return "pstex";
    case terminal_type_t::pstricks:
        return "pstricks";
    case terminal_type_t::sixelgd:
        return "sixelgd";
    case terminal_type_t::sixeltek:
        return "sixeltek";
    case terminal_type_t::starc:
        return "starc";
    case terminal_type_t::svg:
        return "svg";
    case terminal_type_t::tandy_60dpi:
        return "tandy_60dpi";
    case terminal_type_t::tek40xx:
        return "tek40xx";
    case terminal_type_t::tek410x:
        return "tek410x";
    case terminal_type_t::texdraw:
        return "texdraw";
    case terminal_type_t::tikz:
        return "tikz";
    case terminal_type_t::tkcanvas:
        return "tkcanvas";
    case terminal_type_t::unknown:
        return "unknown";
    case terminal_type_t::vttek:
        return "vttek";
    case terminal_type_t::wxt:
        return "wxt";
    case terminal_type_t::x11:
        return "x11";
    case terminal_type_t::xlib:
        return "xlib";
    case terminal_type_t::xterm:
        return "xterm";
    default:
        return "wxt";
    }
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
    // Adjust the global temporary file counter
    Gnuplot::m_tmpfile_num -= static_cast<int>(tmpfile_list.size());
    // Clear the list of temporary files
    tmpfile_list.clear();
}

} // namespace gpcpp
