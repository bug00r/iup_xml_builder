
#include "xml_utils.h"
#include "resource.h"
#include "iup_xml_builder.h"
#include "defs.h"

EXTERN_BLOB(zip_resource, 7z);

static int _iup_xb_prev_show_preview(Ihandle *ih) {

    Ihandle *xml = (Ihandle*)IupGetAttribute(ih, "xml");
    Ihandle *log = (Ihandle*)IupGetAttribute(ih, "log");

    const char * xml_val = (const char*)IupGetAttribute(xml, "VALUE");

    iup_xml_builder_t *builder = iup_xml_builder_new();

    iup_xml_builder_add_bytes(builder, "main",  xml_val, strlen(xml_val));

    iup_xml_builder_parse(builder);

    Ihandle *mres = iup_xml_builder_get_result(builder, "main");

    Ihandle * main_ = iup_xml_builder_get_main(mres);

    Ihandle *dialog = main_;

    if (strcmp(IupGetClassName(dialog),"dialog") != 0) {
        dialog = IupDialog(main_);
        IupSetAttribute(dialog, "SIZE", "HALFxHALF");
    }

    IupShowXY(dialog, IUP_CENTER, IUP_CENTER);

    iup_xml_builder_free(&builder);

    return IUP_DEFAULT;
}

int main(int argc, char **argv) {

    IupOpen(&argc, &argv);

    iup_xml_builder_t *builder = iup_xml_builder_new();
	
    archive_resource_t* ar = archive_resource_memory(&_binary_zip_resource_7z_start, (size_t)&_binary_zip_resource_7z_size);
    
    xml_source_t* xml_src = xml_source_from_resname(ar, "main");

    iup_xml_builder_add_bytes(builder, "main",  (const char *)xml_src->src_data, *xml_src->src_size);
    
    iup_xml_builder_add_callback(builder, "previewclb", (Icallback)_iup_xb_prev_show_preview);

    iup_xml_builder_parse(builder);

    Ihandle *mres = iup_xml_builder_get_result(builder, "main");

    Ihandle * main_ = iup_xml_builder_get_main(mres);

    IupShowXY(main_, IUP_CENTER, IUP_CENTER);

    iup_xml_builder_free(&builder);

    IupMainLoop();

    IupClose();

    xml_source_free(&xml_src);

    archive_resource_free(&ar);

	return 0;
}