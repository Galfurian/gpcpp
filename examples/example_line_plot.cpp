/// @file example_simple_line_plot.cpp
/// @brief A simple example demonstrating how to plot a 2D line (e.g., y = x^2)
/// using Gnuplot.
/// @copyright Copyright (c) 2025 Enrico Fraccaroli <enry.frak@gmail.com>

#include <iostream>
#include <vector>
#include <cmath>

#include <gnuplotcpp/gnuplot.hpp>

int main()
{
    using namespace gnuplotcpp;

    // Create a Gnuplot instance
    Gnuplot gnuplot;

    // Prepare data for plotting
    std::vector<double> x, y;
    for (unsigned int i = 0; i < 50; i++) {
        x.push_back(static_cast<double>(i)); // x[i] = i
        y.push_back(x[i] * x[i]);            // y[i] = i^2
    }

    // Plot the data as a line
    gnuplot
        .set_grid()                           // Show the grid.
        .set_plot_style(plot_style_t::lines)  // Set the plot style to line.
        .set_line_style(line_style_t::dashed) // Set the line stype.
        .set_line_color("blue")               // Set line color to blue.
        .set_line_width(2.0)                  // Set line width.
        .set_title("Simple Plot of y = x^2")  // Set plot title.
        .plot_xy(x, y);                       // Plot the x, y pairs.

    // Wait for user input before closing
    std::cout << "Press Enter to exit..." << std::endl;
    std::cin.get();

    return 0;
}
