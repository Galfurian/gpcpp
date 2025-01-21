/// @file gnuplot.hpp
/// @brief A C++ interface to gnuplot.
/// @details
/// The interface uses pipes and so won't run on a system that doesn't have
/// POSIX pipe support Tested on Windows (MinGW and Visual C++) and Linux (GCC)

#pragma once

#include <unordered_set>
#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <sstream> // for std::ostringstream
#include <stdexcept>
#include <cstdio>
#include <cstdlib> // for getenv()
#include <list>    // for std::list

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__TOS_WIN__)
//defined for 32 and 64-bit environments
#include <io.h> // for _access(), _mktemp()

#elif defined(unix) || defined(__unix) || defined(__unix__) || defined(__APPLE__)
//all UNIX-like OSs (Linux, *BSD, MacOSX, Solaris, ...)
#include <unistd.h> // for access(), mkstemp()

#else
#error unsupported or unknown operating system
#endif

#include "gpcpp/color.hpp"
#include "gpcpp/defines.hpp"
#include "gpcpp/id_manager.hpp"
#include "gpcpp/box_style.hpp"

namespace gpcpp
{

/// @brief Main Gnuplot class for managing plots.
class Gnuplot {
public:
    /// @brief Constructs a Gnuplot session.
    Gnuplot(bool debug = false);

    /// @brief Destructor to clean up and delete temporary files.
    ~Gnuplot();

    /// @brief Sets the Gnuplot path manually.
    /// @details For Windows, ensure the path uses forward slashes ('/') instead of backslashes ('\').
    /// @param path The Gnuplot executable path.
    /// @return `true` if the path was successfully set, `false` otherwise.
    static bool set_gnuplot_path(const std::string &path);

    /// @brief Sets the default terminal type for displaying plots.
    /// @param type The terminal type to set (default is "wxt").
    /// @return void
    Gnuplot &set_terminal(terminal_type_t type = terminal_type_t::wxt);

    /// @brief Sends a command to the Gnuplot session.
    /// @param cmdstr The command string to send to Gnuplot.
    /// @return A reference to the current Gnuplot object.
    Gnuplot &send_cmd(const std::string &cmdstr);

    /// @brief Sends a command to an active Gnuplot session using the `<<` operator.
    /// @param cmdstr The command string to send to Gnuplot.
    /// @return A reference to the current Gnuplot object.
    Gnuplot &operator<<(const std::string &cmdstr)
    {
        this->send_cmd(cmdstr);
        return (*this);
    }

    /// @brief Displays the plot on the screen using the default terminal type.
    /// @details The terminal type defaults to:
    /// - Windows: "win"
    /// - Linux: "x11"
    /// - macOS: "aqua"
    /// @return A reference to the current Gnuplot object.
    Gnuplot &show();

    /// @brief Saves the current plot to a file.
    /// @param filename The name of the output file.
    /// @return A reference to the current Gnuplot object.
    Gnuplot &set_output(const std::string filename);

    /// Sets the plotting style for the current Gnuplot session.
    /// @param style The plot_type_t enum value representing the desired plotting style.
    /// @return Reference to the current Gnuplot object.
    Gnuplot &set_plot_type(plot_type_t style);

    /// @brief Sets the smoothing style for the current Gnuplot session.
    /// @param style The smooth_type enum value representing the desired smoothing style.
    /// @return Reference to the current Gnuplot object.
    Gnuplot &set_smooth_type(smooth_type_t style = smooth_type_t::csplines);

    /// @brief Sets the line style for the Gnuplot plot.
    /// @param style The line style to set (solid, dashed, custom, etc.).
    /// @param custom_pattern Optional custom dash pattern if the style is set to custom.
    /// @return Reference to the Gnuplot object for chaining.
    Gnuplot &set_line_type(line_type_t style, const std::string &custom_pattern = "");

    /// @brief Sets the line color for the Gnuplot plot.
    /// @param color The line color (e.g., "red", "#ff0000").
    /// @return Reference to the Gnuplot object for chaining.
    Gnuplot &set_line_color(const std::string &color);

