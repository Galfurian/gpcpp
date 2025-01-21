/// @file example_labels.cpp
/// @brief An example demonstrating how to add labels to a Gnuplot plot with different alignments.
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
        y.push_back(x[i] * x[i]);            // y[i] = i^2
    }

    // Plot the data as a line
    gnuplot
        .set_title("Plot with Labels") // Set plot title.
        .set_grid()                    // Show the grid.
        // Add labels with different alignments and options for points and boxes.
        .add_label(5, 500, "Left", 12.0, "red", 0.0, 0.0, halign_t::left, 0.0, true,
                   { true, true, "yellow", true, "gray", 1.0, 1, 1 })
        .add_label(10, 1000, "Center", 12.0, "green", 0.0, 0.0, halign_t::center, 0.0, true,
                   { true, true, "yellow", true, "gray", 1.0, 1, 1 })
        .add_label(15, 1500, "Right", 12.0, "blue", 0.0, 0.0, halign_t::right, 0.0, true,
                   { true, true, "yellow", true, "gray", 1.0, 1, 1 })
        // Create the plot.
        .set_plot_type(plot_type_t::lines) // Set the plot type to line.
        .set_line_type(line_type_t::solid) // Set the line type.
        .set_line_color("blue")              // Set line color to blue.
        .set_line_width(2.0)                 // Set line width.
        .plot_xy(x, y)                       // Plot the x, y pairs.
        .show();
    return 0;
}
