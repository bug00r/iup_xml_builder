#include "iup_xml_builder.h"

typedef struct {
    dl_list_t *params;
    dl_list_t *attrs;
    dl_list_t *attrs_handles;
    dl_list_t *attrs_s;
    dl_list_t *children;
    dl_list_t *callbacks;
    dl_list_t *userdata;
    dl_list_t *handlelinks;
} iup_xb_parse_entity_t;

typedef struct {
    xmlChar *name;
    xmlChar *value;
} iup_xb_attr_t;

typedef struct {
    xmlChar *name;
    Ihandle *handle;
} iup_xb_handle_t;

typedef iup_xb_handle_t iup_xb_attrs_handle_t;

typedef enum {
    IUP_XB_UNKNOWN_PARAM, IUP_XB_STRING_PARAM, IUP_XB_HANDLE_PARAM
} iup_xb_params_type_t;

typedef struct {
    iup_xb_params_type_t type;
    union {
        xmlChar *str_value;
        iup_xb_handle_t *value;
    };
} iup_xb_params_t;

typedef struct {
	char *name;
	xmlDocPtr doc;
} iup_xml_resource_t;

typedef struct {
    xmlChar *name;
    xmlChar *ref;
    Ihandle *target;
    Ihandle *parent;
} iup_xb_handle_lnk_t;

//NEW 
typedef enum {
    IUP_XB_E_TYPE_UNKNOWN,
    IUP_XB_E_TYPE_ATTR_STR,
    IUP_XB_E_TYPE_ATTRS_STR,
    IUP_XB_E_TYPE_PARAM_STR,
    IUP_XB_E_TYPE_PARAM_NULL,
    IUP_XB_E_TYPE_ATTR_HANDLE, 
    IUP_XB_E_TYPE_PARAM_HANDLE,
    IUP_XB_E_TYPE_CHILD_HANDLE,
    IUP_XB_E_TYPE_USERDATA,
    IUP_XB_E_TYPE_HANDLE_LINK,
    IUP_XB_E_TYPE_CALLBACK,
} iup_xb_elem_type_t;

typedef struct {
    const unsigned char * tag;
    xmlNodePtr node;
    iup_xb_elem_type_t type;
} iup_xb_elem_t;

typedef void* (*IUP_XB_GET_VOID_FUNC)(iup_xml_builder_t*, iup_xb_elem_t*, Ihandle*, iup_xb_parse_entity_t*);

static Ihandle* __iup_xb_parse_node(iup_xml_builder_t* builder, iup_xb_parse_entity_t *parent_entity, iup_xb_elem_t *element);

//EOF NEW

static iup_xml_resource_t* __iup_xb_xml_res_new(const char *name, xmlDocPtr doc) {
    iup_xml_resource_t* newres = malloc(sizeof(iup_xml_resource_t));

    if (newres != NULL) {
        newres->name = copy_string(name);
        newres->doc = doc;
    }

    return newres;
}

static void __iup_xb_xml_res_free(iup_xml_resource_t**res) {
    if (res != NULL && *res != NULL) {
        iup_xml_resource_t *to_delete = *res;

        free(to_delete->name);
        xmlFreeDoc(to_delete->doc);

        free(to_delete);
        *res = NULL;
    }
}

static iup_xb_parse_entity_t *iup_xb_parse_entity_new() {
    iup_xb_parse_entity_t *newentity = malloc(sizeof(iup_xb_parse_entity_t));

    if(newentity) {
        newentity->params = dl_list_new();
        newentity->attrs = dl_list_new();
        newentity->attrs_s = dl_list_new();
        newentity->children = dl_list_new();
        newentity->callbacks = dl_list_new();
        newentity->userdata = dl_list_new();
        newentity->handlelinks = dl_list_new();
        newentity->attrs_handles = dl_list_new();
    }

    return newentity;
}

static iup_xb_attr_t *iup_xb_parse_attr_new(xmlChar *name, xmlChar* value) {
    iup_xb_attr_t* newattr = malloc(sizeof(iup_xb_attr_t));

    if(newattr) {
        newattr->name = name;
        newattr->value = value;
    }

    return newattr;
}

static iup_xb_handle_lnk_t* __iup_xb_handle_lnk_new(xmlChar *name, xmlChar* ref) {
    iup_xb_handle_lnk_t* newlnk = malloc(sizeof(iup_xb_handle_lnk_t));

    if (newlnk != NULL) {
        newlnk->name = name;
        newlnk->ref = ref;
        newlnk->target = NULL;
        newlnk->parent = NULL;
    }

    return newlnk;
}

static iup_xb_handle_t *iup_xb_parse_handle_new() {
    iup_xb_handle_t* newhandle = malloc(sizeof(iup_xb_handle_t));

    if(newhandle) {
        newhandle->name = NULL;
        newhandle->handle = NULL;
    }

    return newhandle;
}