    /// @brief Sets the line color for the Gnuplot plot using RGB values.
    /// @param r The red component of the color (0 to 255).
    /// @param g The green component of the color (0 to 255).
    /// @param b The blue component of the color (0 to 255).
    /// @return Reference to the Gnuplot object for chaining.
    Gnuplot &set_line_color(int r, int g, int b);

    /// @brief Sets the style of points used in plots.
    /// @param style An integer specifying the Gnuplot point style.
    /// @return Reference to the current Gnuplot object.
    Gnuplot &set_point_type(point_type_t style);

    /// @brief Sets the size of points used in plots.
    /// @param size A positive value specifying the Gnuplot point size.
    /// @return Reference to the current Gnuplot object.
    Gnuplot &set_point_size(double size);

    /// @brief Sets the line width for the current Gnuplot session.
    /// @param width The desired line width. Must be greater than 0.
    /// @return Reference to the current Gnuplot object.
    Gnuplot &set_line_width(double width);

    /// @brief Enables the grid for plots.
    /// @return A reference to the current Gnuplot object.
    Gnuplot &set_grid();

    /// @brief Sets the major tics for the x-axis.
    /// @param major_step The step size for major tics on the x-axis.
    /// @return Gnuplot& Reference to the Gnuplot object.
    Gnuplot &set_xtics_major(double major_step);

    /// @brief Sets the minor tics for the x-axis.
    /// @param minor_intervals The number of minor intervals between major tics.
    /// @return Gnuplot& Reference to the Gnuplot object.
    Gnuplot &set_xtics_minor(int minor_intervals);

    /// @brief Sets the major tics for the y-axis.
    /// @param major_step The step size for major tics on the y-axis.
    /// @return Gnuplot& Reference to the Gnuplot object.
    Gnuplot &set_ytics_major(double major_step);

    /// @brief Sets the minor tics for the y-axis.
    /// @param minor_intervals The number of minor intervals between major tics.
    /// @return Gnuplot& Reference to the Gnuplot object.
    Gnuplot &set_ytics_minor(int minor_intervals);

    /// @brief Sets the line style for a specified grid type.
    /// @param grid_type The type of grid to configure (major, minor, or polar).
    /// @param style The line style for the grid (e.g., solid, dashed).
    /// @param color The line color for the grid.
    /// @param width The line width for the grid.
    /// @param custom_dash The custom dash pattern (only used if style is `custom`).
    /// @return Gnuplot& Reference to the Gnuplot object.
    Gnuplot &set_grid_line_type(grid_type_t grid_type,
                                line_type_t style,
                                const Color &color,
                                double width,
                                const std::string &custom_dash = "");

    /// @brief Builds and applies the grid configuration.
    /// @param tics A string specifying which tics to enable (e.g., "xtics ytics").
    /// @param layer Specify "front" or "back" for the grid layer.
    /// @param vertical_lines Enable or disable vertical grid lines.
    /// @return Gnuplot& Reference to the Gnuplot object.
    Gnuplot &
    apply_grid(const std::string &tics = "xtics ytics", const std::string &layer = "back", bool vertical_lines = true);

    /// @brief Disables the grid for plots.
    /// @details The grid is not enabled by default.
    /// @return A reference to the current Gnuplot object.
    Gnuplot &unset_grid();

    /// @brief Enables multiplot mode for displaying multiple plots in one session.
    /// @return A reference to the current Gnuplot object.
    Gnuplot &set_multiplot();

    /// @brief Disables multiplot mode.
    /// @return A reference to the current Gnuplot object.
    Gnuplot &unset_multiplot();

