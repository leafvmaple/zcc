/*
 * Minimal printf for custom OS
 * Supports: %d, %s, %c, %x, %%, \n
 * Uses sys_write syscall directly — no libc dependency
 */

typedef __builtin_va_list va_list;
#define va_start(ap, param) __builtin_va_start(ap, param)
#define va_arg(ap, type)    __builtin_va_arg(ap, type)
#define va_end(ap)          __builtin_va_end(ap)

/* Implemented in arch-specific syscall.S */
long sys_write(int fd, const void *buf, long count);

static void put_char(char c) {
    sys_write(1, &c, 1);
}

static void put_string(const char *s) {
    long len = 0;
    const char *p = s;
    while (*p++) len++;
    sys_write(1, s, len);
}

static void put_int(int n) {
    char buf[20];
    int i = 0;
    int neg = 0;

    if (n < 0) {
        neg = 1;
        /* Handle INT_MIN without UB */
        if (n == (-2147483647 - 1)) {
            put_string("-2147483648");
            return;
        }
        n = -n;
    }

    if (n == 0) {
        buf[i++] = '0';
    } else {
        while (n > 0) {
            buf[i++] = '0' + (n % 10);
            n /= 10;
        }
    }

    if (neg) put_char('-');

    /* Print digits in reverse order */
    while (i > 0) {
        put_char(buf[--i]);
    }
}

static void put_hex(unsigned int n) {
    char buf[16];
    int i = 0;
    const char *hex = "0123456789abcdef";

    if (n == 0) {
        buf[i++] = '0';
    } else {
        while (n > 0) {
            buf[i++] = hex[n & 0xf];
            n >>= 4;
        }
    }

    while (i > 0) {
        put_char(buf[--i]);
    }
}

int printf(const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    int written = 0;

    while (*fmt) {
        if (*fmt == '%') {
            fmt++;
            switch (*fmt) {
            case 'd': {
                int val = va_arg(ap, int);
                put_int(val);
                break;
            }
            case 's': {
                const char *s = va_arg(ap, const char *);
                put_string(s);
                break;
            }
            case 'c': {
                int c = va_arg(ap, int);
                put_char((char)c);
                break;
            }
            case 'x': {
                unsigned int val = va_arg(ap, unsigned int);
                put_hex(val);
                break;
            }
            case '%': {
                put_char('%');
                break;
            }
            default:
                put_char('%');
                put_char(*fmt);
                break;
            }
        } else {
            put_char(*fmt);
        }
        fmt++;
        written++;
    }

    va_end(ap);
    return written;
}
