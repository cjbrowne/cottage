# Cottage Libc

When writing one's own operating system, one has three choices for porting user programs that were written in C.

1. You can patch the C programs to directly link against your OS, bypassing the C runtime library altogether.
2. You can port an existing C library (newlibc is a good example, as it's designed for environments without an OS so is
   relatively straightforward to port).
3. You can write your own C library, only implementing the parts that you need as you need them.

With 1, your test loop requires booting the entire operating system to test the program you want to test.  This is
obviously not ideal as it slows down development.  With option 2, you have to port the whole library before it will
properly link and load on your OS - you can't do it piece by piece.  With option 3, you can write your libc such that it
will work without your OS (perhaps with weak linking, or with macros that swap implementations depending on whether
you're compiling for your OS or Linux) so you can test ports on your host system, and you only need to implement what
you actually need for the current port.  On the plus side, you also end up with more knowledge gained, and a libc that
takes advantage of any special features your OS exposes.  On the minus side, you're taking on the entire maintenance and
development burden for your libc.

I've opted for option 3 here, on the grounds that my project goals are learning and fun, and I don't expect anyone else
to actually use this software.  If you do, on your head be it really.

## Building

Run `make` to build all versions of the library.

Run `make linux` to only build the linux-targeted .so.

Run `make cottage` to only build the cottage-targeted .so.


