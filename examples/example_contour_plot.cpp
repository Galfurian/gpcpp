/// @file example_contour_plot.cpp
/// @brief An example demonstrating how to apply contours to a 3D surface plot (contours on base, surface, or both).
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

    // Define grid size for the 3D plot
    const unsigned int grid_size = 50;
    std::vector<double> x(grid_size), y(grid_size);
    std::vector<std::vector<double>> z(grid_size, std::vector<double>(grid_size));

    // Set up the ranges for x and y axes
    double x_min = -5.0, x_max = 5.0;
    double y_min = -5.0, y_max = 5.0;

    // Generate the grid data for x, y, and z = sin(x) * cos(y)
    for (unsigned int i = 0; i < grid_size; ++i) {
        x[i] = x_min + (x_max - x_min) * i / (grid_size - 1);
        y[i] = y_min + (y_max - y_min) * i / (grid_size - 1);
        for (unsigned int j = 0; j < grid_size; ++j) {
            z[i][j] = std::sin(x[i]) * std::cos(y[j]);
        }
    }

    // Apply contour settings: Contours on both surface and base
    gnuplot
        .set_title("Contour Plot of sin(x) * cos(y)") // Set plot title
        .set_xlabel("x-axis")                         // Set x-axis label
        .set_ylabel("y-axis")                         // Set y-axis label
        .set_contour_type(contour_type_t::both)       // Apply contours on both surface and base
        .plot_3d_grid(x, y, z)                        // Plot the 3D grid data
        .show();

    return 0;
}
