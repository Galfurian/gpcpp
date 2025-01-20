/// @file example_custom_line_style_color.cpp
/// @brief An example demonstrating how to set a custom line style (e.g., dashed) and a custom line color (e.g., blue).
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

    // Set custom line style and color (dashed, blue)
    gnuplot
        .set_grid()                                         // Show the grid.
        .set_plot_style(plot_style_t::lines)                // Set the plot style to line.
        .set_line_style(line_style_t::custom, "30,10")       // Set line style to custom dashed.
        .set_line_width(2.0)                                // Set line width
        .set_title("Plot with Custom Line Style and Color") // Set plot title
        .plot_xy(x, y);                                     // Plot the x, y pairs

    // Wait for user input before closing
    std::cout << "Press Enter to exit..." << std::endl;
    std::cin.get();

    return 0;
}