    /// @brief Sets the origin and size of the plot area in the Gnuplot window.
    /// @param x_origin The x-coordinate of the origin of the plot area (in the range 0 to 1).
    /// @param y_origin The y-coordinate of the origin of the plot area (in the range 0 to 1).
    /// @param width The width of the plot area (relative to the window size, in the range 0 to 1).
    /// @param height The height of the plot area (relative to the window size, in the range 0 to 1).
    /// @details This function sets the origin and size of the plot area in the Gnuplot window,
    /// allowing for customized positioning and scaling of the plot within the window.
    /// The values for the origin (x_origin, y_origin) and size (width, height) should be given
    /// as fractions of the total window size (e.g., 0.5 for half the window).
    /// @return A reference to the current Gnuplot object, allowing for method chaining.
    Gnuplot &set_origin_and_size(double x_origin, double y_origin, double width, double height);

    /// @brief Sets the sampling rate for plotting functions or interpolating data.
    /// @param samples The number of samples (default is 100).
    /// @return A reference to the current Gnuplot object.
    Gnuplot &set_samples(const int samples = 100);

    /// @brief Sets the isoline density for plotting surfaces in 3D plots.
    /// @param isolines The number of isolines (default is 10).
    /// @return A reference to the current Gnuplot object.
    Gnuplot &set_isosamples(const int isolines = 10);

    /// @brief Sets the contour type for Gnuplot.
    /// @param type The desired contour type (base, surface, both, none).
    /// @return Reference to the current Gnuplot object.
    Gnuplot &set_contour_type(contour_type_t type);

    /// @brief Configures contour levels based on the specified parameter type.
    /// @param param The desired contour parameter type (levels, increment, discrete).
    /// @return Reference to the current Gnuplot object.
    Gnuplot &set_contour_param(contour_param_t param);

    /// @brief Sets the number of contour levels.
    /// @param levels The number of contour levels.
    /// @return Reference to the current Gnuplot object.
    Gnuplot &set_contour_levels(int levels);

    /// @brief Sets the contour increment range and step size.
    /// @param start The start value of the increment range.
    /// @param step The step size for the increments.
    /// @param end The end value of the increment range.
    /// @return Reference to the current Gnuplot object.
    Gnuplot &set_contour_increment(double start, double step, double end);

    /// @brief Sets discrete contour levels.
    /// @param levels A vector of specific levels to be used as contours.
    /// @return Reference to the current Gnuplot object.
    Gnuplot &set_contour_discrete_levels(const std::vector<double> &levels);

    /// @brief Sends the configured contour commands to Gnuplot.
    /// @return Reference to the current Gnuplot object.
    Gnuplot &apply_contour_settings();

    /// @brief Enables hidden line removal for surface plotting in 3D plots.
    /// @details Hidden line removal improves the visualization of surfaces by hiding lines obscured by the surface.
    /// @return A reference to the current Gnuplot object.
    Gnuplot &set_hidden3d();

    /// @brief Disables hidden line removal for surface plotting in 3D plots.
    /// @details Hidden line removal is not enabled by default.
    /// @return A reference to the current Gnuplot object.
    Gnuplot &unset_hidden3d();

    /// @brief Disables contour drawing for surfaces in 3D plots.
    /// @details Contour drawing is not enabled by default.
    /// @return A reference to the current Gnuplot object.
    Gnuplot &unset_contour();

    /// @brief Enables the display of surfaces in 3D plots.
    /// @return A reference to the current Gnuplot object.
    Gnuplot &set_surface();

    /// @brief Disables the display of surfaces in 3D plots.
    /// @details Surface display is enabled by default.
    /// @return A reference to the current Gnuplot object.
    Gnuplot &unset_surface();

