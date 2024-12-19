#include "hexdump.h"

int main(void)
{
    const char buffer[] = "Hello, World!";
    hexdump(buffer, sizeof(buffer));
}
