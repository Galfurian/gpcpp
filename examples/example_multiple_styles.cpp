/// @file example_multiple_styles.cpp
/// @brief An example showing how to combine multiple plotting styles (e.g., points, lines, filled curves) in a single plot.
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
    std::vector<double> x, y, z, w;
    for (unsigned int i = 0; i < 50; i++) {
        x.push_back(static_cast<double>(i)); // x[i] = i
        y.push_back(x[i] * x[i]);            // y[i] = x[i]^2 (parabola)
        z.push_back(x[i] * x[i] * 1.5);      // z[i] = x[i]^2 * 1.5 (scaled parabola)
        w.push_back(x[i] * x[i] * 2);        // w[i] = x[i]^2 * 2 (scaled parabola)
    }

    // Plot multiple styles: Lines
    gnuplot
        .set_title("Multiple Lines: y = x^2, z = 1.5 * x^2, w = 2 * x^2")

        // Plot y = x^2 as a line
        .set_plot_type(plot_type_t::boxes)
        .set_line_color("#ff5733")
        .plot_xy(x, y, "y = x^2")

        // Plot z = 1.5 * x^2 as a line
        .set_plot_type(plot_type_t::lines)
        .set_line_type(line_type_t::dashed)
        .set_line_color("red")
        .set_line_width(2.0)
        .plot_xy(x, z, "z = 1.5 * x^2")

        // Plot w = 2 * x^2 as a line
        .set_plot_type(plot_type_t::lines)
        .set_line_color("#228B22")
        .set_line_width(2.0)
        .plot_xy(x, w, "w = 2 * x^2")

        .show();

    return 0;
}
