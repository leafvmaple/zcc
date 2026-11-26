#include "minilib.h"

__SIZE_TYPE__ strlen(const char* text) {
    __SIZE_TYPE__ n = 0;
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
typedef unsigned long __attribute__((__may_alias__)) copy_word;
typedef unsigned int __attribute__((__may_alias__)) copy_half;

/* Copies low to high, so memmove uses it when the destination is below the
 * source. Whole words move when both pointers share their alignment, keeping
 * every access aligned. */
void* memcpy(void* dest, const void* source, __SIZE_TYPE__ size) {
    unsigned char* out = dest;
    const unsigned char* in = source;
    unsigned long apart = (unsigned long)out ^ (unsigned long)in;
    if ((apart & (sizeof(copy_word) - 1)) == 0) {
        while (((unsigned long)out & (sizeof(copy_word) - 1)) != 0 && size != 0) {
            *out++ = *in++;
            --size;
        }
        for (; size >= 4 * sizeof(copy_word); size -= 4 * sizeof(copy_word)) {
            ((copy_word*)out)[0] = ((const copy_word*)in)[0];
            ((copy_word*)out)[1] = ((const copy_word*)in)[1];
            ((copy_word*)out)[2] = ((const copy_word*)in)[2];
            ((copy_word*)out)[3] = ((const copy_word*)in)[3];
            out += 4 * sizeof(copy_word);
            in += 4 * sizeof(copy_word);
        }
        for (; size >= sizeof(copy_word); size -= sizeof(copy_word)) {
            *(copy_word*)out = *(const copy_word*)in;
            out += sizeof(copy_word);
            in += sizeof(copy_word);
        }
    } else if ((apart & (sizeof(copy_half) - 1)) == 0) {
        while (((unsigned long)out & (sizeof(copy_half) - 1)) != 0 && size != 0) {
            *out++ = *in++;
            --size;
        }
        for (; size >= sizeof(copy_half); size -= sizeof(copy_half)) {
            *(copy_half*)out = *(const copy_half*)in;
            out += sizeof(copy_half);
            in += sizeof(copy_half);
        }
    }
    while (size != 0) {
        *out++ = *in++;
        --size;
    }
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
