#/*******************************************************************************
# * htmlreader - a C++ lib for parsing HTML documents, based on pugixml
# ******************************************************************************/


# reasonable versioning rules:
#   - MAJOR++  -> API-incompatible
#   - MINOR++  -> API-compatible, but new functionality
#   - PATCH++  -> API untouched changes
MAJOR = 1
MINOR = 0
PATCH = 0



LIBRARY = libhtmlreader.so
LIBRARY_STATIC = $(LIBRARY:.so=.a)
LIBRARY_MAJOR = $(LIBRARY).$(MAJOR)
LIBRARY_MINOR = $(LIBRARY_MAJOR).$(MINOR)
LIBRARY_PATCH = $(LIBRARY_MINOR).$(PATCH)
VERSION = $(MAJOR).$(MINOR).$(PATCH)
URL = https://github.com/wirbel-at-vdr-portal/htmlreader






#/******************************************************************************
# * if you prefer verbose non-coloured build messages, remove the '@' here:
# *****************************************************************************/
Q = @

CXX ?= g++
CXXFLAGS += -g -O3 -fPIC -Wall -Wextra -Werror=overloaded-virtual -Wfatal-errors
CXXFLAGS += -DVERSION=\"$(VERSION)\"
DEFINES   = -D_POSIX_C_SOURCE


#/******************************************************************************
# * if you are still running a distro, not beeing able to use all valid
# * standard ASCII Characters (32-126) for package names, and prefer to modify
# * packages instead of fixing your broken distro, you may overwrite the
# * package_name here.
# *****************************************************************************/
package_name ?= $(LIBRARY_PATCH)


