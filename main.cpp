#include <iostream>
#include <string>
#include <format>
#include <memory>
#include <tinyxml2.h>

#define NANOSVG_IMPLEMENTATION
#include "nanosvg.h"

// RAII wrapper to automatically free NSVGimage memory
struct SVGDeleter {
    void operator()(NSVGimage* img) const { if (img) nsvgDelete(img); }
};
using UniqueSVG = std::unique_ptr<NSVGimage, SVGDeleter>;

std::string formatHexColor(unsigned int color) {
    // NanoSVG stores colors in ABGR layout (Byte 0: R, Byte 1: G, Byte 2: B)
    unsigned int r = color & 0xFF;
    unsigned int g = (color >> 8) & 0xFF;
    unsigned int b = (color >> 16) & 0xFF;
    return std::format("#{:02X}{:02X}{:02X}", r, g, b);
}

std::string generatePathData(const NSVGshape* shape) {
    std::string pathData;

    for (const NSVGpath* path = shape->paths; path != nullptr; path = path->next) {
        for (int i = 0; i < path->npts - 1; i += 3) {
            const float* p = &path->pts[i * 2];
            if (i == 0) {
                pathData += std::format("M{:.2f},{:.2f} ", p[0], p[1]);
            }
            pathData += std::format("C{:.2f},{:.2f} {:.2f},{:.2f} {:.2f},{:.2f} ",
                                    p[2], p[3], p[4], p[5], p[6], p[7]);
        }
        if (path->closed) {
            pathData += "Z ";
        }
    }

    if (!pathData.empty() && pathData.back() == ' ') {
        pathData.pop_back(); // Clean off trailing space
    }
    return pathData;
}

int main(int argc, char** argv) {
    if (argc < 3) {
        std::cout << std::format("Usage: {} <input.svg> <output.xml>\n", argv[0]);
        return 1;
    }

    UniqueSVG image(nsvgParseFromFile(argv[1], "px", 96.0f));
    if (!image) {
        std::cerr << std::format("Error: Failed to parse '{}'\n", argv[1]);
        return 1;
    }

    tinyxml2::XMLDocument doc;

    doc.InsertFirstChild(doc.NewDeclaration());

    // Create root <vector> element
    auto* vector = doc.NewElement("vector");
    vector->SetAttribute("xmlns:android", "http://schemas.android.com/apk/res/android");
    vector->SetAttribute("android:width", std::format("{:.0f}dp", image->width).c_str());
    vector->SetAttribute("android:height", std::format("{:.0f}dp", image->height).c_str());
    vector->SetAttribute("android:viewportWidth", std::format("{:.0f}", image->width).c_str());
    vector->SetAttribute("android:viewportHeight", std::format("{:.0f}", image->height).c_str());
    doc.InsertEndChild(vector);

    // Iterate shapes and append <path> tags
    for (const NSVGshape* shape = image->shapes; shape != nullptr; shape = shape->next) {
        if (!(shape->flags & NSVG_FLAGS_VISIBLE)) continue;

        auto* path = doc.NewElement("path");
        path->SetAttribute("android:fillColor", formatHexColor(shape->fill.color).c_str());
        path->SetAttribute("android:pathData", generatePathData(shape).c_str());
        vector->InsertEndChild(path);
    }

    // save
    if (doc.SaveFile(argv[2]) != tinyxml2::XML_SUCCESS) {
        std::cerr << std::format("Error: Failed to write output file '{}'\n", argv[2]);
        return 1;
    }

    return 0;
}
