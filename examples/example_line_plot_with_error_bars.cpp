/// @file example_line_plot_with_error_bars.cpp
/// @brief An example demonstrating how to plot a 2D line plot with error bars
/// along the y-axis using Gnuplot.
/// @copyright Copyright (c) 2025 Enrico Fraccaroli <enry.frak@gmail.com>

#include <cmath>
#include <gpcpp/gnuplot.hpp>
#include <iostream>
#include <vector>

int main()
{
    using namespace gpcpp;

    // Create a Gnuplot instance
    Gnuplot gnuplot(true);

    // Prepare data for plotting
    std::vector<double> x, y, dy;
    for (unsigned int i = 0; i < 50; i++) {
        x.push_back(static_cast<double>(i)); // x[i] = i
        y.push_back(x[i] * x[i]);            // y[i] = x[i]^2
        dy.push_back(x[i] * 10);             // dy[i] = 0.1 * x[i] (error bars)
    }

    // Plot the data as a line with error bars
    gnuplot
        .set_title("Line Plot with Error Bars") // Set plot title
        .set_grid()                             // Show the grid.
        // Plot the line.
        .set_plot_type(plot_type_t::lines) // Set the plot type to line.
        .set_line_type(line_type_t::solid) // Set the line type.
        .set_line_color("red")             // Set line color to blue
        .set_line_width(2.0)               // Set line width
        .plot_xy(x, y, "Data")             // Plot with lines
        // Then, plot the erro bar.
        .set_line_color("blue")                                  // Set line color to blue
        .set_line_width(2.0)                                     // Set line width
        .plot_xy_erorrbar(x, y, dy, erorrbar_type_t::yerrorbars) // Plot with error bars along y-axis
        .show();

    return 0;
}
