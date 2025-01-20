/// @file example_multiple_plots_one_window.cpp
/// @brief An example showing how to use `multiplot` mode to display multiple plots within the same window.
/// @copyright Copyright (c) 2025 Enrico Fraccaroli <enry.frak@gmail.com>

#include <iostream>
#include <vector>
#include <cmath>
#include <gpcpp/gnuplot.hpp>

int main()
{
    using namespace gpcpp;

    // Create a Gnuplot instance
    Gnuplot gnuplot;

    // Prepare data for plotting
    std::vector<double> x, y;
    for (unsigned int i = 0; i < 50; i++) {
        x.push_back(static_cast<double>(i)); // x[i] = i
        y.push_back(x[i] * x[i]);            // y[i] = x[i]^2
    }

    // Set layout to 2x1 grid (2 rows, 1 column)
    gnuplot.set_multiplot();
    gnuplot.set_terminal(terminal_type_t::pngcairo);

    // First plot: Line plot in the top half of the window
    gnuplot.set_origin_and_size(0, 0.5, 1, 0.5); // Set origin and size for the first plot
    gnuplot.set_title("Plot 1: Line Plot").set_line_width(2.0).set_xlabel("x-axis").set_ylabel("y-axis").plot_xy(x, y);

    // Second plot: Points plot in the bottom half of the window
    gnuplot.set_origin_and_size(0, 0, 1, 0.5); // Set origin and size for the second plot
    gnuplot.set_title("Plot 2: Points Plot")
        .set_point_style(point_style_t::filled_circle) // Set point style to filled circle
        .set_line_width(0)                             // No line, only points
        .plot_xy(x, y);

    // End multiplot mode
    gnuplot.unset_multiplot();

    gnuplot.show();

    return 0;
}
