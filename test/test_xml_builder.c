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

    const char * value = (const char *)IupGetAttribute(ih, "hero");

    IupMessage("button event message", value);

    return IUP_DEFAULT;
}

static void test_xml_builder_parse_file() {
    DEBUG_LOG(">>>\n");

    iup_xml_builder_t *builder = iup_xml_builder_new();

    assert(builder != NULL);

    iup_xml_builder_add_file(builder, "testdialog", "dialog.xml");
    iup_xml_builder_add_file(builder, "testdialog2", "dialog.xml");

    iup_xml_builder_add_callback(builder, "testcallback", (Icallback)test_callback_btn);
    iup_xml_builder_add_user_data(builder, "testdata", (void*)"Das hier ist ein userdata text :)");

    iup_xml_builder_parse(builder);

    Ihandle *result = iup_xml_builder_get_result(builder, "testdialog");

    result = iup_xml_builder_get_result_new(builder, "testdialog2");

    Ihandle *handle = iup_xml_builder_get_main(result);

    IupShowXY(handle, IUP_CENTER, IUP_CENTER);

    Ihandle *list = iup_xml_builder_get_name(result, "mylist");

    assert(list != NULL);

    IupSetAttribute(list, "APPENDITEM", "Added from outside");

    assert(handle != NULL);

    iup_xml_builder_free_result(&result);

    assert(result == NULL);

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