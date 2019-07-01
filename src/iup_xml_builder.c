#include "iup_xml_builder.h"

typedef struct {
    dl_list_t *params;
    dl_list_t *attrs;
    dl_list_t *children;
    dl_list_t *callbacks;
} iup_xb_parse_entity_t;

typedef struct {
    xmlChar *name;
    xmlChar *value;
} iup_xb_attr_t;

typedef struct {
    xmlChar *name;
    Ihandle *handle;
} iup_xb_handle_t;

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

static iup_xb_parse_entity_t *iup_xb_parse_entity_new() {
    iup_xb_parse_entity_t *newentity = malloc(sizeof(iup_xb_parse_entity_t));

    if(newentity) {
        newentity->params = dl_list_new();
        newentity->attrs = dl_list_new();
        newentity->children = dl_list_new();
        newentity->callbacks = dl_list_new();
    }

    return newentity;
}

static iup_xb_attr_t *iup_xb_parse_attr_new() {
    iup_xb_attr_t* newattr = malloc(sizeof(iup_xb_attr_t));

    if(newattr) {
        newattr->name = NULL;
        newattr->value = NULL;
    }

    return newattr;
}

static iup_xb_handle_t *iup_xb_parse_handle_new() {
    iup_xb_handle_t* newhandle = malloc(sizeof(iup_xb_handle_t));

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

static void iup_xb_delete_attr(void **data) {
    if (data != NULL && *data != NULL) {
        iup_xb_attr_t *attr = (iup_xb_attr_t *)*data;
        xmlFree(attr->name);
        xmlFree(attr->value);
        free(attr);
        *data = NULL;
    }
}

static void iup_xb_parse_entity_reset(iup_xb_parse_entity_t *entity) {
    if (entity != NULL ) {
        iup_xb_parse_entity_t *to_reset = entity;

        //delete params
        dl_list_each(to_reset->params, iup_xb_delete_params);
        dl_list_clear(to_reset->params);
        //delete attr
        dl_list_each(to_reset->attrs, iup_xb_delete_attr);
        dl_list_clear(to_reset->attrs);
        //delete callbacks
        dl_list_each(to_reset->callbacks, iup_xb_delete_attr);
        dl_list_clear(to_reset->callbacks);
        //delete children
        dl_list_each(to_reset->children, iup_xb_delete_handle);
        dl_list_clear(to_reset->children);

    }
}

static void iup_xb_parse_entity_free(iup_xb_parse_entity_t **entity) {
    if (entity != NULL && *entity != NULL) {
        iup_xb_parse_entity_t *to_delete = *entity;

        iup_xb_parse_entity_reset(to_delete);

        dl_list_free(&to_delete->attrs);
        dl_list_free(&to_delete->params);
        dl_list_free(&to_delete->children);
        dl_list_free(&to_delete->callbacks);
        
        free(to_delete);
        *entity = NULL;
    }
}


static void __iup_xml_builder_xml_res_free(iup_xml_builder_t *builder) {
    
    dl_list_item_t* cur_node = builder->xml_res->first;

	while(cur_node != NULL)   {
		if ( cur_node->data != NULL ) {
			xmlFreeDoc((xmlDocPtr)cur_node->data);
		}
		cur_node = cur_node->next;
	}

    dl_list_free(&builder->xml_res);
}

static void __iup_xml_builder_err_free(iup_xml_builder_t *builder) {
    //todo free items if need
    //..
    //free dl only
    dl_list_free(&builder->err);
}

static bool is_attribute(const char *text) {
    return (strcmp(text, "attr") == 0);
}

static bool is_callback(const char *text) {
    return (strcmp(text, "callback") == 0);
}

static bool is_params(xmlNodePtr node) {

    return (xmlHasProp(node, (xmlChar*)"param") != NULL);

}

static bool is_params_string(const char *text) {
    return (strcmp(text, "string") == 0);
}

static bool is_value_content(const char *value) {
    return (strcmp(value, ":c") == 0);
}

static void __iup_xb_config_handle(iup_xml_builder_t *builder, Ihandle * _handle, iup_xb_parse_entity_t *conf_entity) {
    
    if(_handle != NULL && conf_entity != NULL) {

        dl_list_item_t* cur_node = conf_entity->children->first;

        while(cur_node != NULL) {
            iup_xb_handle_t * handle = (iup_xb_handle_t *)cur_node->data;
            
            IupAppend(_handle, handle->handle);

            if(handle->name != NULL) {

                DEBUG_LOG_ARGS("handle with name %s = %p\n", handle->name, handle->handle);

                IupSetStrAttribute(builder->handles, handle->name, (void*)handle->handle);
            }

            cur_node = cur_node->next;
        }

        cur_node = conf_entity->attrs->first;

        while(cur_node != NULL) {
            
            iup_xb_attr_t * attr = (iup_xb_attr_t *)cur_node->data;

            DEBUG_LOG_ARGS("set attr %s = %s\n", attr->name, attr->value);

            IupSetStrAttribute(_handle, attr->name, attr->value);

            cur_node = cur_node->next;
        }

        cur_node = conf_entity->callbacks->first;

        while(cur_node != NULL) {
            
            iup_xb_attr_t * callback = (iup_xb_attr_t *)cur_node->data;

            DEBUG_LOG_ARGS("set callback %s = %s\n", callback->name, callback->value);

            IupSetCallback(_handle, callback->value, (Icallback)IupGetAttribute(builder->callbacks, callback->name));

            cur_node = cur_node->next;
        }

    }
}

static Ihandle* __iup_xb_handle_from_node(iup_xml_builder_t *builder, xmlNodePtr node, iup_xb_parse_entity_t *conf_entity) {
    Ihandle *result = NULL;

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

                    DEBUG_LOG_ARGS("param %i = %p\n", cnt, handle_param->handle);

                    if (handle_param->name != NULL) {
                        
                        DEBUG_LOG_ARGS("handle with name %s = %p\n", handle_param->name, handle_param->handle);

                        handle_w_name[cnt_handle++] = handle_param;
                    } 

                } else {
                    
                    DEBUG_LOG_ARGS("param %i = %s\n", cnt, param->str_value);

                    params[cnt++] = (void*)param->str_value;
                }

                cur_node = cur_node->next;
            
            }

        }

        DEBUG_LOG_ARGS("create class: %s\n", node->name);

        if (cnt == 1) {
            result = IupCreatep(node->name, (void*)params[0]);
        } else if (cnt > 1) {
            result = IupCreatev(node->name, (void**)&params);
        } else if (cnt == 0) {
            result = IupCreate(node->name);
        }
        
        DEBUG_LOG_ARGS("class done: %p\n", result);

        if (cnt > 0) {

            for(uint32_t handl = 0; handl < cnt_handle ; ++handl) {

                iup_xb_handle_t * curhandle = handle_w_name[handl];

                DEBUG_LOG_ARGS("set named handle: %s\n", curhandle->name);

                IupSetStrAttribute(builder->handles, curhandle->name, (void*)curhandle->handle);
            
            }
        }

        free(handle_w_name);
        free(params);
    }

    return result;
}

