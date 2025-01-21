/// @file example_scatter_plot.cpp
/// @brief An example demonstrating how to plot data points with custom styles
/// (filled and open circles, squares) using Gnuplot.
/// @copyright Copyright (c) 2025 Enrico Fraccaroli <enry.frak@gmail.com>

#include <iostream>
#include <vector>
#include <cmath>

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
        y.push_back(std::sin(x[i] * 0.1));   // y[i] = sin(x[i] * 0.1)
    }

    // Plot the data as scatter points with custom styles
    gnuplot
        .set_grid()                                  // Show the grid.
        .set_plot_type(plot_type_t::points)          // Set the plot type to points.
        .set_point_type(point_type_t::open_triangle) // Set filled circle for points
        .set_point_size(1.5)                         // Set point size
        .set_line_width(0)                           // No line connecting points
        .plot_xy(x, y)                               // Plot the x, y pairs
        .show();

    return 0;
}