static iup_xb_attrs_handle_t *iup_xb_parse_attrs_handle_new() {
    iup_xb_attrs_handle_t* newhandle = malloc(sizeof(iup_xb_attrs_handle_t));

    if(newhandle) {
        newhandle->name = NULL;
        newhandle->handle = NULL;
    }

    return newhandle;
}

static iup_xb_params_t* iup_xb_params_new() {
    iup_xb_params_t* newparams = malloc(sizeof(iup_xb_params_t));

    if(newparams) {
        newparams->type = IUP_XB_UNKNOWN_PARAM;
        newparams->str_value = NULL;
    }

    return newparams;
}

static void iup_xb_delete_handle(void **data) {
    if (data != NULL && *data != NULL) {
        iup_xb_handle_t *handle = (iup_xb_handle_t *)*data;

        xmlFree(handle->name);
        free(handle);

        *data = NULL;
    }
}

static void iup_xb_delete_params(void **data) {
    if (data != NULL && *data != NULL) {
        iup_xb_params_t *params = (iup_xb_params_t *)*data;

        if (params->type == IUP_XB_STRING_PARAM) {
            xmlFree(params->str_value);
        } else {
            iup_xb_handle_t * handle = params->value;
            xmlFree(handle->name);
            free(handle);
        }

        free(params);
        *data = NULL;
    }
}

static void iup_xb_delete_attrs_handle(void **data) {
    if (data != NULL && *data != NULL) {
        iup_xb_attrs_handle_t *params = (iup_xb_attrs_handle_t *)*data;

        xmlFree(params->name);

        free(params);
        *data = NULL;
    }
}

static void iup_xb_delete_attr(void **data) {
    if (data != NULL && *data != NULL) {
        iup_xb_attr_t *attr = (iup_xb_attr_t *)*data;
        xmlFree(attr->name);
        xmlFree(attr->value);
        free(attr);
        *data = NULL;
    }
}

static void iup_xb_delete_handle_lnk(void **data) {
    if (data != NULL && *data != NULL) {
        iup_xb_handle_lnk_t *handle = (iup_xb_handle_lnk_t *)*data;
        xmlFree(handle->name);
        xmlFree(handle->ref);
        free(handle);
        *data = NULL;
    }
}

static void __iup_xb_clear_list(dl_list_t *list, void (*deletefunc)(void**)) {
    dl_list_each(list, deletefunc);
    dl_list_clear(list);
}

static void iup_xb_parse_entity_reset(iup_xb_parse_entity_t *entity) {
    if (entity != NULL ) {
        iup_xb_parse_entity_t *to_reset = entity;

        __iup_xb_clear_list(to_reset->params, iup_xb_delete_params);
        __iup_xb_clear_list(to_reset->attrs, iup_xb_delete_attr);
        __iup_xb_clear_list(to_reset->attrs_handles, iup_xb_delete_attrs_handle);
        __iup_xb_clear_list(to_reset->attrs_s, iup_xb_delete_attr);
        __iup_xb_clear_list(to_reset->attrs_handles, iup_xb_delete_attr);
        __iup_xb_clear_list(to_reset->callbacks, iup_xb_delete_attr);
        dl_list_clear(to_reset->handlelinks);
        //__iup_xb_clear_list(to_reset->handlelinks, iup_xb_delete_handle_lnk);
        __iup_xb_clear_list(to_reset->userdata, iup_xb_delete_attr);
        __iup_xb_clear_list(to_reset->children, iup_xb_delete_handle);

    }
}

static void iup_xb_parse_entity_free(iup_xb_parse_entity_t **entity) {
    if (entity != NULL && *entity != NULL) {
        iup_xb_parse_entity_t *to_delete = *entity;

        iup_xb_parse_entity_reset(to_delete);

        dl_list_free(&to_delete->attrs);
        dl_list_free(&to_delete->attrs_handles);
        dl_list_free(&to_delete->attrs_s);
        dl_list_free(&to_delete->params);
        dl_list_free(&to_delete->children);
        dl_list_free(&to_delete->callbacks);
        dl_list_free(&to_delete->userdata);
        dl_list_free(&to_delete->handlelinks);
        free(to_delete);
        *entity = NULL;
    }
}


static void __iup_xml_builder_xml_res_free(iup_xml_builder_t *builder) {
    
    dl_list_item_t* cur_node = builder->xml_res->first;

	while(cur_node != NULL)   {
		if ( cur_node->data != NULL ) {
			__iup_xb_xml_res_free((iup_xml_resource_t **)&cur_node->data);
		}
		cur_node = cur_node->next;
	}

    dl_list_free(&builder->xml_res);
}


static void __iup_xb_err_free(void **data) {
    if ( data != NULL && *data != NULL ) {
        char *err = *data;
        free(err);
        *data = NULL;
    }
}

static void __iup_xml_builder_err_free(iup_xml_builder_t *builder) { 
    dl_list_each(builder->err, __iup_xb_err_free);
    dl_list_free(&builder->err);
}

static void _iup_xb_add_err(iup_xml_builder_t *builder, const char * format, ...) {

    va_list vl;
	va_start(vl, format);
    dl_list_append( builder->err, format_string_va_new(format, vl));
	va_end(vl);
    
}

