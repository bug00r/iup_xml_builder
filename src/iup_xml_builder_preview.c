
#include "dl_list.h"
#include "xml_utils.h"
#include "resource.h"
#include "iup_xml_builder.h"
#include "defs.h"

EXTERN_BLOB(zip_resource, 7z);

void _iup_xb_prev_xml_err_to_handle(void **data, void *log) {
    if ( data != NULL && *data != NULL ) {
        IupSetAttribute(log, "APPEND", (char*)*data);
    }
}

static bool _iup_xb_prev_add_err(iup_xml_builder_t *builder, Ihandle *log) {
    bool logged = false;

    if (log != NULL && builder->err->cnt > 0 ) {
        dl_list_each_data(builder->err, (void*)log, _iup_xb_prev_xml_err_to_handle);
        logged = true;
    }
    
    return logged;
}

int on_close_preview_dlg(Ihandle *ih) {

    iup_xml_builder_t *builder = (iup_xml_builder_t *)IupGetAttribute(ih, "builder");

    iup_xml_builder_free(&builder);

    IupDestroy(ih);

    return IUP_IGNORE;
}


static int _iup_xb_prev_show_preview(Ihandle *ih) {

    IupSetAttribute(ih, "ACTIVE", "NO");

    Ihandle *xml = (Ihandle*)IupGetAttribute(ih, "xml");
    Ihandle *log = (Ihandle*)IupGetAttribute(ih, "log");

    IupSetAttribute(log, "VALUE", "");
    const char * xml_val = (const char*)IupGetAttribute(xml, "VALUE");

    size_t input_len = strlen(xml_val);

    if (input_len > 0 ) {

        iup_xml_builder_t *builder = iup_xml_builder_new();

        iup_xml_builder_add_bytes(builder, "main",  xml_val, strlen(xml_val));
        
        if ( !_iup_xb_prev_add_err(builder, log) ) {

            iup_xml_builder_parse(builder);

            if ( !_iup_xb_prev_add_err(builder, log) ) {

                Ihandle *mres = iup_xml_builder_get_result(builder, "main");

                Ihandle * main_ = iup_xml_builder_get_main(mres);

                Ihandle *dialog = main_;

                if (strcmp(IupGetClassName(dialog),"dialog") != 0) {
                    dialog = IupDialog(main_);
                    IupSetAttribute(dialog, "SIZE", "HALFxHALF");
                }

                IupSetCallback(dialog, "CLOSE_CB", (Icallback) on_close_preview_dlg);
                IupSetAttribute(dialog, "builder", (void*)builder);

                IupShowXY(dialog, IUP_CENTER, IUP_CENTER);

            }

        } 

    }

     IupSetAttribute(ih, "ACTIVE", "YES");

    return IUP_DEFAULT;
}

int on_xml_src_caret_changed(Ihandle *ih, int _lin, int _col, int _pos) {

    IupSetStrf((Ihandle*)IupGetAttribute(ih, "line"), "TITLE", "%i", _lin);
    IupSetStrf((Ihandle*)IupGetAttribute(ih, "pos"), "TITLE", "%i", _pos);
    IupSetStrf((Ihandle*)IupGetAttribute(ih, "col"), "TITLE", "%i", _col);

    return IUP_DEFAULT;

}

int main(int argc, char **argv) {

    IupOpen(&argc, &argv);

    IupSetGlobal("UTF8MODE", "YES"); 

    archive_resource_t* ar = archive_resource_memory(&_binary_zip_resource_7z_start, (size_t)&_binary_zip_resource_7z_size);
    
    xml_source_t* xml_src = xml_source_from_resname(ar, "main");

    iup_xml_builder_t *builder = iup_xml_builder_new();

    iup_xml_builder_add_bytes(builder, "main",  (const char *)xml_src->src_data, *xml_src->src_size);

    iup_xml_builder_add_callback(builder, "previewclb", (Icallback)_iup_xb_prev_show_preview);
    iup_xml_builder_add_callback(builder, "caretclb", (Icallback)on_xml_src_caret_changed);

    iup_xml_builder_parse(builder);

    Ihandle *mres = iup_xml_builder_get_result(builder, "main");

    Ihandle * main_ = iup_xml_builder_get_main(mres);

    IupShowXY(main_, IUP_CENTER, IUP_CENTER);

    IupMainLoop(); 

    iup_xml_builder_free(&builder);

    IupClose();

    xml_source_free(&xml_src);

    archive_resource_free(&ar);

	return 0;
}