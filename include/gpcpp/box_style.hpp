/// @file box_style.hpp
/// @brief Stores the style for a textbox.

#pragma once

#include "gpcpp/color.hpp"

namespace gpcpp
{

/// @brief Struct to represent the box style for a label in Gnuplot.
struct box_style_t {
    bool show;          ///< Whether to show the box (otherwise, it uses nobox option).
    bool opaque;        ///< Whether the box is opaque (true) or transparent (false).
    Color fill_color;   ///< The fill color of the box.
    bool border;        ///< Whether the box has a border.
    Color border_color; ///< The color of the border.
    double line_width;  ///< Line width of the border.
    double x_margin;    ///< Horizontal margin inside the box.
    double y_margin;    ///< Vertical margin inside the box.

    /// @brief Constructor to initialize the box style.
    /// @param show Whether to show the box (otherwise, it uses nobox option).
    /// @param opaque Whether the box is opaque.
    /// @param fill_color The fill color of the box.
    /// @param border Whether the box has a border.
    /// @param border_color The color of the border.
    /// @param line_width The line width of the border.
    /// @param x_margin The horizontal margin inside the box.
    /// @param y_margin The vertical margin inside the box.
    box_style_t(bool show                       = false,
                bool opaque                     = false,
                const std::string &fill_color   = "white",
                bool border                     = false,
                const std::string &border_color = "black",
                double line_width               = 1.0,
                double x_margin                 = 0.0,
                double y_margin                 = 0.0)
        : show(show), opaque(opaque), fill_color(fill_color), border(border), border_color(border_color),
          line_width(line_width), x_margin(x_margin), y_margin(y_margin)
    {
        // Nothing to do.
    }

    /// @brief Convert the box style to a string for use in Gnuplot.
    std::string get_declaration(int id) const
    {
        std::ostringstream oss;
        if (show) {
            oss << "set style textbox " << id << " ";
            oss << (opaque ? "opaque" : "transparent") << " fillcolor \"" << fill_color.to_string() << "\"";
            if (border) {
                oss << " border lc \"" << border_color.to_string() << "\" lw " << line_width;
            }
            oss << " margins " << x_margin << "," << y_margin;
        }
        return oss.str();
    }
};

} // namespace gpcpp
