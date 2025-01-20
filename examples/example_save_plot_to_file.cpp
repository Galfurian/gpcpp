/// @file example_save_plot_to_file.cpp
/// @brief An example showing how to save the plot as an image or PDF file using a specified terminal type.
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

    // Set terminal to save the plot as a PNG file
    gnuplot
        .set_terminal(terminal_type_t::pngcairo) // Set terminal to PNG format
        .set_output("output_plot.png")           // Set output file name
        .set_title("Saving Plot to File")        // Set plot title
        .plot_xy(x, y);                          // Plot the x, y pairs

    std::cout << "Plot saved to output_plot.png" << std::endl;

    // Wait for user input before closing
    std::cin.get();

    return 0;
}
