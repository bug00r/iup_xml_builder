#ifndef IUP_XML_BUILDER_H
#define IUP_XML_BUILDER_H

#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <iup.h>

#include "dl_list.h"
#include "string_utils.h"
#include "defs.h"

#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/xpath.h>
#include <libxml/xpathInternals.h>

typedef struct {
    DlList *xml_res;
	DlList *handlelinks;
	DlList *current_handle_links;
    DlList *err;
	uint32_t cntparsed;
	Ihandle * parsed;
	Ihandle * handles;
	Ihandle * callbacks;
	Ihandle * userdata;
} IupXmlBuilder;

IupXmlBuilder* iup_xml_builder_new();
void iup_xml_builder_free(IupXmlBuilder **builder);

void iup_xml_builder_add_file(IupXmlBuilder *builder, const char *name, const char* filename);
void iup_xml_builder_add_bytes(IupXmlBuilder *builder, const char *name, const char * buffer, int size);

void iup_xml_builder_add_callback(IupXmlBuilder *builder, const char* clbk_name, Icallback callback);
void iup_xml_builder_rem_callback(IupXmlBuilder *builder, const char* clbk_name);

void iup_xml_builder_add_user_data(IupXmlBuilder *builder, const char* data_name, void *data);
void iup_xml_builder_rem_user_data(IupXmlBuilder *builder, const char* data_name);

void iup_xml_builder_parse(IupXmlBuilder *builder);

Ihandle* iup_xml_builder_get_result(IupXmlBuilder *builder, const char *name);
Ihandle* iup_xml_builder_get_result_new(IupXmlBuilder *builder, const char *name);

void iup_xml_builder_free_result(Ihandle **result);

Ihandle* iup_xml_builder_get_main(Ihandle *result_handle);
Ihandle* iup_xml_builder_get_name(Ihandle *result_handle, const char *name);

#endif