#/******************************************************************************
# * color definitions, RST=reset, CY=cyan, MG=magenta, BL=blue, (..)
# *****************************************************************************/
RST=\e[0m
CY=\e[1;36m
MG=\e[1;35m
BL=\e[1;34m
YE=\e[1;33m
RD=\e[1;31m
GN=\e[1;32m

#/******************************************************************************
# * programs, override if on different paths.
# *****************************************************************************/
AR              ?= ar
CD              ?= cd
CP              ?= cp
CHMOD           ?= chmod
CPPCHECK        ?= cppcheck
GIT             ?= git
INSTALL         ?= install
INSTALL_PROGRAM ?= $(INSTALL) -m 755
INSTALL_DATA    ?= $(INSTALL) -m 644
LN              ?= ln
LN_S            ?= $(LN) -s
LN_SF           ?= $(LN) -sf
LN_SFR          ?= $(LN) -sfr
MAKE            ?= make
MKDIR           ?= mkdir
MKDIR_P         ?= ${MKDIR} -p
PKG_CONFIG      ?= pkg-config
RANLIB          ?= @ranlib
RM              ?= rm
RM_R            ?= rm -r
SED             ?= sed
SHELL            = /bin/sh
STRIP           ?= strip
TAR             ?= tar
WGET            ?= wget


#/******************************************************************************
# * directories
# *****************************************************************************/
srcdir           = $(shell pwd)
tmpdir          ?= /tmp
prefix          ?= /usr
sysconfdir       = $(prefix)/etc
includedir       = $(prefix)/include
sharedstatedir   = $(prefix)/com
localstatedir    = $(prefix)/var
datarootdir      = $(prefix)/share
exec_prefix     ?= $(prefix)
bindir           = $(exec_prefix)/bin
sbindir          = $(exec_prefix)/sbin
libexecdir       = $(exec_prefix)/libexec
libdir           = $(exec_prefix)/lib
datadir          = $(datarootdir)
docdir           = $(datarootdir)/doc/$(package_name)
infodir          = $(datarootdir)/info
localedir        = $(datarootdir)/locale
mandir           = $(datarootdir)/man
man1dir          = $(mandir)/man1
man2dir          = $(mandir)/man2
man3dir          = $(mandir)/man3
man4dir          = $(mandir)/man4
man5dir          = $(mandir)/man5
pkgconfigdir     = /usr/lib/pkgconfig

SOURCES  := $(sort $(wildcard *.cpp))
OBJS      = $(SOURCES:.cpp=.o)
LIBS      =
INCLUDES  = -I$(srcdir) -I$(shell pwd)/include
LDFLAGS  += -shared -pthread
DLL       = $(LIBRARY:.so=.dll)

define PKG_DATA
prefix=$(prefix)
exec_prefix=$${prefix}
includedir=$${prefix}/include
libdir=$${exec_prefix}/lib

Name: htmlreader
Description: a C++ lib for parsing HTML documents, based on pugixml.
URL: $(URL)
Version: $(VERSION)
Libs: -L$${libdir} -l'htmlreader'
Cflags: -I$${includedir}
endef

%.o: %.cpp
ifeq ($(Q),@)
	@echo -e "${BL} CXX $@${RST}"
endif
	$(Q)$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $(DEFINES) $(INCLUDES) -o $@ $<

$(LIBRARY_PATCH): $(OBJS)
ifeq ($(Q),@)
	@echo -e "${GN} LINK $(LIBRARY_PATCH)${RST}"
endif
	$(Q)$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(LDFLAGS) -Wl,-soname,$(LIBRARY_MAJOR) $(OBJS) $(LIBS) -o $(LIBRARY_PATCH)

$(LIBRARY_STATIC): $(OBJS)
ifeq ($(Q),@)
	@echo -e "${GN} CREATE $(LIBRARY_STATIC)${RST}"
endif
	$(Q)$(AR) -r -c $(LIBRARY_STATIC) $(OBJS)
ifeq ($(RANLIB),@ranlib)
	@echo -e "${GN} RANLIB $(LIBRARY_STATIC)${RST}"
endif
	$(Q)$(RANLIB) $(LIBRARY_STATIC)

dll: $(DLL)

$(DLL): $(OBJS)
ifeq ($(Q),@)
	@echo -e "${GN} LINK $(DLL)${RST}"
endif
	$(Q)$(CXX) $(CXXFLAGS) $(LDFLAGS) -Wl,--subsystem,windows,--out-implib,$(DLL).a $(OBJS) $(LIBS) -o $(DLL) 

.PHONY: clean Version.h doc
clean:
	@$(RM) -f $(OBJS) $(LIBRARY)* $(LIBRARY_STATIC) $(DLL) $(DLL).a *.pc

install: $(LIBRARY_PATCH)
	$(file >htmlreader.pc,$(PKG_DATA))
	$(MKDIR_P) $(DESTDIR)$(libdir)
	$(MKDIR_P) $(DESTDIR)$(includedir)
	$(MKDIR_P) $(DESTDIR)$(docdir)
	$(MKDIR_P) $(DESTDIR)$(man1dir)
	$(MKDIR_P) $(DESTDIR)$(pkgconfigdir)
	$(INSTALL_PROGRAM) $(LIBRARY_PATCH) $(DESTDIR)$(libdir)
	$(INSTALL_DATA) htmlreader.h $(DESTDIR)$(includedir)
	$(LN_SFR) $(DESTDIR)$(libdir)/$(LIBRARY_PATCH) $(DESTDIR)$(libdir)/$(LIBRARY_MINOR)
	$(LN_SFR) $(DESTDIR)$(libdir)/$(LIBRARY_MINOR) $(DESTDIR)$(libdir)/$(LIBRARY_MAJOR)
	$(LN_SFR) $(DESTDIR)$(libdir)/$(LIBRARY_MAJOR) $(DESTDIR)$(libdir)/$(LIBRARY)
	$(INSTALL_DATA) LICENSE README.md doc/LICENSE.povilasb.cpp-html doc/README.rst.povilasb.cpp-html $(DESTDIR)$(docdir)
	$(INSTALL_DATA) htmlreader.pc $(DESTDIR)$(pkgconfigdir)
#	$(INSTALL_DATA) doc/htmlreader.1 $(DESTDIR)$(man1dir)

uninstall:
	$(RM) -f $(DESTDIR)$(libdir)/$(LIBRARY_PATCH)
	$(RM) -f $(DESTDIR)$(libdir)/$(LIBRARY_MINOR)
	$(RM) -f $(DESTDIR)$(libdir)/$(LIBRARY_MAJOR)
	$(RM) -f $(DESTDIR)$(libdir)/$(LIBRARY)
	$(RM) -f $(DESTDIR)$(libdir)/$(LIBRARY_STATIC)
	$(RM) -f $(DESTDIR)$(includedir)/htmlreader.h
	$(RM) -f $(DESTDIR)$(docdir)/LICENSE
	$(RM) -f $(DESTDIR)$(docdir)/README.md
	$(RM) -f $(DESTDIR)$(docdir)/LICENSE.povilasb.cpp-html
	$(RM) -f $(DESTDIR)$(docdir)/README.rst.povilasb.cpp-html
	$(RM) -r $(DESTDIR)$(docdir)
	$(RM) -f $(DESTDIR)$(pkgconfigdir)/htmlreader.pc
#	$(RM) -f $(DESTDIR)$(man1dir)/htmlreader.1

dist: clean
	@-$(RM) -rf $(LIBRARY_STATIC)
	@-$(RM) -rf libhtmlreader.so*
	@-$(RM) -rf libhtmlreader.dll*
	@-$(RM) -rf *.tar.bz2
	@-$(RM) -rf $(tmpdir)/$(LIBRARY_PATCH)
	@$(MKDIR_P) $(tmpdir)/$(LIBRARY_PATCH)
	@$(CP) -a * $(tmpdir)/$(LIBRARY_PATCH)
	@$(TAR) cfj $(LIBRARY_PATCH).tar.bz2 -C $(tmpdir) $(LIBRARY_PATCH)
	@-$(RM) -rf $(tmpdir)/$(LIBRARY_PATCH)
	@echo Distribution package created as $(LIBRARY_PATCH).tar.bz2


doc:
	doxygen Doxyfile

SUPPRESS_CHECK = --suppress=missingIncludeSystem --suppress=unusedFunction --suppress=passedByValue --suppress=constParameterReference

cppcheck:
	$(CPPCHECK) --enable=all $(SUPPRESS_CHECK) --include=$(includedir)/htmlreader.h $(SOURCES) 

#/******************************************************************************
# * debug
# *****************************************************************************/
printvars:
	@echo "VERSION            = $(VERSION)"
	@echo "LIBRARY            = $(LIBRARY)"
	@echo "CXX                = $(CXX)"
	@echo "CXXFLAGS           = $(CXXFLAGS)"
	@echo "DEFINES            = $(DEFINES)"
	@echo "INCLUDES           = $(INCLUDES)"
	@echo "SOURCES            = $(SOURCES)"
	@echo "OBJS               = $(OBJS)"
	@echo "LIBS               = $(LIBS)"
	@echo "LDFLAGS            = $(LDFLAGS)"
	@echo "AR                 = $(AR)"
	@echo "RANLIB             = $(RANLIB)"
