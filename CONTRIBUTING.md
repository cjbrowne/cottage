# Branches

Try to keep `master` clean and stable at all times.  We'll gradually implement code linting and static analysis checks to prevent PRs merging down which might jeapordise the stability of the nightly build, but for now just be careful.

# PR etiquette

Before opening a PR, make sure there is a relevant issue created and that you have been assigned that issue.  If you want to be assigned an issue, just ask.  This way we can discuss with you whether the work is even valid and worth doing before you waste any of your time making code changes.

Only ever target `develop` with your PRs.  We use `master` to build our nightly images, so will reject any PRs that directly target `master`.  Those with push access can create PRs that target master from develop, in order to publish a release.

# Releases

When we want to release develop -> master, we will freeze accepting PRs into develop for a period of time while those with push access can continue to fix bugs directly on develop and make things nice.  For now, the only release is the nightly which is automatically taken from master at 3am UTC (previously 2am UTC).

Releases are considered to be "generally" stable - that is, you should have a working ISO image that boots if you download the latest nightly release.  

All you have to do to make a release is merge down develop -> master, and wait for the nightly build.

# Memory management

A few conventions help us keep the memory management more or less sane.

First: any pointer that forms part or all of a return value from a function must always point to heap-allocated memory, or NULL.  It must never point to static allocated (e.g string literal) or stack-allocated memory.  In other words it should always be safe to call "free" on the pointer.

Second: when constructing a structure that contains a string pointer (i.e char*), you *must* copy the string into the structure and allocate your own memory for it.  This memory should then be freed up whenever
the structure's lifetime ends.  Non-string pointers should not use these semantics.  This helps prevent issues when cleaning up structures, that you don't know if the strings are heap-allocated or not.