static bool is_value_content(const char *value) {
    return (value != NULL && (strcmp(value, ":c") == 0));
}

static void __iup_xb_config_handles(void **data, void *params) {

    Ihandle **handles = (Ihandle **)params;

    iup_xb_handle_t * handle = (iup_xb_handle_t *)*data;
    
    if (handle) {

        IupAppend(handles[0], handle->handle);

        if(handle->name != NULL) {
            IupSetAttribute(handles[1], (const char *)handle->name, (void*)handle->handle);
        }
    }

}

static void __iup_xb_config_attr(void **data, void *params) {

    iup_xb_attr_t * attr = (iup_xb_attr_t *)*data;

    if (attr) {

        IupSetStrAttribute((Ihandle*)params, (const char *)attr->name, (const char *)attr->value);

    }

}

static void __iup_xb_config_attr_handle(void **data, void *params) {

    iup_xb_attrs_handle_t * attr = (iup_xb_attrs_handle_t *)*data;

    if (attr) {

        IupSetAttributeHandle((Ihandle*)params, (const char *)attr->name, (void*)attr->handle);

    }

}


static void __iup_xb_config_attr_s(void **data, void *params) {

    iup_xb_attr_t * attrs = (iup_xb_attr_t *)*data;

    if (attrs) {

        IupSetAttributes((Ihandle*)params, (const char *)attrs->value);
    }

}


static void __iup_xb_config_callbacks(void **data, void *params) {

    iup_xb_attr_t * callback = (iup_xb_attr_t *)*data;
    
    Ihandle **handles = (Ihandle **)params;

    if (callback) {

        IupSetCallback(handles[0], (const char *)callback->value, (Icallback)IupGetAttribute(handles[1], (const char *)callback->name));

    }

}

static void __iup_xb_config_userdata(void **data, void *params) {

    iup_xb_attr_t * userdat = (iup_xb_attr_t *)*data;
    
    Ihandle **handles = (Ihandle **)params;

    if (userdat) {

        IupSetAttribute(handles[0], (const char *)userdat->name, IupGetAttribute(handles[1], (const char *)userdat->value));

    }

}

static void __iup_xb_config_handlelinks(void **data, void * _params) {

    iup_xb_handle_lnk_t * handlelink = (iup_xb_handle_lnk_t *)*data;
    
    void **params = (void**)_params;

    dl_list_t *handlelinks = (dl_list_t *)params[0];
    Ihandle *handle = (Ihandle *)params[1];

    handlelink->target = handle;

    dl_list_append(handlelinks, handlelink);

}

static void __iup_xb_config_handlelinks_copy(void **data, void * _params) {

    iup_xb_handle_lnk_t * handlelink = (iup_xb_handle_lnk_t *)*data;
    
    dl_list_t *handlelinks = (dl_list_t *)_params;

    dl_list_append(handlelinks, handlelink);

}

static void __iup_xb_config_handle(iup_xml_builder_t *builder, Ihandle * _handle, iup_xb_parse_entity_t *conf_entity) {
    
    if(_handle != NULL && conf_entity != NULL) {
        
        Ihandle *params[2] = {_handle, builder->handles};
        dl_list_each_data(conf_entity->children, (void*)params, __iup_xb_config_handles);

        dl_list_each_data(conf_entity->attrs, (void*)_handle, __iup_xb_config_attr);

        dl_list_each_data(conf_entity->attrs_handles, (void*)_handle, __iup_xb_config_attr_handle);
 
        dl_list_each_data(conf_entity->attrs_s, (void*)_handle, __iup_xb_config_attr_s);

        params[1] = builder->callbacks;
        dl_list_each_data(conf_entity->callbacks, (void*)params, __iup_xb_config_callbacks);

        params[1] = builder->userdata;
        dl_list_each_data(conf_entity->userdata, (void*)params, __iup_xb_config_userdata);
        
        void *params2[2] = {builder->current_handle_links, _handle};
        dl_list_each_data(conf_entity->handlelinks, (void*)params2, __iup_xb_config_handlelinks);

        dl_list_each_data(conf_entity->handlelinks, (void*)builder->handlelinks, __iup_xb_config_handlelinks_copy);
    }
}

