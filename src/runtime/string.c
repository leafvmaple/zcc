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
void* memcpy(void* dest, void* source, long size) {
    unsigned char* out = dest;
    unsigned char* in = source;
    for (long i = 0; i < size; ++i)
        out[i] = in[i];
    return dest;
}
void* memset(void* dest, int value, long size) {
    unsigned char* out = dest;
    for (long i = 0; i < size; ++i)
        out[i] = (unsigned char)value;
    return dest;
}
void* memmove(void* dest, void* source, long size) {
    unsigned char* out = dest;
    unsigned char* in = source;
    if ((unsigned long)out < (unsigned long)in)
        return memcpy(dest, source, size);
    while (size) {
        --size;
        out[size] = in[size];
    }
    return dest;
}

int memcmp(void* first, void* second, long size) {
    unsigned char* a = first;
    unsigned char* b = second;
    for (long i = 0; i < size; ++i)
        if (a[i] != b[i])
            return a[i] - b[i];
    return 0;
}
