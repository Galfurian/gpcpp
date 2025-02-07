/// @file example_custom_grid.cpp
/// @brief A simple example demonstrating how to customize the grid appearance
/// (major and minor) using Gnuplot.
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
    for (unsigned int i = 0; i <= 50; i++) {
        x.push_back(static_cast<double>(i)); // x[i] = i
        y.push_back(std::sqrt(x[i]));        // y[i] = sqrt(i)
    }

    // Set up the plot
    gnuplot.set_terminal(gpcpp::terminal_type_t::wxt)
        .set_title("Custom Grid Example") // Set plot title.
        .set_xlabel("x-axis")             // Set x-axis label.
        .set_ylabel("y-axis")             // Set y-axis label.
        // Set tics.
        .set_xtics_major(10)
        .set_xtics_minor(2)
        .set_ytics_major(1)
        .set_ytics_minor(2)
        // Configure major grid.
        .set_grid_line_type(grid_type_t::major, line_type_t::solid, Color("#88FF0000"), 1.0)
        // Configure minor grid.
        .set_grid_line_type(grid_type_t::minor, line_type_t::dashed, Color("gray"), 0.5)
        // Apply grid configuration.
        .apply_grid("xtics ytics mxtics mytics", "back")
        .set_plot_type(plot_type_t::lines) // Set the plot type to line.
        .set_line_type(line_type_t::solid) // Set the line type for data.
        .set_line_color("green")           // Set the data line color to green.
        .set_line_width(2.0);              // Set line width.

    // Plot the data as a line
    gnuplot.plot_xy(x, y, "y = sqrt(x)"); // Plot the x, y pairs with a label.

    // Show the plot
    gnuplot.show();

    return 0;
}
