// currently only support the flanterm backend
// future, maybe more? (unlikely though)

#include <flanterm/flanterm.h>
#include <flanterm/backends/fb.h>

static struct flanterm_context* ctx;

void term_init(uint32_t *framebuffer, size_t width, size_t height, size_t pitch)
{
    ctx = flanterm_fb_simple_init(framebuffer, width, height, pitch);
}

void term_write(const char* string, size_t count)
{
    flanterm_write(ctx, string, count);
}