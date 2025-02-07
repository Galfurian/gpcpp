/// @file defines.hpp
/// @brief List of gnuplot defines.

#pragma once

#include <string>

namespace gpcpp
{

/// @brief Enum representing the various plotting styles available in Gnuplot.
enum class plot_type_t : unsigned char {
    none,          ///< Default fallback style (points).
    lines,         ///< Lines connecting the data points.
    points,        ///< Individual data points.
    lines_points,  ///< Lines connecting data points with points marked.
    impulses,      ///< Vertical lines from the x-axis to the data points.
    dots,          ///< Small dots for data points.
    steps,         ///< Stepwise connection of data points.
    fsteps,        ///< Finite steps between data points.
    histeps,       ///< Histogram-like steps between data points.
    boxes,         ///< Boxes for histogram-like data.
    filled_curves, ///< Filled areas under curves.
    histograms,    ///< Histograms.
};

/// @brief Converts a plot_type_t value to its corresponding Gnuplot string representation.
/// @param style The plotting style as a plot_type_t enum value.
/// @return A string representing the corresponding Gnuplot style.
static inline auto plot_type_to_string(plot_type_t style) -> std::string
{
    switch (style) {
    case plot_type_t::lines:
        return "lines";
    case plot_type_t::points:
        return "points";
    case plot_type_t::lines_points:
        return "linespoints";
    case plot_type_t::impulses:
        return "impulses";
    case plot_type_t::dots:
        return "dots";
    case plot_type_t::steps:
        return "steps";
    case plot_type_t::fsteps:
        return "fsteps";
    case plot_type_t::histeps:
        return "histeps";
    case plot_type_t::boxes:
        return "boxes";
    case plot_type_t::filled_curves:
        return "filledcurves";
    case plot_type_t::histograms:
        return "histograms";
    default:
        return "lines";
    }
}

/// @brief The style of error bars.
enum class erorrbar_type_t : unsigned char {
    yerrorbars, ///< Error bars along the y-axis.
    xerrorbars, ///< Error bars along the x-axis.
};

/// @brief Converts an erorrbar_type_t value to a Gnuplot-compatible string.
/// @param style The error bar style enumeration.
/// @return Gnuplot-compatible string for the error bar style.
static inline auto errorbars_to_string(erorrbar_type_t style = erorrbar_type_t::yerrorbars) -> std::string
{
    switch (style) {
    case erorrbar_type_t::yerrorbars:
        return "yerrorbars";
    case erorrbar_type_t::xerrorbars:
        return "xerrorbars";
    default:
        return "yerrorbars";
    }
}

/// @brief Enum representing the smoothing styles available in Gnuplot.
enum class smooth_type_t : unsigned char {
    none,      ///< No smoothing (default).
    unique,    ///< Unique smoothing.
    frequency, ///< Frequency-based smoothing.
    csplines,  ///< Cubic spline interpolation.
    acsplines, ///< Approximation cubic splines.
    bezier,    ///< Bezier curve smoothing.
    sbezier,   ///< Subdivided Bezier smoothing.
};

/// Converts a smooth_type value to its corresponding Gnuplot string.
/// @param style The smoothing style to convert.
/// @return A string representing the Gnuplot smoothing style.
static inline auto smooth_type_to_string(smooth_type_t style) -> std::string
{
    switch (style) {
    case smooth_type_t::unique:
        return "unique";
    case smooth_type_t::frequency:
        return "frequency";
    case smooth_type_t::csplines:
        return "csplines";
    case smooth_type_t::acsplines:
        return "acsplines";
    case smooth_type_t::bezier:
        return "bezier";
    case smooth_type_t::sbezier:
        return "sbezier";
    default:
        return std::string(); // Default: No smoothing
    }
}

/// @brief Contour type options for Gnuplot
enum class contour_type_t : unsigned char {
    none,    ///< Disables contouring
    base,    ///< Contours on the base (XY-plane)
    surface, ///< Contours on the surface
    both,    ///< Contours on both base and surface
};

/// @brief Contour parameter options for Gnuplot
enum class contour_param_t : unsigned char {
    levels,    ///< Number of contour levels
    increment, ///< Contour increment settings
    discrete,  ///< Specific discrete contour levels
};

/// @brief Enumeration to represent different grid types in Gnuplot.
enum class grid_type_t : unsigned char {
    major, ///< Major grid (default).
    minor, ///< Minor grid.
};

/// @brief Enumeration for Gnuplot line styles.
enum class line_type_t : unsigned char {
    none,         ///< No line style set.
    solid,        ///< Solid line (default)
    dashed,       ///< Dashed line
    dotted,       ///< Dotted line
    dash_dot,     ///< Dash-dot pattern
    dash_dot_dot, ///< Dash-dot-dot pattern
    custom        ///< Custom dash pattern
};

/// @brief Converts a line_type_t value to a Gnuplot-compatible string.
/// @param style The line style enumeration.
/// @param custom_pattern Optional custom dash pattern (e.g., "10,5,2,5").
/// @return Gnuplot-compatible string for the line style.
static inline auto line_type_to_string(line_type_t style, const std::string &custom_pattern = "") -> std::string
{
    switch (style) {
    case line_type_t::solid:
        return "dt 1"; // Solid
    case line_type_t::dashed:
        return "dt 2"; // Dashed
    case line_type_t::dotted:
        return "dt 3"; // Dotted
    case line_type_t::dash_dot:
        return "dt 4"; // Dash-dot
    case line_type_t::dash_dot_dot:
        return "dt 5"; // Dash-dot-dot
    case line_type_t::custom:
        return custom_pattern.empty() ? "dt 1" : "dt (" + custom_pattern + ")";
    default:
        return "dt 1"; // Fallback to solid
    }
}

/// @brief Enum representing the various predefined point styles available in
/// Gnuplot
enum class point_type_t : unsigned char {
    none,                     // No point (invisible).
    plus,                     // Plus (+) shape.
    cross,                    // Cross (×) shape.
    asterisk,                 // Asterisk (*) shape.
    open_square,              // Open square (□).
    filled_square,            // Filled square (■).
    open_circle,              // Open circle (○).
    filled_circle,            // Filled circle (●).
    open_triangle,            // Open triangle (△).
    filled_triangle,          // Filled triangle (▲).
    open_inverted_triangle,   // Open inverted triangle (▽).
    filled_inverted_triangle, // Filled inverted triangle (▼).
    open_diamond,             // Open diamond (◇).
    filled_diamond,           // Filled diamond (◆).
};

/// @brief Converts a point_type_t value to a Gnuplot-compatible string.
/// @param style The point style enumeration.
/// @return Gnuplot-compatible string for the point style.
static inline auto point_type_to_string(point_type_t style = point_type_t::none) -> std::string
{
    return std::to_string(static_cast<int>(style));
}

/// @brief Enum representing the horizontal alignment options for labels in Gnuplot.
enum class halign_t : unsigned char {
    left,   ///< Align label to the left of the (x, y) position.
    center, ///< Align label to the center of the (x, y) position.
    right   ///< Align label to the right of the (x, y) position.
};

/// @brief Enumeration of gnuplot terminal types.
/// @details This enum class represents all terminal types supported by gnuplot
/// as per the provided list. Each terminal type corresponds to a specific
/// plotting or output format.
enum class terminal_type_t : unsigned char {
    wxt,          ///< wxWidgets cross-platform interactive terminal
    cairolatex,   ///< LaTeX picture environment using graphicx package and Cairo backend
    canvas,       ///< HTML Canvas object
    cgm,          ///< Computer Graphics Metafile
    context,      ///< ConTeXt with MetaFun (for PDF documents)
    domterm,      ///< DomTerm terminal emulator with embedded SVG
    dpu414,       ///< Seiko DPU-414 thermal printer
    dumb,         ///< ASCII art for anything that prints text
    dxf,          ///< DXF file for AutoCAD
    emf,          ///< Enhanced Metafile format
    epscairo,     ///< EPS terminal based on Cairo
    epslatex,     ///< LaTeX picture environment using graphicx package
    epson_180dpi, ///< Epson LQ-style 180-dot per inch (24 pin) printers
    epson_60dpi,  ///< Epson-style 60-dot per inch printers
    epson_lx800,  ///< Epson LX-800, Star NL-10, NX-1000, PROPRINTER
    fig,          ///< FIG graphics language for XFIG graphics editor
    gif,          ///< GIF images using libgd and TrueType fonts
    hp500c,       ///< HP DeskJet 500c
    hpdj,         ///< HP DeskJet 500
    hpgl,         ///< HP7475 and relatives
    hpljii,       ///< HP Laserjet series II
    hppj,         ///< HP PaintJet and HP3630
    jpeg,         ///< JPEG images using libgd and TrueType fonts
    lua,          ///< Lua generic terminal driver
    mf,           ///< Metafont plotting standard
    mp,           ///< MetaPost plotting standard
    nec_cp6,      ///< NEC printer CP6, Epson LQ-800
    okidata,      ///< OKIDATA 320/321 Standard
    pbm,          ///< Portable bitmap
    pcl5,         ///< PCL5e/PCL5c printers using HP-GL/2
    pdfcairo,     ///< PDF terminal based on Cairo
    pict2e,       ///< LaTeX2e picture environment
    png,          ///< PNG images using libgd and TrueType fonts
    pngcairo,     ///< PNG terminal based on Cairo
    postscript,   ///< PostScript graphics, including EPSF embedded files
    pslatex,      ///< LaTeX picture environment with PostScript \\specials
    pstex,        ///< Plain TeX with PostScript \\specials
    pstricks,     ///< LaTeX picture environment with PSTricks macros
    sixelgd,      ///< Sixel using libgd and TrueType fonts
    sixeltek,     ///< Sixel output using bitmap graphics
    starc,        ///< Star Color Printer
    svg,          ///< W3C Scalable Vector Graphics
    tandy_60dpi,  ///< Tandy DMP-130 series 60-dot per inch graphics
    tek40xx,      ///< Tektronix 4010 and others; most TEK emulators
    tek410x,      ///< Tektronix 4106, 4107, 4109 and 420X terminals
    texdraw,      ///< LaTeX texdraw environment
    tikz,         ///< TeX TikZ graphics macros via the Lua script driver
    tkcanvas,     ///< Tk canvas widget
    unknown,      ///< Unknown terminal type - not a plotting device
    vttek,        ///< VT-like Tek40xx terminal emulator
    x11,          ///< X11 Window System interactive terminal
    xlib,         ///< X11 Window System (dump of gnuplot_x11 command stream)
    xterm         ///< Xterm Tektronix 4014 Mode
};

/// @brief Converts a terminal_type_t enum value to a corresponding
/// gnuplot-compatible string.
/// @details This function takes a terminal_type_t enum value and returns
/// the string representation accepted by gnuplot. For custom terminals, the
/// custom options can be appended.
/// @param terminal The terminal_type_t enum value.
/// @return A string representing the gnuplot terminal type.
static inline auto terminal_type_to_string(terminal_type_t type) -> std::string
{
    switch (type) {
    case terminal_type_t::cairolatex:
        return "cairolatex";
    case terminal_type_t::canvas:
        return "canvas";
    case terminal_type_t::cgm:
        return "cgm";
    case terminal_type_t::context:
        return "context";
    case terminal_type_t::domterm:
        return "domterm";
    case terminal_type_t::dpu414:
        return "dpu414";
    case terminal_type_t::dumb:
        return "dumb";
    case terminal_type_t::dxf:
        return "dxf";
    case terminal_type_t::emf:
        return "emf";
    case terminal_type_t::epscairo:
        return "epscairo";
    case terminal_type_t::epslatex:
        return "epslatex";
    case terminal_type_t::epson_180dpi:
        return "epson_180dpi";
    case terminal_type_t::epson_60dpi:
        return "epson_60dpi";
    case terminal_type_t::epson_lx800:
        return "epson_lx800";
    case terminal_type_t::fig:
        return "fig";
    case terminal_type_t::gif:
        return "gif";
    case terminal_type_t::hp500c:
        return "hp500c";
    case terminal_type_t::hpdj:
        return "hpdj";
    case terminal_type_t::hpgl:
        return "hpgl";
    case terminal_type_t::hpljii:
        return "hpljii";
    case terminal_type_t::hppj:
        return "hppj";
    case terminal_type_t::jpeg:
        return "jpeg";
    case terminal_type_t::lua:
        return "lua";
    case terminal_type_t::mf:
        return "mf";
    case terminal_type_t::mp:
        return "mp";
    case terminal_type_t::nec_cp6:
        return "nec_cp6";
    case terminal_type_t::okidata:
        return "okidata";
    case terminal_type_t::pbm:
        return "pbm";
    case terminal_type_t::pcl5:
        return "pcl5";
    case terminal_type_t::pdfcairo:
        return "pdfcairo";
    case terminal_type_t::pict2e:
        return "pict2e";
    case terminal_type_t::png:
        return "png";
    case terminal_type_t::pngcairo:
        return "pngcairo";
    case terminal_type_t::postscript:
        return "postscript";
    case terminal_type_t::pslatex:
        return "pslatex";
    case terminal_type_t::pstex:
        return "pstex";
    case terminal_type_t::pstricks:
        return "pstricks";
    case terminal_type_t::sixelgd:
        return "sixelgd";
    case terminal_type_t::sixeltek:
        return "sixeltek";
    case terminal_type_t::starc:
        return "starc";
    case terminal_type_t::svg:
        return "svg";
    case terminal_type_t::tandy_60dpi:
        return "tandy_60dpi";
    case terminal_type_t::tek40xx:
        return "tek40xx";
    case terminal_type_t::tek410x:
        return "tek410x";
    case terminal_type_t::texdraw:
        return "texdraw";
    case terminal_type_t::tikz:
        return "tikz";
    case terminal_type_t::tkcanvas:
        return "tkcanvas";
    case terminal_type_t::unknown:
        return "unknown";
    case terminal_type_t::vttek:
        return "vttek";
    case terminal_type_t::wxt:
        return "wxt";
    case terminal_type_t::x11:
        return "x11";
    case terminal_type_t::xlib:
        return "xlib";
    case terminal_type_t::xterm:
        return "xterm";
    default:
        return "wxt";
    }
}

} // namespace gpcpp