    /// @brief Sets the legend (key) properties in Gnuplot.
    /// @details
    /// This function allows you to customize the position, font, title, box visibility,
    /// and spacing of the legend in the plot.
    /// The position can be a keyword (e.g., "top left") or a specific position
    /// (e.g., "graph 0.5, 0.5").
    ///
    /// @param position The position of the legend. If empty, no title is set.
    /// @param font The font and size of the legend text. The format is "fontname,size" (e.g., "Arial,12").
    /// @param title The title of the legend. Default is empty (no title).
    /// @param with_box Whether to show a box around the legend (`true` for box, `false` for no box).
    /// @param spacing The spacing between legend entries. Default is 1.0 (no extra spacing).
    /// @param width The width of the legend box. Default is `2.0`.
    ///
    /// @return Gnuplot& Returns the Gnuplot object itself for method chaining.
    Gnuplot &set_legend(const std::string &position = "default",
                        const std::string &font     = "",
                        const std::string &title    = "",
                        bool with_box               = true,
                        double spacing              = 1.0,
                        double width                = 2.0);

    /// @brief Sets the title of the plot.
    /// @param title The title of the plot (default is an empty string).
    /// @return A reference to the current Gnuplot object.
    Gnuplot &set_title(const std::string &title = "");

    /// @brief Clears the title of the plot.
    /// @details The title is not set by default.
    /// @return A reference to the current Gnuplot object.
    Gnuplot &unset_title();

    /// @brief Sets the label for the x-axis.
    /// @param label The label for the x-axis (default is "x").
    /// @return A reference to the current Gnuplot object.
    Gnuplot &set_xlabel(const std::string &label = "x");

    /// @brief Sets the label for the y-axis.
    /// @param label The label for the y-axis (default is "y").
    /// @return A reference to the current Gnuplot object.
    Gnuplot &set_ylabel(const std::string &label = "y");

    /// @brief Sets the label for the z-axis.
    /// @param label The label for the z-axis (default is "z").
    /// @return A reference to the current Gnuplot object.
    Gnuplot &set_zlabel(const std::string &label = "z");

    /// @brief Sets the range for the x-axis.
    /// @param iFrom The starting value of the range.
    /// @param iTo The ending value of the range.
    /// @return A reference to the current Gnuplot object.
    Gnuplot &set_xrange(const double iFrom, const double iTo);

    /// @brief Sets the range for the y-axis.
    /// @param iFrom The starting value of the range.
    /// @param iTo The ending value of the range.
    /// @return A reference to the current Gnuplot object.
    Gnuplot &set_yrange(const double iFrom, const double iTo);

    /// @brief Sets the range for the z-axis.
    /// @param iFrom The starting value of the range.
    /// @param iTo The ending value of the range.
    /// @return A reference to the current Gnuplot object.
    Gnuplot &set_zrange(const double iFrom, const double iTo);
    /// @brief Enables autoscaling for the x-axis.
    /// @details Autoscaling is enabled by default.
    /// @return A reference to the current Gnuplot object.
    Gnuplot &set_xautoscale();

    /// @brief Enables autoscaling for the y-axis.
    /// @details Autoscaling is enabled by default.
    /// @return A reference to the current Gnuplot object.
    Gnuplot &set_yautoscale();

    /// @brief Enables autoscaling for the z-axis.
    /// @details Autoscaling is enabled by default.
    /// @return A reference to the current Gnuplot object.
    Gnuplot &set_zautoscale();

    /// @brief Enables logarithmic scaling for the x-axis.
    /// @details Logarithmic scaling is not enabled by default.
    /// @param base The base of the logarithm (default is 10).
    /// @return A reference to the current Gnuplot object.
    Gnuplot &set_xlogscale(const double base = 10);

    /// @brief Enables logarithmic scaling for the y-axis.
    /// @details Logarithmic scaling is not enabled by default.
    /// @param base The base of the logarithm (default is 10).
    /// @return A reference to the current Gnuplot object.
    Gnuplot &set_ylogscale(const double base = 10);

    /// @brief Enables logarithmic scaling for the z-axis.
    /// @details Logarithmic scaling is not enabled by default.
    /// @param base The base of the logarithm (default is 10).
    /// @return A reference to the current Gnuplot object.
    Gnuplot &set_zlogscale(const double base = 10);

    /// @brief Disables logarithmic scaling for the x-axis.
    /// @return A reference to the current Gnuplot object.
    Gnuplot &unset_xlogscale();