static iup_xb_params_t* __iup_xb_str_param_get(xmlNodePtr node) {
    iup_xb_params_t *result = NULL;
        
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
    
    result->value->name = xmlGetProp(node, (xmlChar *)"name");
    result->value->handle = param;

    return result;
}

static iup_xb_handle_t* __iup_xb_handle_create(Ihandle *_handle, xmlNodePtr node) {
    iup_xb_handle_t* handlet = NULL;

    if (_handle != NULL && node != NULL) {
        handlet = iup_xb_parse_handle_new();
        handlet->name = xmlGetProp(node, (xmlChar *)"name");
        handlet->handle = _handle;
    }

    return handlet;
}

static iup_xb_params_t* __iup_xb_handle_param_get(iup_xml_builder_t *builder, xmlNodePtr node, Ihandle *param, iup_xb_parse_entity_t *cur_entity) {
    iup_xb_params_t *result = NULL;

    if (param != NULL && cur_entity != NULL) {
        
        DEBUG_LOG("CONFIG_HANDLE\n");

        __iup_xb_config_handle(builder, param, cur_entity);

        result = __iup_xb_handle_param_create(param, node);
    }

    return result;
}

static iup_xb_attr_t* __iup_xb_attr_get(xmlNodePtr node) {
    iup_xb_attr_t *result = NULL;

    xmlChar *name = xmlGetProp(node, (xmlChar *)"name");

    if (name != NULL) {
        
        xmlChar *value = xmlGetProp(node, (xmlChar *)"value");

        if (is_value_content((const char*)value))  {
            xmlFree(value);
            value = xmlNodeGetContent(node);
        } 

        if (value != NULL) {
            result = iup_xb_parse_attr_new();
            result->name = name;
            result->value = value;
        }
    }

    return result;
}

static iup_xb_attr_t* __iup_xb_callback_get(xmlNodePtr node) {
    
    iup_xb_attr_t *result = NULL;

    xmlChar *name = xmlGetProp(node, (xmlChar *)"name");

    if (name != NULL) {
        
        xmlChar *value = xmlGetProp(node, (xmlChar *)"event");

        if (value != NULL) {
            result = iup_xb_parse_attr_new();
            result->name = name;
            result->value = value;
        }
    }

    return result;
}

