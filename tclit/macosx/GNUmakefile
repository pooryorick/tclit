# Copyright (c) 2002-2008 Daniel A. Steffen <das@users.sourceforge.net>
#
# See the file "license.terms" for information on usage and redistribution of
# this file, and for a DISCLAIMER OF ALL WARRANTIES.

# You may distribute and/or modify this program under the terms of the GNU
# Affero General Public License as published by the Free Software Foundation,
# either version 3 of the License, or (at your option) any later version.
#
# See the file "COPYING" for information on usage and redistribution
# of this file, and for a DISCLAIMER OF ALL WARRANTIES.

# Makefile wrapper to build tcl on Mac OS X in a way compatible with the tk/macosx Xcode buildsystem
#	uses the standard Unix build system in tcl/unix (which can be used directly instead of this
#	if you are not using the tk/macosx projects).
#

#-------------------------------------------------------------------------------------------------------
# customizable settings

DESTDIR			?=
INSTALL_ROOT		?= ${DESTDIR}

BUILD_DIR		?= ${CURDIR}/../../build
SYMROOT			?= ${BUILD_DIR}/${PROJECT}
OBJROOT			?= ${SYMROOT}

EXTRA_CONFIGURE_ARGS	?=
EXTRA_MAKE_ARGS		?=

INSTALL_PATH		?= /Library/Frameworks
PREFIX			?= /usr/local
BINDIR			?= ${PREFIX}/bin
LIBDIR			?= ${INSTALL_PATH}
MANDIR			?= ${PREFIX}/man

# set to non-empty value to install manpages in addition to html help:
INSTALL_MANPAGES	?=

# Checks and overrides for subframework builds
ifeq (${SUBFRAMEWORK},1)
ifeq (${DYLIB_INSTALL_DIR},)
	@echo "Cannot install subframework with empty DYLIB_INSTALL_DIR !" && false
endif
ifeq (${DESTDIR},)
	@echo "Cannot install subframework with empty DESTDIR !" && false
endif
override BUILD_DIR = ${DESTDIR}/build
override INSTALL_PATH = /Frameworks
endif

#-------------------------------------------------------------------------------------------------------
# meta targets

meta			:= all install embedded install-embedded clean distclean test

styles			:= develop deploy

all			:= ${styles}
all			: ${all}

install			:= ${styles:%=install-%}
install			: ${install}
install-%:		action := install-

embedded		:= ${styles:%=embedded-%}
embedded		: embedded-deploy
install-embedded	:= ${embedded:%=install-%}
install-embedded	: install-embedded-deploy

clean			:= ${styles:%=clean-%}
clean			: ${clean}
clean-%:		action := clean-
distclean		:= ${styles:%=distclean-%}
distclean		: ${distclean}
distclean-%:		action := distclean-

test			:= ${styles:%=test-%}
test			: ${test}
test-%:			action := test-

targets			:= $(foreach v,${meta},${$v})

#-------------------------------------------------------------------------------------------------------
# build styles

BUILD_STYLE		=
CONFIGURE_ARGS		=
OBJ_DIR			= ${OBJROOT}/${BUILD_STYLE}

empty			:=
space			:= ${empty} ${empty}
objdir			= $(subst ${space},\ ,${OBJ_DIR})

develop_make_args	:= BUILD_STYLE=Development CONFIGURE_ARGS=--enable-symbols
deploy_make_args	:= BUILD_STYLE=Deployment INSTALL_TARGET=install-strip \
			   EXTRA_CFLAGS=-DNDEBUG
embedded_make_args	:= EMBEDDED_BUILD=1
install_make_args	:= INSTALL_BUILD=1

${targets}:
	${MAKE} ${action}${PROJECT} \
	$(foreach s,${styles} embedded install,$(if $(findstring $s,$@),${${s}_make_args}))

#-------------------------------------------------------------------------------------------------------
# project specific settings

PROJECT			:= tcl
PRODUCT_NAME		:= Tcl

UNIX_DIR		:= ${CURDIR}/../unix
VERSION			:= $(shell awk -F= '/^TCL_VERSION/ {print $$2; nextfile}' ${UNIX_DIR}/configure.ac)
TCLSH			:= tclsh${VERSION}

BUILD_TARGET		:= all tcltest
INSTALL_TARGET		:= install

export CPPROG		:= cp -p

INSTALL_TARGETS		= install-binaries install-headers install-libraries
ifeq (${EMBEDDED_BUILD},)
INSTALL_TARGETS		+= install-private-headers
endif
ifeq (${INSTALL_BUILD}_${EMBEDDED_BUILD}_${BUILD_STYLE},1__Deployment)
INSTALL_TARGETS		+= install-packages html-tcl
ifneq (${INSTALL_MANPAGES},)
INSTALL_TARGETS		+= install-doc
endif
endif

MAKE_VARS		:= INSTALL_ROOT INSTALL_TARGETS VERSION GENERIC_FLAGS
MAKE_ARGS_V		= $(foreach v,${MAKE_VARS},$v='${$v}')

