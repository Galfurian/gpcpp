/// @file example_3d_surface_plot.cpp
/// @brief An example demonstrating how to plot a 3D surface using Gnuplot.
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

    // Plot the 3D surface with the generated data
    gnuplot
        .set_title("3D Surface Plot of sin(x) * cos(y)") // Set plot title
        .set_xlabel("x-axis")                            // Set x-axis label
        .set_ylabel("y-axis")                            // Set y-axis label
        .set_zlabel("z-axis")                            // Set z-axis label
        .set_surface()                                   // Enable surface plot
        .plot_3d_grid(x, y, z)                           // Plot the 3D grid data
        .show();

    return 0;
}
