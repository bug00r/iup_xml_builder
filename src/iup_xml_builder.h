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
    dl_list_t *xml_res;
	dl_list_t *handlelinks;
	dl_list_t *current_handle_links;
    dl_list_t *err;
	uint32_t cntparsed;
	Ihandle * parsed;
	Ihandle * handles;
	Ihandle * callbacks;
	Ihandle * userdata;
} iup_xml_builder_t;

iup_xml_builder_t* iup_xml_builder_new();
void iup_xml_builder_free(iup_xml_builder_t **builder);

void iup_xml_builder_add_file(iup_xml_builder_t *builder, const char *name, const char* filename);
void iup_xml_builder_add_bytes(iup_xml_builder_t *builder, const char *name, const char * buffer, int size);

void iup_xml_builder_add_callback(iup_xml_builder_t *builder, const char* clbk_name, Icallback callback);
void iup_xml_builder_rem_callback(iup_xml_builder_t *builder, const char* clbk_name);

void iup_xml_builder_add_user_data(iup_xml_builder_t *builder, const char* data_name, void *data);
void iup_xml_builder_rem_user_data(iup_xml_builder_t *builder, const char* data_name);

void iup_xml_builder_parse(iup_xml_builder_t *builder);

Ihandle* iup_xml_builder_get_result(iup_xml_builder_t *builder, const char *name);
Ihandle* iup_xml_builder_get_result_new(iup_xml_builder_t *builder, const char *name);

void iup_xml_builder_free_result(Ihandle **result);

Ihandle* iup_xml_builder_get_main(Ihandle *result_handle);
Ihandle* iup_xml_builder_get_name(Ihandle *result_handle, const char *name);

#endif