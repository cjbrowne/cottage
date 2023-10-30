# Branches

Try to keep `master` clean and stable at all times.  We'll gradually implement code linting and static analysis checks to prevent PRs merging down which might jeapordise the stability of the nightly build, but for now just be careful.

# PR etiquette

Before opening a PR, make sure there is a relevant issue created and that you have been assigned that issue.  If you want to be assigned an issue, just ask.  This way we can discuss with you whether the work is even valid and worth doing before you waste any of your time making code changes.

Only ever target `develop` with your PRs.  We use `master` to build our nightly images, so will reject any PRs that directly target `master`.  Those with push access can create PRs that target master from develop, in order to publish a release.

# Releases

When we want to release develop -> master, we will freeze accepting PRs into develop for a period of time while those with push access can continue to fix bugs directly on develop and make things nice.  For now, the only release is the nightly which is automatically taken from master at 3am UTC (previously 2am UTC).

Releases are considered to be "generally" stable - that is, you should have a working ISO image that boots if you download the latest nightly release.  

All you have to do to make a release is merge down develop -> master, and wait for the nightly build.