    /// @brief Disables logarithmic scaling for the y-axis.
    /// @return A reference to the current Gnuplot object.
    Gnuplot &unset_ylogscale();

    /// @brief Disables logarithmic scaling for the z-axis.
    /// @return A reference to the current Gnuplot object.
    Gnuplot &unset_zlogscale();

    /// @brief Sets the palette color range for plots.
    /// @details The palette range is used to map data values to colors in plots.
    /// Autoscaling is enabled by default, but this function allows manual control of the range.
    /// @param iFrom The starting value of the color range.
    /// @param iTo The ending value of the color range.
    /// @return A reference to the current Gnuplot object.
    Gnuplot &set_cbrange(const double iFrom, const double iTo);

    /// @brief Sets a vertical line at a given x position..
    /// @param x The x-coordinate where the vertical line should be placed.
    /// @return Reference to the Gnuplot object for chaining.
    Gnuplot &plot_vertical_line(double x);

    /// @brief Sets a horizontal line at a given y position.
    /// @param y The y-coordinate where the horizontal line should be placed.
    /// @return Reference to the Gnuplot object for chaining.
    Gnuplot &plot_horizontal_line(double y);

    /// @brief Sets a vertical line over a range of y values at a given x position.
    /// @param x The x-coordinate where the vertical line should be placed.
    /// @param y_min The minimum y-coordinate of the range.
    /// @param y_max The maximum y-coordinate of the range.
    /// @return Reference to the Gnuplot object for chaining.
    Gnuplot &plot_vertical_range(double x, double y_min, double y_max);

    /// @brief Sets a horizontal line over a range of x values at a given y position.
    /// @param y The y-coordinate where the horizontal line should be placed.
    /// @param x_min The starting x-coordinate of the range.
    /// @param x_max The ending x-coordinate of the range.
    /// @return Reference to the Gnuplot object for chaining.
    Gnuplot &plot_horizontal_range(double y, double x_min, double x_max);

    /// @brief Adds a label at a specific point on the plot with customizable alignment, optional point, and optional box.
    ///
    /// @param x The x-coordinate of the label.
    /// @param y The y-coordinate of the label.
    /// @param label The text of the label.
    /// @param font_size The font size for the label text (optional).
    /// @param color The color of the label (optional).
    /// @param offset_x X-axis offset for label positioning (optional).
    /// @param offset_y Y-axis offset for label positioning (optional).
    /// @param horizontal_alignment The horizontal alignment for the label (optional).
    /// @param rotation The rotation of the label text in degrees (optional).
    /// @param point_type Optionally display a point at the label (optional).
    /// @param box_style The box style to apply to the label (optional).
    /// @return Reference to the Gnuplot object for chaining.
    Gnuplot &add_label(double x,
                       double y,
                       const std::string &label,
                       double font_size              = 12.0,
                       const std::string &color      = "black",
                       double offset_x               = 0.0,
                       double offset_y               = 0.0,
                       halign_t horizontal_alignment = halign_t::center,
                       double rotation               = 0.0,
                       bool point_type               = false,
                       const box_style_t &box_style  = box_style_t());

    /// @brief Plots a single vector of data.
    /// @tparam X The type of the data in the vector.
    /// @param x The data to plot.
    /// @param title The title of the plot (default is an empty string).
    /// @return A reference to the current Gnuplot object.
    template <typename X>
    Gnuplot &plot_x(const X &x, const std::string &title = "");

    /// @brief Plots multiple vectors with separate titles.
    /// @tparam X The type of the data in the vectors.
    /// @param datasets The vectors of datasets to plot.
    /// @param titles The titles for each vector.
    /// @return A reference to the current Gnuplot object.
    template <typename X>
    Gnuplot &plot_x(const std::vector<X> &datasets, const std::vector<std::string> &titles);

