#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>

#include "iup_xml_builder.h"

static void test_xml_builder_alloc_free() {
    DEBUG_LOG(">>> \n");

    iup_xml_builder_t *builder = iup_xml_builder_new();

    assert(builder != NULL);

    iup_xml_builder_free(&builder);

    assert(builder == NULL);

    DEBUG_LOG("<<<\n");
}

static void test_xml_builder_parse_file() {
    DEBUG_LOG(">>>\n");

    iup_xml_builder_t *builder = iup_xml_builder_new();

    assert(builder != NULL);

    iup_xml_builder_add_file(builder, "dialog.xml");

    Ihandle *handle = iup_xml_builder_parse(builder);

    assert(handle != NULL);

    iup_xml_builder_free(&builder);

    assert(builder == NULL);

    DEBUG_LOG("<<<\n");
}

int main(int argc, char **argv) {

	DEBUG_LOG(">> Start xml builder tests:\n");

    test_xml_builder_alloc_free();

    test_xml_builder_parse_file();

	DEBUG_LOG("<< end builder tests:\n");

	return 0;
}