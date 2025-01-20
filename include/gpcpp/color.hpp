/// @file color.hpp
/// @brief Add standardized color support.

#pragma once

#include <string>
#include <sstream>
#include <iomanip>
#include <stdexcept>

namespace gpcpp
{

/// @brief A struct that represents a color in RGB format and ensures valid color formatting.
struct Color {
    int r, g, b; // RGB components (0-255)

    // Default constructor initializes as "unset" state.
    Color() : r(-1), g(-1), b(-1)
    {
    }

    // Constructor: initializes the color with valid RGB values.
    Color(int r, int g, int b)
    {
        set_from_rgb(r, g, b);
    }

    // Constructor from a string (hex format #RRGGBB or color name)
    Color(const std::string &color_str)
    {
        if (!color_str.empty() && color_str[0] == '#') {
            // If it's in hex format #RRGGBB
            set_from_hex(color_str);
        } else {
            // If it's a named color, fall back to predefined colors.
            set_from_name(color_str);
        }
    }
    
    void unset()
    {
         r = -1, g = -1, b = -1;
    }

    bool is_set() const
    {
        return r != -1 && g != -1 && b != -1;
    }

    /// @brief Set color components using RGB values.
    void set_from_rgb(int r, int g, int b)
    {
        this->r = (r >= 0 && r <= 255) ? r : 0;
        this->g = (g >= 0 && g <= 255) ? g : 0;
        this->b = (b >= 0 && b <= 255) ? b : 0;
    }

    /// @brief Set color from a hex code string.
    void set_from_hex(const std::string &hex)
    {
        if (hex.size() == 7 && hex[0] == '#') {
            r = std::stoi(hex.substr(1, 2), nullptr, 16);
            g = std::stoi(hex.substr(3, 2), nullptr, 16);
            b = std::stoi(hex.substr(5, 2), nullptr, 16);
        }
    }

    /// @brief Set color from a predefined name.
    void set_from_name(const std::string &name)
    {
        // Predefined color names (you can expand this list)
        if (name == "red") {
            set_from_rgb(255, 0, 0);
        } else if (name == "green") {
            set_from_rgb(0, 255, 0);
        } else if (name == "blue") {
            set_from_rgb(0, 0, 255);
        } else if (name == "yellow") {
            set_from_rgb(255, 255, 0);
        } else if (name == "cyan") {
            set_from_rgb(0, 255, 255);
        } else if (name == "magenta") {
            set_from_rgb(255, 0, 255);
        } else if (name == "black") {
            set_from_rgb(0, 0, 0);
        } else if (name == "white") {
            set_from_rgb(255, 255, 255);
        } else if (name == "gray") {
            set_from_rgb(128, 128, 128);
        } else {
            r = g = b = -1; // Unset state if unknown color name
        }
    }

    // Getter for RGB values
    std::string to_string() const
    {
        if (!is_set())
            return ""; // Return empty string if color is unset
        return "#" + format_hex(r) + format_hex(g) + format_hex(b);
    }

private:
    static inline std::string format_hex(int value)
    {
        char buf[3];
        snprintf(buf, sizeof(buf), "%02x", value);
        return std::string(buf);
    }
};
} // namespace gpcpp