static Ihandle* __iup_xb_parse_node(iup_xml_builder_t* builder, iup_xb_parse_entity_t *parent_entity, xmlNodePtr node) {
    Ihandle *handle = NULL;
    if (node && node->type == XML_ELEMENT_NODE) {
        
        DEBUG_LOG_ARGS("BEGIN: parent: %s node: %s\n", (node->parent != NULL ? node->parent->name : NULL),  node->name );
        
        xmlNodePtr curChild = node->children;
        
        iup_xb_parse_entity_t *cur_entity = iup_xb_parse_entity_new(); 
        while(curChild && curChild != node->last ) {
            
            if (curChild->type != XML_ELEMENT_NODE) {
                curChild = curChild->next;
                continue;
            } 

            //if(is_params(node->name)) {
            if (is_params(curChild)) {
                
                DEBUG_LOG("param found:");

                iup_xb_params_t * param = NULL;
                //is parameter
                if(is_params_string(curChild->name)) {
                    param = __iup_xb_str_param_get(curChild);
                    DEBUG_LOG_ARGS("is string = %s \n", param->str_value);
                } else {
                    //is handle
                    DEBUG_LOG("is handle: \n");

                    Ihandle * p_child = __iup_xb_parse_node(builder, cur_entity, curChild);

                    DEBUG_LOG_ARGS("handle obj: %p\n", p_child);

                    param = __iup_xb_handle_param_get(builder, curChild, p_child, cur_entity);

                    DEBUG_LOG_ARGS("handle = %p name = \n", param->value->handle, param->value->name);
                }

                dl_list_append(parent_entity->params, param);

            } else if(is_attribute(curChild->name)) {
                //is attribute
                iup_xb_attr_t *attr = __iup_xb_attr_get(curChild);
                dl_list_append(parent_entity->attrs, attr);

            } else if (is_callback(curChild->name)) {
                iup_xb_attr_t *callback = __iup_xb_callback_get(curChild);
                dl_list_append(parent_entity->callbacks, callback);
            } else {
                //is children
                Ihandle * child_handle = __iup_xb_parse_node(builder, cur_entity, curChild);
                
                DEBUG_LOG("CONFIG_HANDLE\n");
                
                __iup_xb_config_handle(builder, child_handle, cur_entity);
                
                iup_xb_handle_t* child = __iup_xb_handle_create(child_handle, curChild);
                
                dl_list_append(parent_entity->children, child);
            }

            iup_xb_parse_entity_reset(cur_entity);

            curChild = curChild->next;
        
        }

        DEBUG_LOG_ARGS("END: parent: %s node: %s\n", (node->parent != NULL ? node->parent->name : NULL),  node->name );

        handle = __iup_xb_handle_from_node(builder, node, parent_entity);

        DEBUG_LOG("CONFIG_HANDLE\n");

        //__iup_xb_config_handle(handle, parent_entity);

        iup_xb_parse_entity_free(&cur_entity);
    }
    return handle;
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
        newbuilder->handles = IupUser();
        newbuilder->callbacks = IupUser();
    }

    return newbuilder;
}

void iup_xml_builder_add_file(iup_xml_builder_t *builder, const char* filename) {
    xmlDocPtr newsrc = xmlReadFile(filename, "UTF-8", 0);
    dl_list_append(builder->xml_res, newsrc);
}

void iup_xml_builder_add_bytes(iup_xml_builder_t *builder, const char * buffer, int size) {
    xmlDocPtr newsrc = xmlReadMemory(buffer, size, "default", "UTF-8", 0);
    dl_list_append(builder->xml_res, newsrc);
}

void iup_xml_builder_add_callback(iup_xml_builder_t *builder, const char* clbk_name, Icallback callback) {
    if (builder != NULL && clbk_name != NULL && callback != NULL) {
        IupSetAttribute(builder->callbacks, clbk_name, (void*)callback);
    }
}

void iup_xml_builder_rem_callback(iup_xml_builder_t *builder, const char* clbk_name) {
    if (builder != NULL && clbk_name != NULL ) {
        IupSetAttribute(builder->callbacks, clbk_name, NULL);
    }
}

Ihandle *iup_xml_builder_parse(iup_xml_builder_t *builder) {

    dl_list_item_t* cur_node = builder->xml_res->first;

    Ihandle *result = NULL;
	
    while(cur_node != NULL) {

        xmlDocPtr curxml = (xmlDocPtr)cur_node->data;
		
        if ( curxml ) {
            
            iup_xb_parse_entity_t *cur_entity = iup_xb_parse_entity_new(); 

            result = __iup_xb_parse_node(builder, cur_entity, xmlDocGetRootElement(curxml));

            __iup_xb_config_handle(builder, result, cur_entity);

            iup_xb_parse_entity_free(&cur_entity);
		}
		cur_node = cur_node->next;
	}

    return result;

}

void iup_xml_builder_free(iup_xml_builder_t **builder) {
    if (builder != NULL && *builder != NULL) {
        iup_xml_builder_t *to_delete = *builder;

        __iup_xml_builder_xml_res_free(to_delete);
        __iup_xml_builder_err_free(to_delete);

        IupDestroy(to_delete->handles);
        IupDestroy(to_delete->callbacks);

        free(to_delete);
        *builder = NULL;
    }
}

void iup_xml_builder_perr(iup_xml_builder_t*builder) {

}