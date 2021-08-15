#include <stdio.h>

#include <vasq/safe_snprintf.h>

int
main()
{
    int num;
    ssize_t written;
    char buffer[100];

    vasqSafeSnprintf(buffer, sizeof(buffer), "Printing zero: %i", 0);
    puts(buffer);

    vasqSafeSnprintf(buffer, sizeof(buffer), "Printing a pointer: %p", &num);
    puts(buffer);

    written = vasqSafeSnprintf(buffer, sizeof(buffer), "This line has %n", &num);
    vasqSafeSnprintf(buffer + written, sizeof(buffer) - written, "%i characters before the integer.", num);
    puts(buffer);

    return 0;
}
