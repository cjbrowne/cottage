#include <klog/klog.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

// bunch of printf state machine variables
// todo: refactor into enums
#define PRINTF_STATE_NORMAL 0
#define PRINTF_STATE_LENGTH 1
#define PRINTF_STATE_LENGTH_SHORT 2
#define PRINTF_STATE_LENGTH_LONG 3
#define PRINTF_STATE_SPEC 4

#define PRINTF_LENGTH_DEFAULT 0
#define PRINTF_LENGTH_SHORT_SHORT 1
#define PRINTF_LENGTH_SHORT 2
#define PRINTF_LENGTH_LONG 3
#define PRINTF_LENGTH_LONG_LONG 4

int sputc(char *str, char c)
{
  
}

int vsnprintf(char *str, size_t n, const char *fmt, va_list args) {
  bool writing =
      true; // turn writing to the buffer off if we reach the end
            // this is used so we can still keep track of how many bytes we
            // would have written and conform to the return value spec
  int state = PRINTF_STATE_NORMAL;
  int length = PRINTF_LENGTH_DEFAULT;
  int radix = 10;
  bool sign = false;
  bool number = false;
  const size_t orig_size = n; // so we can return how many bytes we wrote

  while (*fmt) {
    switch (state) {
    case PRINTF_STATE_NORMAL:
      switch (*fmt) {
      case '%':
        state = PRINTF_STATE_LENGTH;
        break;
      default:
        SPUTC(*fmt);
        break;
      }
      break;

    case PRINTF_STATE_LENGTH:
      switch (*fmt) {
      case 'h':
        length = PRINTF_LENGTH_SHORT;
        state = PRINTF_STATE_LENGTH_SHORT;
        break;
      case 'l':
        length = PRINTF_LENGTH_LONG;
        state = PRINTF_STATE_LENGTH_LONG;
        break;
      default:
        goto PRINTF_STATE_SPEC_;
      }
      break;

    case PRINTF_STATE_LENGTH_SHORT:
      if (*fmt == 'h') {
        length = PRINTF_LENGTH_SHORT_SHORT;
        state = PRINTF_STATE_SPEC;
      } else
        goto PRINTF_STATE_SPEC_;
      break;

    case PRINTF_STATE_LENGTH_LONG:
      if (*fmt == 'l') {
        length = PRINTF_LENGTH_LONG_LONG;
        state = PRINTF_STATE_SPEC;
      } else
        goto PRINTF_STATE_SPEC_;
      break;

    case PRINTF_STATE_SPEC:
    PRINTF_STATE_SPEC_:
      switch (*fmt) {
      case 'c':
        SPUTC((char)va_arg(args, int));
        break;

      case 's':
        SPUTS(va_arg(args, const char *), strlen(va_arg(args, const char *)));
        break;

      case '%':
        SPUTC('%');
        break;

      case 'd':
      case 'i':
        radix = 10;
        sign = true;
        number = true;
        break;

      case 'u':
        radix = 10;
        sign = false;
        number = true;
        break;

      case 'X':
      case 'x':
      case 'p':
        radix = 16;
        sign = false;
        number = true;
        break;

      case 'o':
        radix = 8;
        sign = false;
        number = true;
        break;

      // ignore invalid spec
      default:
        break;
      }

      if (number) {
        if (sign) {
          switch (length) {
          case PRINTF_LENGTH_SHORT_SHORT:
          case PRINTF_LENGTH_SHORT:
          case PRINTF_LENGTH_DEFAULT:
            vnsprintf_signed(str, va_arg(args, int), radix);
            break;

          case PRINTF_LENGTH_LONG:
            vnsprintf_signed(va_arg(args, long), radix);
            break;

          case PRINTF_LENGTH_LONG_LONG:
            vnsprintf_signed(va_arg(args, long long), radix);
            break;
          }
        } else {
          switch (length) {
          case PRINTF_LENGTH_SHORT_SHORT:
          case PRINTF_LENGTH_SHORT:
          case PRINTF_LENGTH_DEFAULT:
            vnsprintf_unsigned(va_arg(args, unsigned int), radix);
            break;

          case PRINTF_LENGTH_LONG:
            vnsprintf_unsigned(va_arg(args, unsigned long), radix);
            break;

          case PRINTF_LENGTH_LONG_LONG:
            vnsprintf_unsigned(va_arg(args, unsigned long long), radix);
            break;
          }
        }
      }

      // reset state
      state = PRINTF_STATE_NORMAL;
      length = PRINTF_LENGTH_DEFAULT;
      radix = 10;
      sign = false;
      number = false;
      break;
    default:
      klog("stdio", "Unrecognised printf state");
      break;
    }

    fmt++;
  }

  va_end(args);
}

const char g_HexChars[] = "0123456789abcdef";

int vnsprintf_unsigned(char **str, size_t n, unsigned long long number,
                       int radix) {
  char buffer[32];
  int pos = 0;

  // convert number to ASCII
  do {
    unsigned long long rem = number % radix;
    number /= radix;
    buffer[pos++] = g_HexChars[rem];
  } while (number > 0);

  // print number in reverse order
  while (--pos >= 0)
    SPUTC(buffer[pos]);
}

int term_printf_signed(char *str, size_t n, long long number, int radix) {
  if (number < 0) {
    term_putc('-');
    term_printf_unsigned(-number, radix);
  } else
    term_printf_unsigned(number, radix);
}