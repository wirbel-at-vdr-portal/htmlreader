# About

**This is a fork of the cpp-html library.**
for the original code, see <https://github.com/povilasb/cpp-html>

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

have phun,
--wirbel

For reference, the original text of the README.rst is included in the doc folder.
