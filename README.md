# About

--------------------------------------------------------------------------------
- **This is a fork of the cpp-html library.**                                  -
for the original code, see <https://github.com/povilasb/cpp-html>              -
--------------------------------------------------------------------------------

The htmlreader library <https://github.com/wirbel-at-vdr-portal/htmlreader> is
based on
  - cpp-html <https://github.com/povilasb/cpp-html>, which is based on
  - pugihtml <https://github.com/rofldev/pugihtml> , which is based on
  - pugixml  <https://github.com/zeux/pugixml>


The project was renamed to htmlreader, to allow installing the original version
in parallel.

Other changes are
- standard Makefile, instead of cmake
- README.md instead of README.rst
- change .gitignore
- change header extension from .hpp to .h
- change folder structure
- move class functions out of header
- get rid of '-' in filenames
- add doc Makefile target for html documentation and Doxyfile.
- add new tool functions for easier handling in code
- remove unrelated XML examples
- merge headers


--------------------------------------------------------------------------------
- About Library Versioning                                                     -
--------------------------------------------------------------------------------
This library uses a three number version numbering, following to what i under-
stand as "reasonable versioning rules".
The rule of thumb is here:

    version = MAJOR.MINOR.PATCH

example: 1.0.0

MAJOR increased -> an API-incompatible change was introduced. Installing the lib
                   may break installed packages using the old lib.

MINOR increased -> an API-compatible, new functionality was added. It's safe to
                   replace any older version of this lib with a newer one.

PATCH increased -> the API is untouched. An inprovement or bug fix was done. It
                   is recommended to replace older libs with same MAJOR.MINOR.

--------------------------------------------------------------------------------
- Installation on Linux                                                        -
--------------------------------------------------------------------------------
Installation:
   make -j8
   sudo make install

It's possible to build a static version of this lib (htmlreader.a):
   make -j8 htmlreader.a
   sudo make install-static

--------------------------------------------------------------------------------
- mingw64 on WIN32 x86_64                                                      -
--------------------------------------------------------------------------------
Installation (in mingw64 window):
   make dll

After that, you copy the dll and header to their destinations. Suggestion for
a possible 'make install-dll' target where to put the dll and header are welcome.

Linking to the lib is done by adding
   -l:htmlreader.dll 
to your programs Makefile.


have phun,
--wirbel

For reference, the text of the original README.rst is included in the doc folder.