    /// @brief Plots x, y pairs of data.
    /// @tparam X The type of the x data.
    /// @tparam Y The type of the y data.
    /// @param x The x values.
    /// @param y The y values.
    /// @param title The title of the plot (default is an empty string).
    /// @return A reference to the current Gnuplot object.
    template <typename X, typename Y>
    Gnuplot &plot_xy(const X &x, const Y &y, const std::string &title = "");

    /// @brief Plots x, y pairs with error bars (x, y, dy).
    /// @tparam X The type of the x data.
    /// @tparam Y The type of the y data.
    /// @tparam E The type of the error data.
    /// @param x The x values.
    /// @param y The y values.
    /// @param dy The error values.
    /// @param style The error bar style (default is errorbars).
    /// @param title The title of the plot (default is an empty string).
    /// @return A reference to the current Gnuplot object.
    template <typename X, typename Y, typename E>
    Gnuplot &plot_xy_erorrbar(const X &x,
                              const Y &y,
                              const E &dy,
                              erorrbar_type_t style    = erorrbar_type_t::yerrorbars,
                              const std::string &title = "");

    /// @brief Plots x, y, z triples of data.
    /// @tparam X The type of the x data.
    /// @tparam Y The type of the y data.
    /// @tparam Z The type of the z data.
    /// @param x The x values.
    /// @param y The y values.
    /// @param z The z values.
    /// @param title The title of the plot (default is an empty string).
    /// @return A reference to the current Gnuplot object.
    template <typename X, typename Y, typename Z>
    Gnuplot &plot_xyz(const X &x, const Y &y, const Z &z, const std::string &title = "");

    /// @brief Plots a 3D grid of data points.
    /// @param x A vector of x-coordinates.
    /// @param y A vector of y-coordinates.
    /// @param z A 2D vector of z-values (size: x.size() × y.size()).
    /// @param title Title for the plot.
    /// @return Reference to the current Gnuplot object.
    template <typename X, typename Y, typename Z>
    Gnuplot &plot_3d_grid(const X &x, const Y &y, const Z &z, const std::string &title = "");

    /// @brief Plots a linear equation of the form y = ax + b.
    /// @param a The slope of the line.
    /// @param b The y-intercept of the line.
    /// @param title The title of the plot (default is an empty string).
    /// @return A reference to the current Gnuplot object.
    Gnuplot &plot_slope(const double a, const double b, const std::string &title = "");

    /// @brief Plots a 2D equation of the form y = f(x).
    /// @param equation The equation to plot (e.g., "sin(x)").
    /// @param title The title of the plot (default is an empty string).
    /// @return A reference to the current Gnuplot object.
    Gnuplot &plot_equation(const std::string &equation, const std::string &title = "");

    /// @brief Plots a 3D equation of the form z = f(x, y).
    /// @param equation The equation to plot (e.g., "x^2 + y^2").
    /// @param title The title of the plot (default is an empty string).
    /// @return A reference to the current Gnuplot object.
    Gnuplot &plot_equation3d(const std::string &equation, const std::string &title = "");

    /// @brief Plots an image.
    /// @param ucPicBuf The image data buffer.
    /// @param iWidth The width of the image.
    /// @param iHeight The height of the image.
    /// @param title The title of the plot (default is an empty string).
    /// @return A reference to the current Gnuplot object.
    Gnuplot &plot_image(const unsigned char *ucPicBuf,
                        const unsigned int iWidth,
                        const unsigned int iHeight,
                        const std::string &title = "");

    /// @brief Repeats the last plot or splot command.
    /// @details Useful for viewing the same plot with different settings or generating it for multiple devices (e.g., screen or file).
    /// @return A reference to the current Gnuplot object.
    Gnuplot &replot();

    /// @brief Resets the current Gnuplot session.
    /// @details The next plot will erase all previous ones.
    /// @return A reference to the current Gnuplot object.
    Gnuplot &reset_plot();

    /// @brief Resets the Gnuplot session and restores all variables to their default values.
    /// @return A reference to the current Gnuplot object.
    Gnuplot &reset_all();

    /// @brief Deletes all temporary files created during the session.
    void remove_tmpfiles();

