ARFLAGS?=rcs
PATHSEP?=/
BUILDROOT?=build

BUILDDIR?=$(BUILDROOT)$(PATHSEP)$(CC)
BUILDPATH?=$(BUILDDIR)$(PATHSEP)

ifndef PREFIX
	INSTALL_ROOT=$(BUILDPATH)
else
	INSTALL_ROOT=$(PREFIX)$(PATHSEP)
	ifeq ($(INSTALL_ROOT),/)
	INSTALL_ROOT=$(BUILDPATH)
	endif
endif

ifdef DEBUG
	CFLAGS+=-ggdb
	ifeq ($(DEBUG),)
	CFLAGS+=-Ddebug=1
	else 
	CFLAGS+=-Ddebug=$(DEBUG)
	endif
endif

ifeq ($(M32),1)
	CFLAGS+=-m32
	BIT_SUFFIX+=32
endif

CFLAGS=-std=c11 -Wpedantic -Wall -Wextra

LDFLAGS+=-L./$(BUILDPATH) -L/c/dev/lib/
CFLAGS+=-I./src -I/c/dev/include/

ONW_LIBS=dl_list utils
IUP_LIBS=cdcontextplus gdiplus im iupcd iup cd
THIRD_PARTY_LIBS=exslt xslt xml2 archive zstd lzma z lz4 bz2 freetype6 iconv
REGEX_LIBS=pcre2-8
OS_LIBS=kernel32 user32 gdi32 winspool comdlg32 advapi32 shell32 uuid ole32 oleaut32 comctl32 ws2_32

LDFLAGS+=-static
LDFLAGS+=$(patsubst %,-l%,  $(NAME) $(ONW_LIBS) $(IUP_LIBS) $(REGEX_LIBS) $(THIRD_PARTY_LIBS) $(OS_LIBS) )
#pcre2 config
CFLAGS+=-DPCRE2_STATIC -DIN_LIBXML

_SRC_FILES=iup_xml_builder iup_resource

SRC+=$(patsubst %,src/%,$(patsubst %,%.c,$(_SRC_FILES)))
OBJ=$(patsubst %,$(BUILDPATH)%,$(patsubst %,%.o,$(_SRC_FILES)))
NAME=iup_xml_builder
LIBNAME=lib$(NAME).a
LIB=$(BUILDPATH)$(LIBNAME)

RES=zip_resource
RES_O=$(RES).o
RES_O_PATH=$(BUILDPATH)$(RES_O)
RES_7Z=$(RES).7z
RES_FILES_PATTERN=./data/*
ZIP=7z
ZIP_ARGS=a -t7z
ZIP_CMD=$(ZIP) $(ZIP_ARGS)

PREVIEW_NAME=iup_xb_preview
PREVIEW_BIN=$(BUILDPATH)$(PREVIEW_NAME).exe
PREVIEW_SRC=src/iup_xml_builder_preview.c

all: mkbuilddir mkzip addzip $(LIB) $(PREVIEW_BIN)

$(LIB): $(OBJ)
	$(AR) $(ARFLAGS) $@ $^

#@F = only filename, $(@F:.o=.c) replaces .o with .c
$(OBJ):
	$(CC) $(CFLAGS) -c src/$(@F:.o=.c) -o $@

test_xml_builder: mkbuilddir $(LIB)
	$(CC) $(CFLAGS) ./test/$@.c -o $(BUILDPATH)$@.exe $(LDFLAGS)
	-cp test/dialog.xml $(BUILDPATH)dialog.xml
	-cp test/window.xml $(BUILDPATH)window.xml

test_iup_resource: mkbuilddir $(BUILDPATH)$(LIB)
	$(CC) $(CFLAGS) ./test/$@.c -o $(BUILDPATH)$@.exe $(LDFLAGS)
	$(BUILDPATH)$@.exe

$(PREVIEW_BIN):
	$(CC) $(CFLAGS) $(PREVIEW_SRC) $(RES_O_PATH) -o $@ $(LDFLAGS)

.PHONY: clean mkbuilddir small

addzip:
	cd $(BUILDPATH); \
	ld -r -b binary $(RES_7Z) -o $(RES_O)

mkzip:
	-$(ZIP_CMD) $(BUILDPATH)$(RES_7Z) $(RES_FILES_PATTERN)

mkbuilddir:
	mkdir -p $(BUILDDIR)

small:
	-strip $(BUILDPATH)$(LIB)
	-upx $(BUILDPATH)$(LIB)
	
clean:
	-rm -dr $(BUILDROOT)

install:
	mkdir -p $(INSTALL_ROOT)include
	mkdir -p $(INSTALL_ROOT)lib$(BIT_SUFFIX)
	cp ./src/iup_xml_builder.h $(INSTALL_ROOT)include/iup_xml_builder.h
	cp $(LIB) $(INSTALL_ROOT)lib$(BIT_SUFFIX)/$(LIBNAME)

	