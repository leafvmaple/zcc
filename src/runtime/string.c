#include "minilib.h"

long strlen(char* text) {
    long n = 0;
    while (text[n])
        ++n;
    return n;
}
int strcmp(char* first, char* second) {
    while (*first && *first == *second) {
        ++first;
        ++second;
    }
    return (unsigned char)*first - (unsigned char)*second;
}
void* memcpy(void* dest, const void* source, __SIZE_TYPE__ size) {
    unsigned char* out = dest;
    const unsigned char* in = source;
    for (__SIZE_TYPE__ i = 0; i < size; ++i)
        out[i] = in[i];
    return dest;
}
void* memset(void* dest, int value, __SIZE_TYPE__ size) {
    unsigned char* out = dest;
    for (__SIZE_TYPE__ i = 0; i < size; ++i)
        out[i] = (unsigned char)value;
    return dest;
}
void* memmove(void* dest, const void* source, __SIZE_TYPE__ size) {
    unsigned char* out = dest;
    const unsigned char* in = source;
    if ((unsigned long)out < (unsigned long)in)
        return memcpy(dest, source, size);
    while (size) {
        --size;
        out[size] = in[size];
    }
    return dest;
}

int memcmp(const void* first, const void* second, __SIZE_TYPE__ size) {
    const unsigned char* a = first;
    const unsigned char* b = second;
    for (__SIZE_TYPE__ i = 0; i < size; ++i)
        if (a[i] != b[i])
            return a[i] - b[i];
    return 0;
}