static Ihandle* __iup_xb_handle_from_node(iup_xml_builder_t *builder, iup_xb_elem_t *element, iup_xb_parse_entity_t *conf_entity) {
    Ihandle *result = NULL;

    xmlNodePtr node = element->node;
    if(node != NULL && conf_entity != NULL) {
        dl_list_t *params_list = conf_entity->params;

        char **params = NULL; 
        iup_xb_handle_t ** handle_w_name = NULL;

        params = malloc(params_list->cnt * sizeof(char *));
        handle_w_name = malloc(params_list->cnt * sizeof(iup_xb_handle_t *));

        uint32_t cnt = 0;
        uint32_t cnt_handle = 0;

        if (params_list->cnt > 0) {

            dl_list_item_t* cur_node = conf_entity->params->first;

            while(cur_node != NULL) {
                
                iup_xb_params_t* param = (iup_xb_params_t*)cur_node->data;

                if (param->type == IUP_XB_HANDLE_PARAM) {

                    iup_xb_handle_t* handle_param = (iup_xb_handle_t*)param->value;
                    
                    params[cnt++] = (void*)handle_param->handle;

                    if (handle_param->name != NULL) {

                        handle_w_name[cnt_handle++] = handle_param;
                    } 

                } else {
                    params[cnt++] = (void*)param->str_value;
                }

                cur_node = cur_node->next;
            
            }

        }

        if (cnt == 1) {
            result = IupCreatep((const char*)element->tag, (void*)params[0]);
        } else if (cnt > 1) {
            result = IupCreatev((const char*)element->tag, (void**)&params[0]);
        } else if (cnt == 0) {
            result = IupCreate((const char*)element->tag);
        }

        if (cnt > 0) {

            for(uint32_t handl = 0; handl < cnt_handle ; ++handl) {

                iup_xb_handle_t * curhandle = handle_w_name[handl];

                IupSetAttribute(builder->handles, (const char*)curhandle->name, (void*)curhandle->handle);
            
            }

        }
        
        xmlChar *param_name = xmlGetProp(node, (xmlChar*)"name");

        if (param_name) {

            IupSetAttribute(builder->handles, (const char*)param_name, (void*)result);

            xmlFree(param_name);
        }

        free(handle_w_name);
        free(params);
    }

    return result;
}

static iup_xb_params_t* __iup_xb_str_param_get(iup_xb_elem_t *elem) {
    iup_xb_params_t *result = NULL;
        
    xmlNodePtr node = elem->node;

    xmlChar *value = xmlGetProp(node, (xmlChar *)"value");

    if (is_value_content((const char*)value))  {
        xmlFree(value);
        value = xmlNodeGetContent(node);
    } 

    if (value != NULL) {
        result = iup_xb_params_new();
        result->type = IUP_XB_STRING_PARAM;
        result->str_value = value;
    }
    
    return result;
}

static iup_xb_params_t* __iup_xb_handle_param_create(Ihandle* param, xmlNodePtr node) {

    iup_xb_params_t *result = NULL;

    result = iup_xb_params_new();
    result->type = IUP_XB_HANDLE_PARAM;

    result->value = iup_xb_parse_handle_new();
    
    result->value->name = NULL;//xmlGetProp(node, (xmlChar *)"name");
    result->value->handle = param;

    return result;
}

static iup_xb_handle_t* __iup_xb_handle_create(Ihandle *_handle, iup_xb_elem_t *elem) {
    iup_xb_handle_t* handlet = NULL;

    xmlNodePtr node = elem->node;

    if (_handle != NULL && node != NULL) {
        handlet = iup_xb_parse_handle_new();
        handlet->name = xmlGetProp(node, (xmlChar *)"name");
        handlet->handle = _handle;
    }

    return handlet;
}

static iup_xb_params_t* __iup_xb_handle_param_get(iup_xml_builder_t *builder, iup_xb_elem_t *elem, Ihandle *param, iup_xb_parse_entity_t *cur_entity) {
    iup_xb_params_t *result = NULL;

    if (param != NULL && cur_entity != NULL) {

        __iup_xb_config_handle(builder, param, cur_entity);


        result = __iup_xb_handle_param_create(param, elem->node);
    }

    return result;
}

static iup_xb_handle_t* __iup_xb_handle_child_get(iup_xml_builder_t *builder, iup_xb_elem_t *elem, Ihandle *param, iup_xb_parse_entity_t *cur_entity) {
    iup_xb_handle_t * result = NULL;

    if (param != NULL && cur_entity != NULL) {

        __iup_xb_config_handle(builder, param, cur_entity);

        result = __iup_xb_handle_create(param, elem);

    }

    return result;
}


static iup_xb_attrs_handle_t* __iup_xb_handle_attr_create(Ihandle* param, xmlNodePtr node) {

    iup_xb_attrs_handle_t *result = NULL;

    result = iup_xb_parse_attrs_handle_new();
    
    result->name = xmlGetProp(node, (xmlChar *)"name");
    result->handle = param;

    return result;
}

static iup_xb_attrs_handle_t* __iup_xb_handle_attr_get(iup_xml_builder_t *builder, iup_xb_elem_t *elem, Ihandle *param, iup_xb_parse_entity_t *cur_entity) {
    iup_xb_attrs_handle_t *result = NULL;

    if (param != NULL && cur_entity != NULL) {

        __iup_xb_config_handle(builder, param, cur_entity);

        result = __iup_xb_handle_attr_create(param, elem->node);
    }

    return result;
}

static xmlChar* __iup_xb_examin_value(xmlNodePtr node) {
    xmlChar *result = xmlGetProp(node, (xmlChar *)"value");

    if ( is_value_content((const char*)result))  {
        xmlFree(result);
        result = xmlNodeGetContent(node);
    } 

    return result;
}

