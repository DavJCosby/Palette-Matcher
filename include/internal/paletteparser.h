#pragma once

#include <fstream>
using std::string;

class PaletteParser {
  public:
    static string generate_code_insert(const std::string& filename);

  private:
    std::ifstream file;
};
