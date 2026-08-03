#include "validation.h"

int is_valid_field(const char *field, int max_len)
{
    size_t len = strlen(field);
    if (len == 0 || len >= (size_t)max_len)
        return 0;

    for (int i = 0; field[i] != '\0'; i++)
    {
        unsigned char c = (unsigned char)field[i];
        if (!isalnum(c) && c != '_')
            return 0;
    }
    return 1;
}