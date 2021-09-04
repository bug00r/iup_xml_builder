#export MAKE=mingw32-make
AR=ar
ARFLAGS=rcs
PATHSEP=/
CC=gcc
BUILDROOT=build

BUILDDIR=$(BUILDROOT)$(PATHSEP)$(CC)
BUILDPATH=$(BUILDDIR)$(PATHSEP)


ifeq ($(DEBUG),1)
	export debug=-ggdb -Ddebug=1
	export isdebug=1
endif

ifeq ($(ANALYSIS),1)
	export analysis=-Danalysis=1
	export isanalysis=1
endif

ifeq ($(DEBUG),2)
	export debug=-ggdb -Ddebug=2
	export isdebug=1
endif

ifeq ($(DEBUG),3)
	export debug=-ggdb -Ddebug=3
	export isdebug=1
endif

ifeq ($(OUTPUT),1)
	export outimg= -Doutput=1
endif

CFLAGS=-std=c11 -Wpedantic -Wall -Wextra

LIB?=-L/c/dev/lib/
INCLUDE?=-I/c/dev/include/

ONW_LIBS=dl_list utils
IUP_LIBS=cdcontextplus gdiplus im iupcd iup cd
THIRD_PARTY_LIBS=exslt xslt xml2 archive zstd lzma z lz4 bz2 freetype6 iconv
REGEX_LIBS=pcre2-8
OS_LIBS=kernel32 user32 gdi32 winspool comdlg32 advapi32 shell32 uuid ole32 oleaut32 comctl32 ws2_32

USED_LIBS=$(patsubst %,-l%,  $(LIBNAME) $(ONW_LIBS) $(IUP_LIBS) $(REGEX_LIBS) $(THIRD_PARTY_LIBS) $(OS_LIBS) )
#pcre2 config
CFLAGS+=-DPCRE2_STATIC -DIN_LIBXML

#-ggdb  -mwindows
#-pg for profiling 

INCLUDEDIR=-I./src $(INCLUDE)
_SRC_FILES=iup_xml_builder iup_resource

SRC+=$(patsubst %,src/%,$(patsubst %,%.c,$(_SRC_FILES)))
OBJ=$(patsubst %,$(BUILDPATH)/%,$(patsubst %,%.o,$(_SRC_FILES)))
LIBNAME=iup_xml_builder
BIN=lib$(LIBNAME).a

USED_LIBSDIR=-L./$(BUILDPATH) $(LIB)

RES=zip_resource
RES_O=$(RES).o
RES_O_PATH=$(BUILDPATH)$(RES_O)
RES_7Z=$(RES).7z
RES_FILES_PATTERN=./data/*
ZIP=7z
ZIP_ARGS=a -t7z
ZIP_CMD=$(ZIP) $(ZIP_ARGS)

PREVIEW_NAME=iup_xb_preview
PREVIEW_BIN=$(PREVIEW_NAME).exe
PREVIEW_SRC=src/iup_xml_builder_preview.c

all: mkbuilddir mkzip addzip $(BUILDPATH)$(BIN) $(BUILDPATH)$(PREVIEW_BIN)

$(BUILDPATH)$(BIN): $(_SRC_FILES)
	$(AR) $(ARFLAGS) $(BUILDPATH)$(BIN) $(OBJ)

$(_SRC_FILES):
	$(CC) $(CFLAGS) -c src/$@.c -o $(BUILDPATH)$@.o $(INCLUDEDIR) $(debug)

test_xml_builder: mkbuilddir $(BUILDPATH)$(BIN)
	$(CC) $(CFLAGS) ./test/test_xml_builder.c -o $(BUILDPATH)test_xml_builder.exe -I./src/ $(INCLUDEDIR) $(USED_LIBSDIR) -static $(USED_LIBS) $(debug)
	-cp test/dialog.xml $(BUILDPATH)dialog.xml
	-cp test/window.xml $(BUILDPATH)window.xml

test_iup_resource: mkbuilddir $(BUILDPATH)$(BIN)
	$(CC) $(CFLAGS) ./test/test_iup_resource.c -o $(BUILDPATH)test_iup_resource.exe -I./src/ $(INCLUDEDIR) $(USED_LIBSDIR) -static $(USED_LIBS) $(debug)
	$(BUILDPATH)test_iup_resource.exe

$(BUILDPATH)$(PREVIEW_BIN):
	$(CC) $(CFLAGS) $(PREVIEW_SRC) $(RES_O_PATH) -o $(BUILDPATH)$(PREVIEW_BIN) -I./src/ $(INCLUDEDIR) $(USED_LIBSDIR) -static $(USED_LIBS) $(debug) $(release)

.PHONY: clean mkbuilddir small

addzip:
	cd $(BUILDPATH); \
	ld -r -b binary $(RES_7Z) -o $(RES_O)

mkzip:
	-$(ZIP_CMD) $(BUILDPATH)$(RES_7Z) $(RES_FILES_PATTERN)

mkbuilddir:
	mkdir -p $(BUILDDIR)

small:
	-strip $(BUILDPATH)$(BIN)
	-upx $(BUILDPATH)$(BIN)
	
clean:
	-rm -dr $(BUILDROOT)

install:
	mkdir -p $(INSTALL_ROOT)include
	mkdir -p $(INSTALL_ROOT)lib$(BIT_SUFFIX)
	cp ./src/iup_xml_builder.h $(INSTALL_ROOT)include/iup_xml_builder.h
	cp $(BUILDPATH)$(BIN) $(INSTALL_ROOT)lib$(BIT_SUFFIX)/$(BIN)

	