build-${PROJECT}:	target = ${BUILD_TARGET}
install-${PROJECT}:	target = ${INSTALL_TARGET}
clean-${PROJECT} distclean-${PROJECT} test-${PROJECT}: \
			target = $*

DO_MAKE			= +${MAKE} -C "${OBJ_DIR}" ${target} ${MAKE_ARGS_V} ${MAKE_ARGS} ${EXTRA_MAKE_ARGS}

#-------------------------------------------------------------------------------------------------------
# build rules

${PROJECT}:
	${MAKE} install-${PROJECT} INSTALL_ROOT="${OBJ_DIR}/"

${objdir}/Makefile: ${UNIX_DIR}/Makefile.in ${UNIX_DIR}/configure \
		     ${UNIX_DIR}/tclConfig.sh.in Tcl-Info.plist.in
	mkdir -p "${OBJ_DIR}" && cd "${OBJ_DIR}" && \
	if [ ${UNIX_DIR}/configure -nt config.status ]; then ${UNIX_DIR}/configure -C \
	--prefix="${PREFIX}" --bindir="${BINDIR}" --libdir="${LIBDIR}" \
	--mandir="${MANDIR}" --enable-framework --enable-dtrace --disable-zipfs \
	${CONFIGURE_ARGS} ${EXTRA_CONFIGURE_ARGS}; else ./config.status; fi

build-${PROJECT}: ${objdir}/Makefile
	${DO_MAKE}
ifeq (${INSTALL_BUILD},)
# symbolic link hackery to trick
# 'make install INSTALL_ROOT=${OBJ_DIR}'
# into building Tcl.framework and tclsh in ${SYMROOT}
	@cd "${OBJ_DIR}" && mkdir -p $(dir $(subst ${space},\ ,.${LIBDIR})) $(dir $(subst ${space},\ ,.${BINDIR})) "${SYMROOT}" && \
	rm -f ".${LIBDIR}" ".${BINDIR}" && ln -fs "${SYMROOT}" ".${LIBDIR}" && \
	ln -fs "${SYMROOT}" ".${BINDIR}" && ln -fs "${OBJ_DIR}/tcltest" "${SYMROOT}"
endif

install-${PROJECT}: build-${PROJECT}
ifeq (${EMBEDDED_BUILD}_${INSTALL_ROOT},1_)
	@echo "Cannot install-embedded with empty INSTALL_ROOT !" && false
endif
ifeq (${EMBEDDED_BUILD},1)
	@rm -rf "${INSTALL_ROOT}${LIBDIR}/Tcl.framework"
endif
	${DO_MAKE}
ifeq (${INSTALL_BUILD},1)
ifeq (${EMBEDDED_BUILD},1)
# if we are embedding frameworks, don't install tclsh
	@rm -f "${INSTALL_ROOT}${BINDIR}/${TCLSH}" && \
	rmdir -p "${INSTALL_ROOT}${BINDIR}" 2>&- || true
else
# install tclsh symbolic link
	@ln -fs ${TCLSH} "${INSTALL_ROOT}${BINDIR}/tclsh"
endif
endif
ifeq (${BUILD_STYLE}_${EMBEDDED_BUILD},Development_)
# keep copy of debug library around, so that
# Deployment build can be installed on top
# of Development build without overwriting
# the debug library
	@if [ -d "${INSTALL_ROOT}${LIBDIR}/${PRODUCT_NAME}.framework/Versions/${VERSION}" ]; then \
	    cd "${INSTALL_ROOT}${LIBDIR}/${PRODUCT_NAME}.framework/Versions/${VERSION}"; \
	    ln -f "${PRODUCT_NAME}" "${PRODUCT_NAME}_debug"; \
	fi
endif

clean-${PROJECT}: %-${PROJECT}:
	${DO_MAKE}
	rm -rf "${SYMROOT}"/{${PRODUCT_NAME}.framework,${TCLSH},tcltest}
	rm -f "${OBJ_DIR}"{"${LIBDIR}","${BINDIR}"} && \
	rmdir -p "${OBJ_DIR}"$(dir $(subst ${space},\ ,${LIBDIR})) 2>&- || true && \
	rmdir -p "${OBJ_DIR}"$(dir $(subst ${space},\ ,${BINDIR})) 2>&- || true

distclean-${PROJECT}: %-${PROJECT}: clean-${PROJECT}
	${DO_MAKE}
	rm -rf "${OBJ_DIR}"

test-${PROJECT}: %-${PROJECT}: build-${PROJECT}
	${DO_MAKE}

#-------------------------------------------------------------------------------------------------------

.PHONY: ${meta} ${targets} ${PROJECT} build-${PROJECT} install-${PROJECT} \
	clean-${PROJECT} distclean-${PROJECT}

.NOTPARALLEL:

#-------------------------------------------------------------------------------------------------------
