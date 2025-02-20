#include "user.h"

void main(void) {
    __asm__ __volatile__(
        "li sp, 0xdeaadbeef\n"
        "unimp"
    );
}