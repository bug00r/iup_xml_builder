#include "iup_xml_builder.h"

typedef struct {
    dl_list_t *params;
    dl_list_t *attrs;
    dl_list_t *children;
} iup_xb_parse_entity_t;

static iup_xb_parse_entity_t *iup_xb_parse_entity_new() {
    iup_xb_parse_entity_t *newentity;

    if(newentity) {
        newentity->params = dl_list_new();
        newentity->attrs = dl_list_new();
        newentity->children = dl_list_new();
    }

    return newentity;
}

static void iup_xb_parse_entity_free(iup_xb_parse_entity_t **entity) {
    if (entity != NULL && *entity != NULL) {
        iup_xb_parse_entity_t *to_delete = *entity;

        //delete params
            //can be strings, ints or handles 
        //delete attr
            //can only be text
        //delete children
            //can be handles

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

    dl_list_free(&builder->err);
}

static void __iup_xml_builder_err_free(iup_xml_builder_t*builder) {
    //todo free items if need
    //..
    //free dl only
    dl_list_free(&builder->err);
}

static Ihandle* __iup_xb_parse_node(iup_xb_parse_entity_t *parent_entity, xmlNodePtr node) {
    Ihandle *handle = NULL;
    if (node && node->type == XML_ELEMENT_NODE) {
        
        DEBUG_LOG_ARGS("parent: %s node: %s\n", (node->parent != NULL ? node->parent->name : NULL),  node->name );
        
        xmlNodePtr curChild = node->children;
        
        iup_xb_parse_entity_t *cur_entity = iup_xb_parse_entity_new(); 
        while(curChild && curChild != node->last) {
        
            Ihandle * child = __iup_xb_parse_node(cur_entity, curChild);
        
            curChild = curChild->next;
        
        }

        //creating handle
        //...
        //EOF handle creation

        iup_xb_parse_entity_free(&cur_entity);
    }
    return handle;
}

iup_xml_builder_t* iup_xml_builder_new() {
    iup_xml_builder_t *newbuilder = malloc(sizeof(iup_xml_builder_t));

    if (newbuilder) {
        newbuilder->err = dl_list_new();
        newbuilder->xml_res = dl_list_new();
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

Ihandle *iup_xml_builder_parse(iup_xml_builder_t *builder) {

    dl_list_item_t* cur_node = builder->xml_res->first;

    Ihandle *result = NULL;
	while(cur_node != NULL) {
        xmlDocPtr curxml = (xmlDocPtr)cur_node->data;
		if ( curxml ) {
            
            iup_xb_parse_entity_t *cur_entity = iup_xb_parse_entity_new(); 

            result = __iup_xb_parse_node(cur_entity, xmlDocGetRootElement(curxml));

            //creating handle
            //...
            //EOF handle creation

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
        free(to_delete);
        *builder = NULL;
    }
}

void iup_xml_builder_perr(iup_xml_builder_t*builder) {

}