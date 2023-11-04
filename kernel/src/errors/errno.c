#include <errors/errno.h>
#include <proc/proc.h>
#include <term/term.h>
#include <errno.h>

// a lot of this came from musl (MIT licensed)
// reference: https://git.musl-libc.org/cgit/musl/tree/src/errno/strerror.c

struct errmsgstr_t {
#define E(n, s) char str##n[sizeof(s)];
#include <errors/errlist.h>
#undef E
};

static const struct errmsgstr_t errmsgstr = {
#define E(n, s) s,
#include <errors/errlist.h>
#undef E
};

static const unsigned short errmsgidx[] = {
#define E(n, s) [n] = offsetof(struct errmsgstr_t, str##n),
#include <errors/errlist.h>
#undef E
};

void set_errno(int num)
{
    thread_t* t = get_current_thread();
    t->errnum = num;
}

const char* strerror(int num)
{
    return (char*)&errmsgstr + errmsgidx[num];
}

void perror(const char* msg)
{
    thread_t* t = get_current_thread();
    term_printf("error: %s: %s", msg, strerror(t->errnum));
}



