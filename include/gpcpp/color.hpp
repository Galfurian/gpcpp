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
    int r; ///< Color red.
    int g; ///< Color green.
    int b; ///< Color blue.

    /// @brief Default constructor initializes as "unset" state.
    /// @details This constructor sets the RGB values to -1, indicating an unset state.
    Color() : r(-1), g(-1), b(-1)
    {
    }

    /// @brief Constructor: initializes the color with valid RGB values.
    /// @param r The red component of the color (0-255).
    /// @param g The green component of the color (0-255).
    /// @param b The blue component of the color (0-255).
    /// @details This constructor sets the RGB values to the provided parameters, ensuring they are valid (0-255).
    Color(int r, int g, int b)
    {
        set_from_rgb(r, g, b);
    }

    /// @brief Constructor from a string (hex format \#RRGGBB or color name).
    /// @param color_str A string representing the color, either in hex format (e.g., "\#ff0000") or as a predefined color name (e.g., "red").
    /// @details This constructor interprets the string as either a hex color code or a predefined color name and sets the RGB values accordingly.
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

    /// @brief Resets the color to an unset state.
    /// @details This function sets all RGB values to -1, marking the color as unset.
    void unset()
    {
        r = -1, g = -1, b = -1;
    }

    /// @brief Checks if the color is set.
    /// @return Returns true if the color is set (i.e., RGB values are not -1), otherwise false.
    bool is_set() const
    {
        return r != -1 && g != -1 && b != -1;
    }

    /// @brief Sets the color components using RGB values.
    /// @param r The red component of the color (0-255).
    /// @param g The green component of the color (0-255).
    /// @param b The blue component of the color (0-255).
    /// @details This function ensures that each RGB component is within the valid range (0-255).
    /// If any component is out of range, it is clamped to the nearest valid value.
    void set_from_rgb(int r, int g, int b)
    {
        this->r = (r >= 0 && r <= 255) ? r : 0;
        this->g = (g >= 0 && g <= 255) ? g : 0;
        this->b = (b >= 0 && b <= 255) ? b : 0;
    }

    /// @brief Sets the color from a hex code string.
    /// @param hex A string representing the hex code (e.g., "#ff0000").
    /// @details This function extracts the RGB components from the provided hex code and sets the color.
    void set_from_hex(const std::string &hex)
    {
        if (hex.size() == 7 && hex[0] == '#') {
            r = std::stoi(hex.substr(1, 2), nullptr, 16);
            g = std::stoi(hex.substr(3, 2), nullptr, 16);
            b = std::stoi(hex.substr(5, 2), nullptr, 16);
        }
    }

    /// @brief Sets the color from a predefined name.
    /// @param name A string representing a predefined color name (e.g., "red", "green").
    /// @details This function matches the provided name with predefined colors and sets the corresponding RGB values.
    /// If the name is unrecognized, the color is set to an unset state.
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

    /// @brief Returns the color as a hex string.
    /// @return A string representing the color in hex format (e.g., "#ff0000").
    /// @details If the color is unset, an empty string is returned.
    std::string to_string() const
    {
        if (!is_set())
            return ""; // Return empty string if color is unset
        return "#" + format_hex(r) + format_hex(g) + format_hex(b);
    }

private:
    /// @brief Formats an integer as a two-character hex string.
    /// @param value The integer to format (0-255).
    /// @return A string representing the integer as a two-character hex string.
    static inline std::string format_hex(int value)
    {
        char buf[3];
        snprintf(buf, sizeof(buf), "%02x", value);
        return std::string(buf);
    }
};

} // namespace gpcpp
