#pragma once
#include <vector>
#include <unordered_map>
#include "token.h"

namespace mem {

    inline std::vector<symtab::object_t> arena;
    inline std::unordered_map<std::string, size_t> var_map;
}