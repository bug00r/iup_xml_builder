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

int test_callback_btn(Ihandle* ih) {

    IupMessage("button event message", "triggered from predefined Callback :)");

    return IUP_DEFAULT;
}

static void test_xml_builder_parse_file() {
    DEBUG_LOG(">>>\n");

    iup_xml_builder_t *builder = iup_xml_builder_new();

    assert(builder != NULL);

    iup_xml_builder_add_file(builder, "dialog.xml");

    iup_xml_builder_add_callback(builder, "testcallback", (Icallback)test_callback_btn);

    Ihandle *handle = iup_xml_builder_parse(builder);

    IupShowXY(handle, IUP_CENTER, IUP_CENTER);

    assert(handle != NULL);

    iup_xml_builder_free(&builder);

    assert(builder == NULL);

    DEBUG_LOG("<<<\n");
}

int main(int argc, char **argv) {

    IupOpen(&argc, &argv);

	DEBUG_LOG(">> Start xml builder tests:\n");

    test_xml_builder_alloc_free();

    test_xml_builder_parse_file();

    IupMainLoop();

	DEBUG_LOG("<< end builder tests:\n");

    IupClose();

	return 0;
}