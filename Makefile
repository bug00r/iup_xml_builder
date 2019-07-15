include ../make_config

CFLAGS+=-std=c11 -Wall

#-ggdb  -mwindows
#-pg for profiling 

#iup2.4
#IUP_INCLUDE=$(patsubst %,tec_tools/%/include/,iup cd im)
#iup2.7
IUP_INCLUDE=tec_tools/v27/release/include/
THIRD_PARTY_LIB_DIR=./../_third_/

INCLUDEDIR=-I./src -I../utils/src -I../collections/dl_list
INCLUDEDIR+=$(patsubst %,-I./src/%, lexicon taw_calc main hgen utils)  
INCLUDEDIR+=$(patsubst %,-I$(THIRD_PARTY_LIB_DIR)%,$(IUP_INCLUDE) pcre2_bin/include libxml_bin/include/libxml2 libxslt_bin/include)

_SRC_FILES=iup_xml_builder

#_SRC_UTILS=utils/iup_std_callbacks utils/iup_tab_utils utils/iup_dlg_utils
#_SRC_PLUGIN_MAIN=main/plugin_main
#_SRC_PLUGIN_LEXICON=lexicon/plugin_lexicon lexicon/plugin_lexicon_ui lexicon/plugin_lexicon_ui_callback lexicon/plugin_lexicon_ui_search
#_SRC_PLUGIN_TAW_CALC=taw_calc/plugin_taw_calc
#_SRC_PLUGIN_HGEN=hgen/plugin_hgen hgen/plugin_hgen_ui \
				 hgen/plugin_hgen_ui_callback \
				 hgen/plugin_hgen_ui_hero_details \
				 hgen/plugin_hgen_ui_funcs \
				 hgen/plugin_hgen_ui_utils
				 

#_SRC_PLUGIN_FILES=$(_SRC_PLUGIN_MAIN) $(_SRC_PLUGIN_LEXICON) $(_SRC_PLUGIN_TAW_CALC) $(_SRC_PLUGIN_HGEN) $(_SRC_UTILS)
#$(_SRC_PLUGIN_MAIN) $(_SRC_PLUGIN_LEXICON) $(_SRC_PLUGIN_TAW_CALC) $(_SRC_PLUGIN_HGEN)

#_SRC_FILES+=$(_SRC_PLUGIN_FILES)

SRC+=$(patsubst %,src/%,$(patsubst %,%.c,$(_SRC_FILES)))
OBJ=$(patsubst %,$(BUILDPATH)/%,$(patsubst %,%.o,$(_SRC_FILES)))
LIBNAME=iup_xml_builder
BIN=lib$(LIBNAME).a

ONW_LIBS=dl_list utils
IUP_LIBS=cdcontextplus gdiplus im iupcd iup cd
THIRD_PARTY_LIBS=exslt xslt xml2 archive crypto nettle regex zstd lzma z lz4 bz2 bcrypt freetype6 iconv
REGEX_LIBS=pcre2-8
#this c flags is used by regex lib
CFLAGS+=-DPCRE2_STATIC

OS_LIBS=kernel32 user32 gdi32 winspool comdlg32 advapi32 shell32 uuid ole32 oleaut32 comctl32 ws2_32

USED_LIBS=$(patsubst %,-l%,  $(LIBNAME) $(ONW_LIBS) $(IUP_LIBS) $(REGEX_LIBS) $(THIRD_PARTY_LIBS) $(OS_LIBS) )

#iup2.4
#USED_LIBSDIR=-L$(BUILDPATH) -L$(THIRD_PARTY_LIB_DIR)tec_tools/release/static/ -LC:/dev/opt/msys64/mingw64/lib
#iup2.7
USED_LIBSDIR=-L$(BUILDPATH) -L$(THIRD_PARTY_LIB_DIR)tec_tools/v27/release/lib -LC:/dev/opt/msys64/mingw64/lib
USED_LIBSDIR+=$(patsubst %,-L$(THIRD_PARTY_LIB_DIR)%,libxml_bin/lib libxslt_bin/lib pcre2_bin/lib)
USED_LIBSDIR+=-L./../collections/dl_list/$(BUILDPATH)
USED_LIBSDIR+=-L./../utils/$(BUILDPATH) 

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

#$(CC) $(CFLAGS) $(OBJ) -o $(BUILDPATH)$(BIN) $(INCLUDEDIR)  $(USED_LIBSDIR) -static $(USED_LIBS) $(debug) $(release)

$(_SRC_FILES):
	$(CC) $(CFLAGS) -c src/$@.c -o $(BUILDPATH)$@.o $(INCLUDEDIR) $(debug)

test_xml_builder: mkbuilddir $(BUILDPATH)$(BIN)
	$(CC) $(CFLAGS) ./test/test_xml_builder.c -o $(BUILDPATH)test_xml_builder.exe -I./src/ $(INCLUDEDIR) $(USED_LIBSDIR) -static $(USED_LIBS) $(debug)
	-cp test/dialog.xml $(BUILDPATH)dialog.xml
	-cp test/window.xml $(BUILDPATH)window.xml
	#$(BUILDPATH)test_xml_builder.exe

$(BUILDPATH)$(PREVIEW_BIN):
	$(CC) $(CFLAGS) $(PREVIEW_SRC) $(RES_O_PATH) -o $(BUILDPATH)$(PREVIEW_BIN) -I./src/ $(INCLUDEDIR) $(USED_LIBSDIR) -static $(USED_LIBS) $(debug)

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
	