static iup_xb_attr_t* __iup_xb_attr_get(iup_xb_elem_t *elem) {
    iup_xb_attr_t *result = NULL;

    xmlNodePtr node = elem->node;
    xmlChar *name = xmlGetProp(node, (xmlChar *)"name");
    xmlChar *value = __iup_xb_examin_value(node);

    if (name != NULL && value != NULL) {

         result = iup_xb_parse_attr_new(name, value);

    } else {
        xmlFree(name);
        xmlFree(value);
    }

    return result;
}

static iup_xb_attr_t* __iup_xb_attr_pair(xmlNodePtr node, const char *_name, const char * val_name) {
    
    iup_xb_attr_t *result = NULL;

    xmlChar *name = xmlGetProp(node, (xmlChar *)_name);

    if (name != NULL) {
        
        xmlChar *value = xmlGetProp(node, (xmlChar *)val_name);

        if (value != NULL) {
            result = iup_xb_parse_attr_new(name, value);
        } else {
            xmlFree(name);
        }
    }

    return result;
}

static iup_xb_handle_lnk_t* __iup_xb_handle_link_get(iup_xb_elem_t *elem) {

    iup_xb_handle_lnk_t *result = NULL;

    xmlNodePtr node = elem->node;

    xmlChar *name = xmlGetProp(node, (xmlChar *)"name");

    if (name != NULL) {
        
        xmlChar *ref = xmlGetProp(node, (xmlChar *)"ref");

        result = __iup_xb_handle_lnk_new(name, ref);
     
    }

    return result;
}

static iup_xb_attr_t* __iup_xb_attrs_get(iup_xb_elem_t *elem) {
    iup_xb_attr_t *result = NULL;

    xmlChar *value = __iup_xb_examin_value(elem->node);

    if (value != NULL) {
        
        result = iup_xb_parse_attr_new(NULL, value);

    }

    return result;
}

static iup_xb_attr_t* __iup_xb_callback_get(iup_xb_elem_t *elem) {
    
    return __iup_xb_attr_pair(elem->node, "name", "event");

}

static void __iup_xb_set_attr_to_list(dl_list_t *list, iup_xb_elem_t *xb_element, iup_xb_attr_t* (*attr_func)(iup_xb_elem_t *xb_element)) {
    iup_xb_attr_t *data = attr_func(xb_element);
                
    if (data) {
        dl_list_append(list, data);
    }
}

static void __iup_xb_set_handle_to_list(dl_list_t *list, iup_xb_elem_t *elem, iup_xb_handle_lnk_t* (*attr_func)(iup_xb_elem_t *xb_element)) {
    
    iup_xb_handle_lnk_t *data = attr_func(elem);
                
    if (data) {
        dl_list_append(list, data);
    }
}

static void _iup_xb_create_set_handle(iup_xml_builder_t *builder, iup_xb_parse_entity_t* cur_entity, iup_xb_elem_t* xb_element, dl_list_t *handle_list,
              IUP_XB_GET_VOID_FUNC get_func) {
    
    Ihandle * p_child = __iup_xb_parse_node(builder, cur_entity, xb_element);

    if (p_child) {
        
        void* child = get_func(builder, xb_element, p_child, cur_entity);

        if (child) dl_list_append(handle_list, child);
    }
}

#if 0
    //################################ NEW PART ##########################################
#endif

void _iup_xb_get_elem_type(iup_xb_elem_t *element, xmlNodePtr node) {

    element->node = node;

    iup_xb_elem_type_t result = IUP_XB_E_TYPE_UNKNOWN; 
    
    const char *tagname = (const char*)node->name;
    const char *sep = strrchr(tagname, '-');
    size_t len_prefix = 0;
    size_t len_suffix = 0;
    size_t sep_offset = 0;

    bool isattr = false;
    bool isparam = false;
    
    if (sep) {

        len_prefix = sep - tagname;

        if ( strncmp(tagname, "attr", len_prefix) == 0 ) {
            isattr = true;
        } else if ( strncmp(tagname, "param", len_prefix) == 0 ) {
            isparam = true;
        }

        sep_offset = 1;

    }

    const char *tag = tagname + len_prefix + sep_offset;

    element->tag = (const unsigned char *)tag;

    len_suffix = strlen((tagname + len_prefix + sep_offset));

    if ( strncmp(tag, "str", len_suffix) == 0 ) {
        if ( isattr ) {
            result = IUP_XB_E_TYPE_ATTR_STR;
        } else if (isparam) {
            result = IUP_XB_E_TYPE_PARAM_STR;
        } else {
            result = IUP_XB_E_TYPE_UNKNOWN;
        }
    } else if ( strncmp(tag, "attr", len_suffix) == 0 ) {
        result = IUP_XB_E_TYPE_ATTR_STR;
    } else if ( strncmp(tag, "null", len_suffix) == 0 ) {
        if (isparam) {
            result = IUP_XB_E_TYPE_PARAM_NULL;
        } else {
            result = IUP_XB_E_TYPE_UNKNOWN;
        }
    } else if ( strncmp(tag, "attrs", len_suffix) == 0 ) {
        result = IUP_XB_E_TYPE_ATTRS_STR;
    } else if ( strncmp(tag, "userdata", len_suffix) == 0 ) {
        result = IUP_XB_E_TYPE_USERDATA;
    } else if ( strncmp(tag, "handle", len_suffix) == 0 ) {
        result = IUP_XB_E_TYPE_HANDLE_LINK;
    } else if ( strncmp(tag, "callback", len_suffix) == 0 ) {
        result = IUP_XB_E_TYPE_CALLBACK;
    } else {
        if ( isattr ) {
            result = IUP_XB_E_TYPE_ATTR_HANDLE;
        } else if (isparam) {
            result = IUP_XB_E_TYPE_PARAM_HANDLE;
        } else {
            result = IUP_XB_E_TYPE_CHILD_HANDLE;
        }
    }

    element->type = result;
}

