
#include "internal/paletteparser.h"
#include <iostream>
using std::string;
using std::to_string;

typedef struct vec3 {
    float x, y, z;
} vec3;

// Function to convert a hex color string to RGB
vec3 hex_to_rgb(const string& hex) {
    // Remove '#' if present
    string h = hex;
    if (h[0] == '#') {
        h = h.substr(1);
    }

    // Parse the hex string
    int r = 0, g = 0, b = 0;
    if (h.length() == 6) {
        r = std::stoi(h.substr(0, 2), nullptr, 16);
        g = std::stoi(h.substr(2, 2), nullptr, 16);
        b = std::stoi(h.substr(4, 2), nullptr, 16);
    } else {
        std::cerr << "Malformed hex '" << hex
                  << "'. Expecting a # followed by 6 characters." << std::endl;
        exit(1);
    }
    return vec3(r / 255.0, g / 255.0, b / 255.0);
}

// https://bottosson.github.io/misc/ok_color.h
vec3 oklab_from_rgb(vec3 rgb) {
    float l =
        0.4122214708f * rgb.x + 0.5363325363f * rgb.y + 0.0514459929f * rgb.z;
    float m =
        0.2119034982f * rgb.x + 0.6806995451f * rgb.y + 0.1073969566f * rgb.z;
    float s =
        0.0883024619f * rgb.x + 0.2817188376f * rgb.y + 0.6299787005f * rgb.z;

    float l_ = cbrtf(l);
    float m_ = cbrtf(m);
    float s_ = cbrtf(s);

    return {
        0.2104542553f * l_ + 0.7936177850f * m_ - 0.0040720468f * s_,
        1.9779984951f * l_ - 2.4285922050f * m_ + 0.4505937099f * s_,
        0.0259040371f * l_ + 0.7827717662f * m_ - 0.8086757660f * s_,
    };
}

vec3 hex_to_oklab(const string& hex) {
    return oklab_from_rgb(hex_to_rgb(hex));
}

string PaletteParser::generate_code_insert(const string& filename) {
    string palette_txt = "const vec3[] PALETTE = vec3[](\n";

    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << filename << std::endl;
        exit(1);
    }

    std::string line;
    bool beyond_first_line = false;
    while (std::getline(file, line)) {
        if (line.empty()) {
            continue;
        }
        if (beyond_first_line) {
            palette_txt += ",\n";
        }
        vec3 color = hex_to_oklab(line);
        palette_txt += "\tvec3(" + to_string(color.x) + ", "
            + to_string(color.y) + ", " + to_string(color.z) + ")";
        beyond_first_line = true;
    }

    palette_txt += "\n);\n";

    std::cout << palette_txt << std::endl;

    return palette_txt;
}
