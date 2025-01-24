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
    /// @brief Default constructor initializes as "unset" state.
    /// @details This constructor sets the RGB values to -1, indicating an unset state.
    Color() : r(-1), g(-1), b(-1), a(255)
    {
    }

    /// @brief Constructor: initializes the color with valid RGB values.
    /// @param _r The red component of the color (0-255).
    /// @param _g The green component of the color (0-255).
    /// @param _b The blue component of the color (0-255).
    /// @param _a The alpha component (0 = transparent, 255 = opaque).
    /// @details This constructor sets the RGB values to the provided parameters, ensuring they are valid (0-255).
    Color(int _r, int _g, int _b, int _a = 255) : r(-1), g(-1), b(-1), a(255)
    {
        set_from_rgba(_r, _g, _b, _a);
    }

    /// @brief Constructor from a string (hex format \#RRGGBB or color name).
    /// @param color_str A string representing the color, either in hex format (e.g., "\#ff0000") or as a predefined color name (e.g., "red").
    /// @details This constructor interprets the string as either a hex color code or a predefined color name and sets the RGB values accordingly.
    Color(const std::string &color_str) : r(-1), g(-1), b(-1), a(255)
    {
        if (!color_str.empty()) {
            if (color_str[0] == '#') {
                // If it's in hex format #RRGGBB
                set_from_hex(color_str);
            } else {
                // If it's a named color, fall back to predefined colors.
                set_from_name(color_str);
            }
        }
    }

    /// @brief Resets the color to an unset state.
    /// @details This function sets all RGB values to -1, marking the color as unset.
    void unset()
    {
        r = -1, g = -1, b = -1, a = 255;
    }

    /// @brief Checks if the color is set.
    /// @return Returns true if the color is set (i.e., RGB values are not -1), otherwise false.
    bool is_set() const
    {
        return r != -1 && g != -1 && b != -1;
    }

    /// @brief Sets the color components using RGB values.
    /// @param _r The red component of the color (0-255).
    /// @param _g The green component of the color (0-255).
    /// @param _b The blue component of the color (0-255).
    /// @param _a The alpha component (0-255, default: 255).
    /// @details This function ensures that each RGB component is within the valid range (0-255).
    /// If any component is out of range, it is clamped to the nearest valid value.
    void set_from_rgba(int _r, int _g, int _b, int _a = 255)
    {
        r = (_r >= 0 && _r <= 255) ? _r : 0;
        g = (_g >= 0 && _g <= 255) ? _g : 0;
        b = (_b >= 0 && _b <= 255) ? _b : 0;
        a = (_a >= 0 && _a <= 255) ? _a : 0;
    }

    /// @brief Sets the color from a hex code string.
    /// @param hex A string representing the hex code (e.g., "#ff0000").
    /// @details This function extracts the RGB components from the provided hex code and sets the color.
    void set_from_hex(const std::string &hex)
    {
        if (hex.size() == 7 && hex[0] == '#') {                           // #RRGGBB
            this->set_from_rgba(std::stoi(hex.substr(1, 2), nullptr, 16), // Red
                                std::stoi(hex.substr(3, 2), nullptr, 16), // Green
                                std::stoi(hex.substr(5, 2), nullptr, 16), // Blue
                                255                                       // Alpha (fully opaque by default)
            );
        } else if (hex.size() == 9 && hex[0] == '#') {                    // #AARRGGBB
            this->set_from_rgba(std::stoi(hex.substr(3, 2), nullptr, 16), // Red
                                std::stoi(hex.substr(5, 2), nullptr, 16), // Green
                                std::stoi(hex.substr(7, 2), nullptr, 16), // Blue
                                std::stoi(hex.substr(1, 2), nullptr, 16)  // Alpha
            );
        }
    }

    /// @brief Sets the color from a predefined name.
    /// @param name A string representing a predefined color name (e.g., "red", "green").
    /// @details This function matches the provided name with predefined colors and sets the corresponding RGB values.
    /// If the name is unrecognized, the color is set to an unset state.
    void set_from_name(const std::string &name)
    {
        if (name == "red") {
            this->set_from_rgba(255, 0, 0, 255);
        } else if (name == "green") {
            this->set_from_rgba(0, 255, 0, 255);
        } else if (name == "blue") {
            this->set_from_rgba(0, 0, 255, 255);
        } else if (name == "yellow") {
            this->set_from_rgba(255, 255, 0, 255);
        } else if (name == "cyan") {
            this->set_from_rgba(0, 255, 255, 255);
        } else if (name == "magenta") {
            this->set_from_rgba(255, 0, 255, 255);
        } else if (name == "black") {
            this->set_from_rgba(0, 0, 0, 255);
        } else if (name == "white") {
            this->set_from_rgba(255, 255, 255, 255);
        } else if (name == "gray") {
            this->set_from_rgba(128, 128, 128, 255);
        } else {
            unset();
        }
    }

    /// @brief Returns the color as a hex string.
    /// @return A string representing the color in hex format (e.g., "#ff0000").
    /// @details If the color is unset, an empty string is returned.
    std::string to_string() const
    {
        if (!is_set())
            return ""; // Return empty string if color is unset

        char buf[11];
        snprintf(buf, sizeof(buf), "#%02X%02X%02X%02X", (255 - a), r, g, b);
        return std::string(buf);
    }

private:
    int r; ///< Color red.
    int g; ///< Color green.
    int b; ///< Color blue.
    int a; ///< Alpha component (0 = fully transparent, 255 = fully opaque for the user).
};

} // namespace gpcpp
