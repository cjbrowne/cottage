#include <assert.h>
#include <panic.h>

void __assert_fail(const char * assertion, const char * file, unsigned int line, const char * function)
{
    panic("Assertion failed on line %s:%u (%s): %s", file, line, function, assertion);
}
