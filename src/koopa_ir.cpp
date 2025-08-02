#include "koopa_ir.h"

const char* c_string(std::string name) {
    auto* str = new char[name.size() + 1];
    std::copy(name.begin(), name.end(), str);
    str[name.size()] = '\0';
    return str;
}

const char* c_string(int value) {
    auto* str = new char[12];  // Enough for int32_t
    snprintf(str, 12, "%%%d", value);
    return str;
}