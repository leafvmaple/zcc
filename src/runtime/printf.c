/* Minimal printf: %d, %s, %c, %x and %%; uses the shared syscall runtime. */
typedef __builtin_va_list va_list;
#define va_start(ap, param) __builtin_va_start(ap, param)
#define va_arg(ap, type) __builtin_va_arg(ap, type)
#define va_end(ap) __builtin_va_end(ap)
long sys_write(int fd, const void *buf, long count);

static int put_bytes(const char *text, long size) {
    if (size > 2147483647)
        return -1;
    long done = 0;
    while (done < size) {
        long count = sys_write(1, text + done, size - done);
        if (count <= 0)
            return -1;
        done += count;
    }
    return (int)done;
}

static int put_number(unsigned int value, unsigned int base, int negative) {
    const char *digits = "0123456789abcdef";
    char buffer[12];
    char *end = buffer + sizeof(buffer);
    char *out = end;
    do {
        *--out = digits[value % base];
        value /= base;
    } while (value);
    if (negative)
        *--out = '-';
    return put_bytes(out, end - out);
}

int printf(const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    int written = 0;
    while (*fmt) {
        char c = *fmt++;
        int count;
        if (c != '%') {
            count = put_bytes(&c, 1);
        } else {
            c = *fmt;
            if (c)
                ++fmt;
            switch (c) {
            case 'd': {
                int value = va_arg(ap, int);
                unsigned int magnitude = value < 0 ? 0u - (unsigned int)value : (unsigned int)value;
                count = put_number(magnitude, 10, value < 0);
                break;
            }
            case 'x': count = put_number(va_arg(ap, unsigned int), 16, 0); break;
            case 's': {
                const char *text = va_arg(ap, const char *);
                long size = 0;
                while (text[size])
                    ++size;
                count = put_bytes(text, size);
                break;
            }
            case 'c': c = (char)va_arg(ap, int); count = put_bytes(&c, 1); break;
            case '%': count = put_bytes("%", 1); break;
            default: {
                char literal[2] = {'%', c};
                count = put_bytes(literal, c ? 2 : 1);
                break;
            }
            }
        }
        if (count < 0 || written > 2147483647 - count) {
            written = -1;
            break;
        }
        written += count;
    }
    va_end(ap);
    return written;
}
