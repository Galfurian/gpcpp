// Example for C++ Interface to Gnuplot

// requirements:
// * gnuplot has to be installed (http://www.gnuplot.info/download.html)
// * for Windows: set Path-Variable for Gnuplot path (e.g. C:/program files/gnuplot/bin)
//             or set Gnuplot path with: Gnuplot::set_GNUPlotPath(const std::string &path);

#include <iostream>
#include <array>
#include <cmath>

#include <gnuplotcpp/gnuplot.hpp>

static inline void wait_for_key()
{
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) ||                                                         \
    defined(__TOS_WIN__) // every keypress registered, also arrow keys
    std::cout << "Press any key to continue...\n";
    FlushConsoleInputBuffer(GetStdHandle(STD_INPUT_HANDLE));
    _getch();
#elif defined(unix) || defined(__unix) || defined(__unix__) || defined(__APPLE__)
    std::cout << "Press ENTER to continue...\n";
    std::cin.clear();
    std::cin.ignore(std::cin.rdbuf()->in_avail());
    std::cin.get();
#endif
}

int main(int, char *[])
{
    using namespace gnuplotcpp;

#if 0
    // Initialize gnuplot.
    Gnuplot gnuplot;
    // Plot the data with a title.
    gnuplot.set_line_color("red").plot_slope(1, 2.5, "sine");
    // Pause for the user to view.
    wait_for_key();
#endif

#if 0
    std::vector<double> x, y, dy;
    for (unsigned int i = 0; i < 50; i++) {
        x.push_back((double)i);                           // x[i] = i
        y.push_back((double)i * (double)i);               // y[i] = i^2
        dy.push_back((double)i * (double)i / (double)10); // dy[i] = i^2 / 10
    }
    // Initialize gnuplot.
    Gnuplot gnuplot;
    // Plot the data with a title.
    gnuplot.set_plot_style(plot_style_t::lines)
        .set_line_width(2.0)
        .plot_xy(x, y)
        .set_point_style(point_style_t::filled_square)
        .set_line_style(line_style_t::dashed)
        .set_line_color("blue")
        .plot_xy_erorrbar(x, y, dy, erorrbar_style_t::yerrorbars, "x^2");
    // Pause for the user to view.
    wait_for_key();
#endif

#if 1
    // Initialize gnuplot.
    Gnuplot gnuplot;
    gnuplot.set_terminal(gnuplotcpp::terminal_type_t::pdfcairo);
    std::vector<double> x, y;
    for (unsigned int i = 0; i < 50; i++) {
        x.push_back((double)i);             // x[i] = i
        y.push_back((double)i * (double)i); // y[i] = i^2
    }
    // Plot the data with a title.
    gnuplot.set_grid()
        .set_legend("top left", "Courier,12")
        .set_plot_style(plot_style_t::steps)
        .set_line_width(2.0)
        .plot_xy(x, y, "x^2");

    // Pause for the user to view.
    wait_for_key();
#endif

    // {
    //     // Initialize Gnuplot
    //     Gnuplot gnuplot;

    //     // Set contour type to both base and surface
    //     gnuplot.set_contour_type(contour_type_t::both);

    //     // Apply the settings
    //     gnuplot.apply_contour_settings();

    //     // Data to plot
    //     std::vector<double> x1_data = { 1.5, 1.0, 0.5, 4.0, 2.0 };
    //     std::vector<double> x2_data = { .5, .25, 0.15, 6.0, 1.05 };
    //     // Plot the data with a title
    //     gnuplot.set_plot_style(plot_style_t::lines)
    //         .set_line_width(4.0)
    //         .set_xlabel("X Axis")
    //         .set_ylabel("Y Axis")
    //         .plot_x(x1_data, "X1")
    //         .plot_x(x1_data, "X2");

    //     // Pause for the user to view
    //     wait_for_key();
    // }
    // {
    //     // Initialize gnuplot.
    //     Gnuplot gnuplot;
    //     // Plot the data with a title.
    //     gnuplot.set_plot_style(plot_style_t::impulses)
    //         .set_line_width(2.5)
    //         .set_point_size(2.0)
    //         .set_line_color("blue")
    //         .plot_equation("sin(x)", "sine");
    //     // Pause for the user to view.
    //     wait_for_key();
    // }
    // {
    //     // Initialize gnuplot.
    //     Gnuplot gp;
    //     // Plot.
    //     gp.set_line_color("red").set_line_width(2.0).plot_equation3d("sin(x)*cos(y)", "3D Sine-Cosine");
    //     // Pause for the user to view.
    //     wait_for_key();
    // }
#if 0
        Gnuplot gnuplot;

        const std::size_t grid_size = 50U;

        // Define ranges for x and y
        std::array<double, grid_size> x, y;
        std::array<std::array<double, grid_size>, grid_size> z;

        double x_min = -10.0, x_max = 10.0;
        double y_min = -10.0, y_max = 10.0;

        // Generate grid data for x, y, and z = sin(x) * cos(y)
        for (std::size_t i = 0; i < grid_size; ++i) {
            x[i] = x_min + static_cast<double>(i) * (x_max - x_min) / (grid_size - 1);
            y[i] = y_min + static_cast<double>(i) * (y_max - y_min) / (grid_size - 1);
        }
        for (std::size_t i = 0; i < grid_size; ++i) {
            for (std::size_t j = 0; j < grid_size; ++j) {
                z[i][j] = std::sin(x[i]) * std::cos(y[j]);
            }
        }

        // Contours on the surface
        gnuplot.set_contour_type(contour_type_t::surface)
            .set_title("Contours on Surface")
            .plot_3d_grid(x, y, z)
            .apply_contour_settings();

        // Pause for the user to view
        wait_for_key();

        // Contours on the base plane
        gnuplot.set_contour_type(contour_type_t::base)
            .set_contour_param(contour_param_t::levels)
            .set_contour_levels(10)
            .set_title("Contours on Base Plane")
            .plot_3d_grid(x, y, z)
            .apply_contour_settings();

        // Pause for the user to view
        wait_for_key();

        // Plot 3: Contours on both base and surface with discrete levels
        gnuplot.set_contour_type(contour_type_t::both)
            .set_contour_param(contour_param_t::discrete)
            .set_contour_discrete_levels({ -0.5, 0.0, 0.5 })
            .set_title("Contours on Base and Surface")
            .plot_3d_grid(x, y, z)
            .apply_contour_settings();

        // Pause for the user to view
        wait_for_key();
#endif

#if 0
    std::cout << "*** example of gnuplot control through C++ ***\n";

    try {
        gnuplotcpp::Gnuplot g1("lines");

        //
        // Slopes
        //
        std::cout << "*** plotting slopes ***\n";
        g1.set_title("Slopes\\nNew Line");

        std::cout << "y = x" << std::endl;
        g1.plot_slope(1.0, 0.0, "y=x");

        std::cout << "y = 2*x" << std::endl;
        g1.plot_slope(2.0, 0.0, "y=2x");

        std::cout << "y = -x" << std::endl;
        g1.plot_slope(-1.0, 0.0, "y=-x");
        g1.unset_title();

        //
        // Equations
        //
        g1.reset_plot();
        std::cout << std::endl << std::endl << "*** various equations" << std::endl;

        std::cout << "y = sin(x)" << std::endl;
        g1.plot_equation("sin(x)", "sine");

        std::cout << "y = log(x)" << std::endl;
        g1.plot_equation("log(x)", "logarithm");

        std::cout << "y = sin(x) * cos(2*x)" << std::endl;
        g1.plot_equation("sin(x)*cos(2*x)", "sine product");

        //
        // Styles
        //
        g1.reset_plot();
        std::cout << std::endl << std::endl << "*** showing styles" << std::endl;

        std::cout << "sine in points" << std::endl;
        g1.set_pointsize(0.8).set_plot_style("points");
        g1.plot_equation("sin(x)", "points");

        std::cout << "sine in impulses" << std::endl;
        g1.set_plot_style("impulses");
        g1.plot_equation("sin(x)", "impulses");

        std::cout << "sine in steps" << std::endl;
        g1.set_plot_style("steps");
        g1.plot_equation("sin(x)", "steps");

        //
        // Save to ps
        //
        g1.reset_all();
        std::cout << std::endl << std::endl << "*** save to ps " << std::endl;

        std::cout << "y = sin(x) saved to test_output.ps in working directory" << std::endl;
        //      g1.savetops("test_output");
        g1.savetofigure("test_output.ps", "postscript color");
        g1.set_plot_style("lines").set_samples(300).set_xrange(0, 5);
        g1.plot_equation("sin(12*x)*exp(-x)").plot_equation("exp(-x)");

        g1.showonscreen(); // window output

        //
        // User defined 1d, 2d and 3d point sets
        //
        std::vector<double> x, y, y2, dy, z;

        for (unsigned int i = 0; i < 50; i++) // fill double arrays x, y, z
        {
            x.push_back((double)i);                           // x[i] = i
            y.push_back((double)i * (double)i);               // y[i] = i^2
            z.push_back(x[i] * y[i]);                         // z[i] = x[i]*y[i] = i^3
            dy.push_back((double)i * (double)i / (double)10); // dy[i] = i^2 / 10
        }
        y2.push_back(0.00);
        y2.push_back(0.78);
        y2.push_back(0.97);
        y2.push_back(0.43);
        y2.push_back(-0.44);
        y2.push_back(-0.98);
        y2.push_back(-0.77);
        y2.push_back(0.02);

        g1.reset_all();
        std::cout << std::endl << std::endl << "*** user-defined lists of doubles" << std::endl;
        g1.set_plot_style("impulses").plot_x(y, "user-defined doubles");

        g1.reset_plot();
        std::cout << std::endl << std::endl << "*** user-defined lists of points (x,y)" << std::endl;
        g1.set_grid();
        g1.set_plot_style("points").plot_xy(x, y, "user-defined points 2d");

        g1.reset_plot();
        std::cout << std::endl << std::endl << "*** user-defined lists of points (x,y,z)" << std::endl;
        g1.unset_grid();
        g1.plot_xyz(x, y, z, "user-defined points 3d");

        g1.reset_plot();
        std::cout << std::endl << std::endl << "*** user-defined lists of points (x,y,dy)" << std::endl;
        g1.plot_xy_err(x, y, dy, "user-defined points 2d with errorbars");

        //
        // Multiple output screens
        //
        std::cout << std::endl << std::endl;
        std::cout << "*** multiple output windows" << std::endl;

        g1.reset_plot();
        g1.set_plot_style("lines");
        std::cout << "window 1: sin(x)" << std::endl;
        g1.set_grid().set_samples(600).set_xrange(0, 300);
        g1.plot_equation("sin(x)+sin(x*1.1)");

        g1.set_xautoscale().replot();

        gnuplotcpp::Gnuplot g2;
        std::cout << "window 2: user defined points" << std::endl;
        g2.plot_x(y2, "points");
        g2.set_smooth_style().plot_x(y2, "cspline");
        g2.set_smooth_style("bezier").plot_x(y2, "bezier");
        g2.unset_smooth();

        gnuplotcpp::Gnuplot g3("lines");
        std::cout << "window 3: log(x)/x" << std::endl;
        g3.set_grid();
        g3.plot_equation("log(x)/x", "log(x)/x");

        gnuplotcpp::Gnuplot g4("lines");
        std::cout << "window 4: splot x*x+y*y" << std::endl;
        g4.set_zrange(0, 100);
        g4.set_xlabel("x-axis").set_ylabel("y-axis").set_zlabel("z-axis");
        g4.plot_equation3d("x*x+y*y");

        gnuplotcpp::Gnuplot g5("lines");
        std::cout << "window 5: splot with hidden3d" << std::endl;
        g5.set_isosamples(25).set_hidden3d();
        g5.plot_equation3d("x*y*y");

        gnuplotcpp::Gnuplot g6("lines");
        std::cout << "window 6: splot with contour" << std::endl;
        g6.set_isosamples(60).set_contour();
        g6.unset_surface().plot_equation3d("sin(x)*sin(y)+4");

        g6.set_surface().replot();

        gnuplotcpp::Gnuplot g7("lines");
        std::cout << "window 7: set_samples" << std::endl;
        g7.set_xrange(-30, 20).set_samples(40);
        g7.plot_equation("besj0(x)*0.12e1").plot_equation("(x**besj0(x))-2.5");

        g7.set_samples(400).replot();

        gnuplotcpp::Gnuplot g8("filledcurves");
        std::cout << "window 8: filledcurves" << std::endl;
        g8.set_legend("outside right top").set_xrange(-5, 5);
        g8.plot_equation("x*x").plot_equation("-x*x+4");

        //
        // Plot an image
        //
        gnuplotcpp::Gnuplot g9;
        std::cout << "window 9: plot_image" << std::endl;
        const int unsigned uiWidth  = 255U;
        const int unsigned uiHeight = 255U;
        g9.set_xrange(0, uiWidth).set_yrange(0, uiHeight).set_cbrange(0, 255);
        g9.send_cmd("set palette gray");
        unsigned char ucPicBuf[uiWidth * uiHeight];
        // generate a greyscale image
        for (unsigned int uiIndex = 0; uiIndex < uiHeight * uiWidth; uiIndex++) {
            ucPicBuf[uiIndex] = static_cast<unsigned char>(uiIndex % 255U);
        }
        g9.plot_image(ucPicBuf, uiWidth, uiHeight, "greyscale");

        g9.set_pointsize(0.6).unset_legend().plot_slope(0.8, 20);

        //
        // manual control
        //
        gnuplotcpp::Gnuplot g10;
        std::cout << "window 10: manual control" << std::endl;
        g10.send_cmd("set samples 400").send_cmd("plot abs(x)/2"); // either with cmd()
        g10 << "replot sqrt(x)" << "replot sqrt(-x)";    // or with <<

        wait_for_key();
    } catch (const gnuplotcpp::GnuplotException &e) {
        std::cout << e.what() << std::endl;
    }

    std::cout << std::endl << "*** end of gnuplot example" << std::endl;
#endif

    return 0;
}
