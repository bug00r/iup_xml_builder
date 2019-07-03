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

int test_callback_linkhandle_btn(Ihandle* ih) {

    Ihandle *mylist = (Ihandle* )IupGetAttribute(ih, "mylist");

    const char * value = (const char*)IupGetAttribute(mylist, "VALUESTRING");

    IupMessage("button event message for list", value);

    return IUP_DEFAULT;
}

static void test_xml_builder_parse_file() {
    DEBUG_LOG(">>>\n");

    iup_xml_builder_t *builder = iup_xml_builder_new();

    assert(builder != NULL);

    iup_xml_builder_add_file(builder, "testdialog", "dialog.xml");
    iup_xml_builder_add_file(builder, "window", "window.xml");

    iup_xml_builder_add_callback(builder, "testcallback", (Icallback)test_callback_btn);
    iup_xml_builder_add_callback(builder, "testcallbacklnk", (Icallback)test_callback_linkhandle_btn);
    iup_xml_builder_add_user_data(builder, "testdata", (void*)"Das hier ist ein userdata text :)");

    iup_xml_builder_parse(builder);

    Ihandle *result = iup_xml_builder_get_result(builder, "testdialog");

    DEBUG_LOG("=> copy from window:\n");

    /*copy interface struggles with handle linking. Currently it is better 
      to create a new builder for each copy. linking have to be manually.
    */
    //Ihandle * result_win = iup_xml_builder_get_result_new(builder, "window");
    Ihandle * result_win = iup_xml_builder_get_result(builder, "window");

    Ihandle *handle = iup_xml_builder_get_main(result);

    Ihandle *window = iup_xml_builder_get_main(result_win);

    Ihandle *hbox = iup_xml_builder_get_name(result, "myhbox");

    IupAppend(hbox, window);

    IupShowXY(handle, IUP_CENTER, IUP_CENTER);

    Ihandle *list = iup_xml_builder_get_name(result, "mylist");

    assert(list != NULL);

    IupSetAttribute(list, "APPENDITEM", "Added from outside");

    assert(handle != NULL);

    //iup_xml_builder_free_result(&result_win);
    //assert(result_win == NULL);

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