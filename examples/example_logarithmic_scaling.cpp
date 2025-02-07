/// @file example_logarithmic_scaling.cpp
/// @brief An example demonstrating how to set logarithmic scaling on the x, y, or z axes.
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
        x.push_back(static_cast<double>(i));              // x[i] = i
        y.push_back(std::pow(static_cast<double>(i), 2)); // y[i] = i^2
    }

    // Plot the data as a line
    gnuplot.set_title("Logarithmic Scaling on x-axis")
        .set_xlabel("X (log scale)")       // Label for the x-axis
        .set_ylabel("Y = X^2")             // Label for the y-axis
        .set_grid()                        // Show the grid.
        .set_xlogscale()                   // Set logarithmic scale on x-axis
        .set_plot_type(plot_type_t::lines) // Set the plot type to line.
        .set_line_color("blue")            // Set line color to blue.
        .set_line_width(2.0)               // Set line width.
        .plot_xy(x, y)                     // Plot the x, y pairs.
        .show();

    return 0;
}
