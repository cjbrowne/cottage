#include "ctype.h"

char toupper(char chr)
{
    return islower(chr) ? chr - 'a' + 'A' : chr;
}

char islower(char chr)
{
    return chr >= 'a' && chr <= 'z';
}
