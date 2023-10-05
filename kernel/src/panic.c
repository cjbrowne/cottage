#include "panic.h"
#include <term/term.h>
#include <string.h>
#include <stdbool.h>

// Halt and catch fire function.
static void hcf(void) {
  asm ("cli");
  for (;;) {
    asm ("hlt");
  }
}

extern bool have_term;

void panic(char* msg)
{
  if(have_term)
  {
    term_write("PANIC!\n", 7);
    term_write(msg, strlen(msg));  
  }
  
  hcf();
}