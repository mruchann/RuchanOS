#include "common.h"

void putchar(char ch);

void printf(const char* fmt, ...) {
    va_list vargs;
    va_start(vargs, fmt);
    int value;

    while (*fmt) {
        if (*fmt == '%') {
            fmt++;
            switch(*fmt) {
                case '\0':
                    putchar('%');
                    break;

                case '%':
                    putchar('%');
                    break;

                case 's':
                    const char* s = va_arg(vargs, const char*);
                    while (*s) {
                        putchar(*s++);
                    }
                    break;

                case 'd':
                    value = va_arg(vargs, int);
                    if (value < 0) {
                        putchar('-');
                        value = -value;
                    }

                    int divisor = 1;
                    while (value / divisor >= 10) {
                        divisor *= 10;
                    }

                    while (divisor > 0) {
                        char digit = '0' + value / divisor;
                        putchar(digit);
                        value %= divisor;
                        divisor /= 10;
                    }

                    break;

                case 'x':
                    value = va_arg(vargs, int);
                    for (int i = 7; i >= 0; i--) {
                        int nibble = (value >> (4 * i)) & 0xf;
                        putchar("0123456789abcdef"[nibble]);
                    }

                    break;
            }
        }

        else {
            putchar(*fmt);
        }


        fmt++;
    }

end:
    va_end(vargs);
}

