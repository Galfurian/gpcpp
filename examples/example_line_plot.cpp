/// @file example_simple_line_plot.cpp
/// @brief A simple example demonstrating how to plot a 2D line (e.g., y = x^2)
/// using Gnuplot.
/// @copyright Copyright (c) 2025 Enrico Fraccaroli <enry.frak@gmail.com>

#include <cmath>
#include <iostream>
#include <vector>

#include <gpcpp/gnuplot.hpp>

int main()
{
    using namespace gpcpp;

    // Create a Gnuplot instance
    Gnuplot gnuplot(true);

    // Prepare data for plotting
    std::vector<double> x, y;
    for (unsigned int i = 0; i < 50; i++) {
        x.push_back(static_cast<double>(i)); // x[i] = i
        y.push_back(x[i] * x[i]);            // y[i] = i^2
    }

    // Plot the data as a line
    gnuplot
        .set_title("Simple Plot of y = x^2") // Set plot title.
        .set_grid()                          // Show the grid.
        // Create a vertical line.
        .set_line_type(line_type_t::dash_dot)
        .set_line_width(2.0)
        .set_line_color(125, 255, 125)
        .plot_vertical_line(10)
        // Create an horizontal line.
        .set_line_type(line_type_t::dash_dot_dot)
        .set_line_width(3.0)
        .set_line_color(125, 125, 255)
        .plot_horizontal_line(500)
        // Create an vertical range.
        .set_line_type(line_type_t::solid)
        .set_line_width(2.0)
        .set_line_color(255, 125, 255)
        .plot_vertical_range(5, 250, 500)
        // Create an horizontal range.
        .set_line_type(line_type_t::solid)
        .set_line_width(2.0)
        .set_line_color(255, 125, 125)
        .plot_horizontal_range(250, 5, 25)
        // Plot the line.
        .set_plot_type(plot_type_t::lines)  // Set the plot type to line.
        .set_line_type(line_type_t::dashed) // Set the line type.
        .set_line_color("blue")             // Set line color to blue.
        .set_line_width(2.0)                // Set line width.
        .plot_xy(x, y)                      // Plot the x, y pairs.
        .show();

    return 0;
}
