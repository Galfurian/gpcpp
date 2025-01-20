# GPCpp

[![Ubuntu](https://github.com/Galfurian/gpcpp/actions/workflows/ubuntu.yml/badge.svg)](https://github.com/Galfurian/gpcpp/actions/workflows/ubuntu.yml)
[![Windows](https://github.com/Galfurian/gpcpp/actions/workflows/windows.yml/badge.svg)](https://github.com/Galfurian/gpcpp/actions/workflows/windows.yml)
[![MacOS](https://github.com/Galfurian/gpcpp/actions/workflows/macos.yml/badge.svg)](https://github.com/Galfurian/gpcpp/actions/workflows/macos.yml)
[![Documentation](https://github.com/Galfurian/gpcpp/actions/workflows/documentation.yml/badge.svg)](https://github.com/Galfurian/gpcpp/actions/workflows/documentation.yml)

GPCpp is a C++ interface to the powerful plotting utility **Gnuplot**. It allows seamless interaction with Gnuplot for generating and customizing a variety of plots, including line plots, scatter plots, 3D surface plots, error bars, logarithmic scaling, and more.

## Features

- **Cross-platform Support**: Works on Windows, Linux, and macOS (via POSIX-compliant pipes).
- **Custom Plot Styles**: Supports various plot styles such as points, lines, filled curves, and more.
- **Logarithmic and Linear Scales**: Easily set logarithmic scales for axes.
- **Flexible Plotting**: Support for 2D and 3D plotting, including multi-plot setups.
- **Customizable Colors**: Easily set line colors using named colors or RGB hex values.
- **Error Bar Support**: Add error bars to your plots.
- **Smoothing Options**: Multiple smoothing options for data visualization.
- **File Output**: Save plots to various formats (e.g., PNG, PDF).

## Installation

### Prerequisites

- Gnuplot must be installed on your system.
- A C++ compiler supporting C++11 or later.
- CMake for building the project.

### Building the Library

Clone the repository:

```bash
git clone https://github.com/yourusername/gpcpp.git
cd gpcpp
```

Create a build directory and navigate into it:

```bash
mkdir build
cd build
```

Run CMake to configure the build:

```bash
cmake ..
```

Build the library:

```bash
make
```

### Linking the Library

To link GPCpp to your project, include the following in your CMakeLists.txt:

```cmake
target_link_libraries(your_project gpcpp)
```

## Usage

Here is a simple example demonstrating how to plot data using GPCpp:

```cpp
#include <iostream>
#include <vector>
#include <cmath>
#include <gpcpp/gnuplot.hpp>

int main() {
    using namespace gpcpp;

    // Create a Gnuplot instance
    Gnuplot gnuplot;

    // Prepare data for plotting
    std::vector<double> x, y;
    for (unsigned int i = 0; i < 50; i++) {
        x.push_back(static_cast<double>(i));      // x[i] = i
        y.push_back(std::pow(x[i], 2));           // y[i] = x[i]^2 (parabola)
    }

    // Plot the data as a line
    gnuplot.set_title("Simple Plot Example")
        .set_xlabel("X Axis")
        .set_ylabel("Y = X^2")
        .plot_xy(x, y);  // Plot x vs y as a line (parabola)

    return 0;
}
```

## License

This library is released under the MIT License. See LICENSE for details.

## Contributing

Fork this repository.
Create a feature branch.
Commit your changes.
Push to your fork.
Open a pull request.
Feel free to submit issues and pull requests to enhance the functionality of the library.