#if 0
    //################################ EO NEW PART ##########################################
#endif

static Ihandle* __iup_xb_parse_node(iup_xml_builder_t* builder, iup_xb_parse_entity_t *parent_entity, iup_xb_elem_t *element) {
    
    Ihandle *handle = NULL;
    
    xmlNodePtr node = element->node;
    
    if (node && node->type == XML_ELEMENT_NODE) {

        xmlNodePtr curChild = node->children;
        
        iup_xb_parse_entity_t *cur_entity = iup_xb_parse_entity_new(); 
        while(curChild) {

            if (curChild->type != XML_ELEMENT_NODE) {
                curChild = curChild->next;
                continue;
            } 
            
            iup_xb_elem_t *xb_element = malloc(sizeof(iup_xb_elem_t));
            _iup_xb_get_elem_type(xb_element, curChild);

            switch(xb_element->type) {
                case IUP_XB_E_TYPE_ATTR_STR:    __iup_xb_set_attr_to_list(parent_entity->attrs, xb_element, __iup_xb_attr_get); break;
                case IUP_XB_E_TYPE_ATTRS_STR:   __iup_xb_set_attr_to_list(parent_entity->attrs_s, xb_element, __iup_xb_attrs_get);break;
                case IUP_XB_E_TYPE_PARAM_NULL:  { 
                    iup_xb_params_t *result = iup_xb_params_new();
                    result->type = IUP_XB_HANDLE_PARAM;
                    result->value = iup_xb_parse_handle_new();
                    dl_list_append(parent_entity->params, result);
                    break; 
                }
                case IUP_XB_E_TYPE_PARAM_STR:  { 
                    iup_xb_params_t * param = __iup_xb_str_param_get(xb_element);
                    if ( param ) dl_list_append(parent_entity->params, param);
                    break; 
                }
                case IUP_XB_E_TYPE_ATTR_HANDLE: { 
                    _iup_xb_create_set_handle(builder, cur_entity, xb_element, parent_entity->attrs_handles, (IUP_XB_GET_VOID_FUNC)__iup_xb_handle_attr_get);
                    break;
                }
                case IUP_XB_E_TYPE_PARAM_HANDLE: { 
                    _iup_xb_create_set_handle(builder, cur_entity, xb_element, parent_entity->params, (IUP_XB_GET_VOID_FUNC)__iup_xb_handle_param_get);
                    break; 
                }
                case IUP_XB_E_TYPE_CHILD_HANDLE: { 
                    _iup_xb_create_set_handle(builder, cur_entity, xb_element, parent_entity->children, (IUP_XB_GET_VOID_FUNC)__iup_xb_handle_child_get);
                    break; 
                }
                case IUP_XB_E_TYPE_USERDATA: __iup_xb_set_attr_to_list(parent_entity->userdata, xb_element, __iup_xb_attr_get); break;
                case IUP_XB_E_TYPE_HANDLE_LINK: __iup_xb_set_handle_to_list(parent_entity->handlelinks, xb_element, __iup_xb_handle_link_get); break;
                case IUP_XB_E_TYPE_CALLBACK: __iup_xb_set_attr_to_list(parent_entity->callbacks, xb_element, __iup_xb_callback_get);  break;
                case IUP_XB_E_TYPE_UNKNOWN: break;
                default: break;
            }

            iup_xb_parse_entity_reset(cur_entity);
            free(xb_element);
            curChild = curChild->next;
        
        }

        handle = __iup_xb_handle_from_node(builder, element, parent_entity);

        if (!handle) {
            _iup_xb_add_err(builder, "Unknown Iup Ui Element \"%s\"", node->name);
        }

        iup_xb_parse_entity_free(&cur_entity);
    }
    return handle;
}

static void __iup_xb_set_handle_attr(Ihandle *handle, const char *attr_name, void *value) {
    if (handle != NULL && attr_name != NULL) {
        IupSetAttribute(handle, attr_name, value);
    }
}

#if 0
//###################################################################################
//EOF private section
//###################################################################################
#endif

