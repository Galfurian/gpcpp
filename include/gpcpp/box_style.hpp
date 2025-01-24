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
    /// @param _show Whether to show the box (otherwise, it uses nobox option).
    /// @param _opaque Whether the box is opaque.
    /// @param _fill_color The fill color of the box.
    /// @param _border Whether the box has a border.
    /// @param _border_color The color of the border.
    /// @param _line_width The line width of the border.
    /// @param _x_margin The horizontal margin inside the box.
    /// @param _y_margin The vertical margin inside the box.
    box_style_t(bool _show                       = false,
                bool _opaque                     = false,
                const std::string &_fill_color   = "white",
                bool _border                     = false,
                const std::string &_border_color = "black",
                double _line_width               = 1.0,
                double _x_margin                 = 0.0,
                double _y_margin                 = 0.0)
        : show(_show), opaque(_opaque), fill_color(_fill_color), border(_border), border_color(_border_color),
          line_width(_line_width), x_margin(_x_margin), y_margin(_y_margin)
    {
        // Nothing to do.
    }

    /// @brief Convert the box style to a string for use in Gnuplot.
    /// @param id The id of the box.
    /// @return the string representation of the box style.
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
