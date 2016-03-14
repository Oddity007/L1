License: zlib
Copyright 2011-2016 Oliver Daids

L1 is a common language frontend that I intend to use for several other projects.  This implements the source -> IR pass.  It is currently incomplete, unoptimized, and untested.  Enter at your own peril.

Features:
-Tiny (22 KB on OSX/clang with -flto -Os -DNDEBUG)
-Portable C11
-Dependent function/pair types

Plans:
-Add records with row polymorphism
-Finish type inference
-Faster evaluation mechanism
-Package as a library
-Add testing system