iup_xml_builder_t* iup_xml_builder_new() {
    iup_xml_builder_t *newbuilder = malloc(sizeof(iup_xml_builder_t));

    if (newbuilder) {
        newbuilder->err = dl_list_new();
        newbuilder->xml_res = dl_list_new();
        newbuilder->handlelinks = dl_list_new();
        newbuilder->current_handle_links = dl_list_new();
        newbuilder->parsed = IupUser();
        newbuilder->handles = NULL;
        newbuilder->callbacks = IupUser();
        newbuilder->userdata = IupUser();
        newbuilder->cntparsed = 0;
    }

    return newbuilder;
}

static void __iup_xb_huilder_xml_error_func(void *ctx, const char *msg, ...) {
    iup_xml_builder_t *builder = (iup_xml_builder_t *)ctx;

    va_list vl;
	va_start(vl, msg);
    dl_list_append( builder->err, format_string_va_new(msg, vl));
	va_end(vl);

}

void iup_xml_builder_add_file(iup_xml_builder_t *builder, const char *name, const char* filename) {
    if (builder != NULL  && is_not_blank(name)) {
        xmlSetGenericErrorFunc(builder,__iup_xb_huilder_xml_error_func);
        xmlDocPtr newsrc = xmlReadFile(filename, "UTF-8", 0);
        xmlErrorPtr err = xmlGetLastError();
        if (err == NULL) {
            dl_list_append(builder->xml_res, __iup_xb_xml_res_new(name, newsrc));
        } 
        xmlResetLastError();
    }
}

void iup_xml_builder_add_bytes(iup_xml_builder_t *builder, const char *name, const char * buffer, int size) {
    if (builder != NULL  && is_not_blank(name)) {
        xmlSetGenericErrorFunc(builder, __iup_xb_huilder_xml_error_func);
        xmlDocPtr newsrc = xmlReadMemory(buffer, size, "default", "UTF-8", 0);
        xmlErrorPtr err = xmlGetLastError();
        if (err == NULL) {
            dl_list_append(builder->xml_res, __iup_xb_xml_res_new(name, newsrc));
        } 
        xmlResetLastError();
    }
}

void iup_xml_builder_add_callback(iup_xml_builder_t *builder, const char* clbk_name, Icallback callback) {
    __iup_xb_set_handle_attr(builder->callbacks, clbk_name, (void*)callback);
}

void iup_xml_builder_rem_callback(iup_xml_builder_t *builder, const char* clbk_name) {
    __iup_xb_set_handle_attr(builder->callbacks, clbk_name, NULL);
}
void iup_xml_builder_add_user_data(iup_xml_builder_t *builder, const char* data_name, void *data) {
    __iup_xb_set_handle_attr(builder->userdata, data_name, data);
}

void iup_xml_builder_rem_user_data(iup_xml_builder_t *builder, const char* data_name) {
    __iup_xb_set_handle_attr(builder->userdata, data_name, NULL);
}

static void __iup_xb_link_handler_single_global(void **data, void * _params) {

    iup_xb_handle_lnk_t * handlelink = (iup_xb_handle_lnk_t *)*data;

    if ( handlelink->ref != NULL) {

        Ihandle *parsed_res = (Ihandle *)_params;

        Ihandle *res = (Ihandle*)IupGetAttribute(parsed_res, (const char *)handlelink->ref);

        if (res) {
            
            Ihandle *localhandles = (Ihandle*)IupGetAttribute(res, "handles");

            Ihandle *src = (Ihandle*)IupGetAttribute(localhandles, (const char*)handlelink->name);

            if ( src != NULL ) {
                
                IupSetAttribute(handlelink->target, (const char*)handlelink->name, (void*)src);
            }

        }

    }
    
}

static void __iup_xb_link_handler_single_local(void **data, void * _params) {

    iup_xb_handle_lnk_t * handlelink = (iup_xb_handle_lnk_t *)*data;

    if ( handlelink->ref == NULL ) {    
        
        void **params = (void**)_params;

        Ihandle *localhandles = (Ihandle *)params[0];
        Ihandle *parent = (Ihandle *)params[1];

        Ihandle *src = (Ihandle*)IupGetAttribute(localhandles, (const char*)handlelink->name);

        if ( src != NULL && handlelink->parent == parent ) {
            IupSetAttribute(handlelink->target, (const char*)handlelink->name, (void*)src);
        }

    }

}

static void __iup_xb_link_handler_global(iup_xml_builder_t *builder) {

    dl_list_each_data(builder->handlelinks, (void*)builder->parsed, __iup_xb_link_handler_single_global);

}

static void __iup_xb_link_handler_local(dl_list_t *handlelinks, Ihandle *localhandles, Ihandle *parent) {

    void *param[2] = {localhandles , parent};

    dl_list_each_data(handlelinks, (void*)param, __iup_xb_link_handler_single_local);

}

static void __iup_xb_config_handlelinks_parent(void **data, void * _params) {

    iup_xb_handle_lnk_t * handlelink = (iup_xb_handle_lnk_t *)*data;

    handlelink->parent = (Ihandle*)_params;

}