    /// @brief Checks if the current Gnuplot session is valid.
    /// @return `true` if the session is valid, `false` otherwise.
    bool is_ready() const;

private:
    /// @brief Initializes the Gnuplot session.
    /// Sets up necessary configurations and opens the Gnuplot pipe.
    void init();

    /// @brief Creates a unique temporary file and returns its name.
    ///
    /// This function generates a temporary file with a unique name
    /// using platform-specific methods (`_mktemp` on Windows and `mkstemp` on Unix).
    /// It ensures that the maximum allowed number of temporary files is not exceeded.
    /// The file is opened for writing, and its name is stored for cleanup.
    ///
    /// @param tmp A reference to an `std::ofstream` object where the file will be opened.
    ///
    /// @return The name of the created temporary file.
    ///
    /// @throws GnuplotException If the maximum number of temporary files is reached
    ///         or if the temporary file cannot be created or opened.
    std::string create_tmpfile(std::ofstream &tmp);

    /// @brief Checks if the Gnuplot executable path is valid.
    /// @return `true` if the Gnuplot path is found, `false` otherwise.
    static bool get_program_path();

    /// @brief Checks if a file is available for use.
    /// @param filename The name of the file to check.
    /// @return `true` if the file exists and is accessible, `false` otherwise.
    static bool file_ready(const std::string &filename);

    /// @brief Checks if a file exists and satisfies the specified mode.
    /// @param filename The name of the file to check.
    /// @param mode The access mode to check (e.g., read, write, execute). Defaults to 0 (existence only).
    /// @return `true` if the file exists and satisfies the mode, `false` otherwise.
    static bool file_exists(const std::string &filename, int mode = 0);

    /// @brief Enables debug.
    bool debug;

    /// @brief pointer to the stream that can be used to write to the pipe
    FILE *gnuplot_pipe;
    /// @brief standart terminal, used by show.
    terminal_type_t terminal_type;
    /// @brief validation of gnuplot session
    bool valid;
    /// @brief true = 2d, false = 3d
    bool two_dim;
    /// @brief number of plots in session
    int nplots;

    /// The line width for plotted lines.
    double line_width;
    /// The type used for plotting data (e.g., lines, points, histograms).
    plot_type_t plot_type;
    /// The smoothing type applied to the data (e.g., csplines, bezier).
    smooth_type_t smooth_type;
    /// @brief Define the line type for Gnuplot plots
    std::string line_type;
    /// @brief The line color in Gnuplot-compatible format (e.g., "red", "#ff0000").
    gpcpp::Color line_color;
    /// @brief Specifies the point type.
    point_type_t point_type;
    /// @brief Specifies the size of points.
    double point_size = -1.0;

    struct {
        contour_type_t type   = contour_type_t::none;    ///< Default: no contours
        contour_param_t param = contour_param_t::levels; ///< Default: levels
        std::vector<double> discrete_levels;             ///< For discrete contour levels
        double increment_start = 0.0;                    ///< Start of increment range
        double increment_step  = 0.1;                    ///< Step size for increments
        double increment_end   = 1.0;                    ///< End of increment range
        int levels             = 10;                     ///< Number of contour levels
    } contour;

    /// @brief list of created tmpfiles.
    std::vector<std::string> tmpfile_list;

    /// @brief ID for major grid style.
    int grid_major_style_id;
    /// @brief ID for minor grid style.
    int grid_minor_style_id;

    /// @brief number of all tmpfiles (number of tmpfiles restricted)
    static int m_tmpfile_num;
    /// @brief name of executed GNUPlot file
    static std::string m_gnuplot_filename;
    /// @brief gnuplot path
    static std::string m_gnuplot_path;

    /// @brief Keeps track of the used IDs for the line styles.
    id_manager_t id_manager_line_style;
    /// @brief Keeps track of the used IDs for the textbox styles.
    id_manager_t id_manager_textbox_style;
};

} // namespace gpcpp

#include "gnuplot.i.hpp"