static Ihandle * __iup_xb_parse_xml_res(iup_xml_builder_t *builder, iup_xml_resource_t* res) {
        
    xmlDocPtr curxml = res->doc;

    Ihandle *result = NULL;

    if ( curxml ) {
            
        builder->handles = IupUser();

        iup_xb_parse_entity_t *cur_entity = iup_xb_parse_entity_new(); 

        iup_xb_elem_t *xb_element = malloc(sizeof(iup_xb_elem_t));
        _iup_xb_get_elem_type(xb_element, xmlDocGetRootElement(curxml));

        Ihandle *result_handle = __iup_xb_parse_node(builder, cur_entity, xb_element);

        __iup_xb_config_handle(builder, result_handle, cur_entity);

        if (result_handle) {
            
            result = IupUser();

            IupSetAttribute(result, "res", (void*)res);
            IupSetAttribute(result, "handle", (void*)result_handle);
            IupSetAttribute(result, "handles",(void*)builder->handles);
            
            dl_list_each_data(builder->current_handle_links, (void*)result_handle, __iup_xb_config_handlelinks_parent);

            __iup_xb_link_handler_local(builder->current_handle_links, builder->handles, result_handle);

            dl_list_clear(builder->current_handle_links);
        }
        
        iup_xb_parse_entity_free(&cur_entity);
        free(xb_element);
	}

    return result;
}

void iup_xml_builder_parse(iup_xml_builder_t *builder) {

    if (builder == NULL) return;

    dl_list_item_t* cur_node = builder->xml_res->first;
	
    while(cur_node != NULL) {

        iup_xml_resource_t* newres = (iup_xml_resource_t*)cur_node->data;
        
        Ihandle * result_handle = __iup_xb_parse_xml_res(builder, newres);

        if (result_handle) {

            IupSetAttribute(builder->parsed, newres->name, (void*)result_handle);

            ++builder->cntparsed;

        }

		cur_node = cur_node->next;
	}

    __iup_xb_link_handler_global(builder);

}

Ihandle* iup_xml_builder_get_result(iup_xml_builder_t *builder, const char *name) {

    return  (Ihandle*)IupGetAttribute(builder->parsed, name);

}

Ihandle* iup_xml_builder_get_result_new(iup_xml_builder_t *builder, const char *name) {
    
    Ihandle *result = NULL;

    if (builder != NULL  && is_not_blank(name)) {

        Ihandle *entity = (Ihandle*)IupGetAttribute(builder->parsed, name);

        iup_xml_resource_t* res = (iup_xml_resource_t*)IupGetAttribute(entity, "res");

        result = __iup_xb_parse_xml_res(builder, res);

    }

    return result;
}

void iup_xml_builder_free_result(Ihandle **result) {
    if (result != NULL && *result != NULL) {
        Ihandle *to_delete = *result;
        IupDestroy((Ihandle*)IupGetAttribute(to_delete, "handle"));
        IupDestroy(to_delete);
        *result = NULL;
    }
}

Ihandle* iup_xml_builder_get_main(Ihandle *result_handle) {
    
    Ihandle *result = NULL;
    
    if (result_handle != NULL) {

        result = (Ihandle*)IupGetAttribute(result_handle, "handle");
    }

    return result;

}

Ihandle* iup_xml_builder_get_name(Ihandle *result_handle, const char *name) {

    Ihandle *result = NULL;

    if (result_handle != NULL  && is_not_blank(name)) {

        Ihandle* handles = (Ihandle*)IupGetAttribute(result_handle, "handles");

        result = (Ihandle*)IupGetAttribute(handles, name);

    }

    return result;
}

void iup_xml_builder_free(iup_xml_builder_t **builder) {
    if (builder != NULL && *builder != NULL) {
        iup_xml_builder_t *to_delete = *builder;

        __iup_xml_builder_xml_res_free(to_delete);
        __iup_xml_builder_err_free(to_delete);

        __iup_xb_clear_list(to_delete->handlelinks, iup_xb_delete_handle_lnk);
        __iup_xb_clear_list(to_delete->current_handle_links, iup_xb_delete_handle_lnk);

        dl_list_free(&to_delete->handlelinks);
        dl_list_free(&to_delete->current_handle_links);

        IupDestroy(to_delete->callbacks);
        IupDestroy(to_delete->userdata);
        IupDestroy(to_delete->handles);

        char **names = malloc( to_delete->cntparsed * sizeof(char *) );

        int cntnames = IupGetAllAttributes(to_delete->parsed, names, to_delete->cntparsed);

        for (int handle = 0; handle < cntnames; ++handle) {
            
            DEBUG_LOG_ARGS("remove: %s\n", names[handle]);

            Ihandle *entity = (Ihandle*)IupGetAttribute(to_delete->parsed, names[handle] );
            
            iup_xml_builder_free_result(&entity);
        }

        free(names);
        IupDestroy(to_delete->parsed);

        free(to_delete);
        *builder = NULL;
    }
}
