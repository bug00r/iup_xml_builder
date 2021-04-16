#include "iup_resource.h"

static void __iup_res_init_default(struct _IupResource* resource, void* init_params) {
    UNUSED(resource);
    UNUSED(init_params);
}
static void __iup_res_cleanup_default(struct _IupResource* resource) {
    UNUSED(resource);
}
static void* __iup_res_get_default(struct _IupResource* resource,  unsigned char* res_name) { 
    UNUSED(resource);
    UNUSED(res_name);
    return NULL; 
}

static iup_resource_t* __iup_res_new() {
    iup_resource_t *new_res = malloc(sizeof(iup_resource_t));
    return new_res;
}

/*
    ############ PUBLIC #############
*/

iup_resource_t* iup_resource_new() {
    iup_resource_t* new_res = __iup_res_new();
    iup_resource_init(new_res);
    return new_res;
}

iup_resource_t* iup_resource_new_default() {
    iup_resource_t* new_res = __iup_res_new();
    iup_resource_init_default(new_res);
    return new_res;
}

void iup_resource_free(iup_resource_t** resource) {  
	if ( resource != NULL && *resource != NULL ) {
		iup_resource_t *to_delete = *resource;
		
		iup_resource_cleanup(to_delete);

		free(to_delete);
		*resource = NULL;
	}
}


void iup_resource_init(iup_resource_t* resource) {
    if (resource) {
        resource->_base.errors = NULL;
        resource->init_params = NULL;
        resource->res_obj = NULL;
        resource->init = NULL;
        resource->get = NULL;
        resource->cleanup = NULL;
    }
}

void iup_resource_init_default(iup_resource_t* resource) {
    if (resource) {
        resource->_base.errors = dl_list_new();
        resource->init_params = NULL;
        resource->res_obj = NULL;
        resource->init = __iup_res_init_default;
        resource->get = __iup_res_get_default;
        resource->cleanup = __iup_res_cleanup_default;
    }
}

void iup_resource_cleanup(iup_resource_t* resource) {
    if (resource) {
        dl_list_free(&resource->_base.errors);
        iup_resource_init(resource);
    }
}

void iup_resource_add_error(iup_resource_t* resource, const unsigned char* errormessage) {
    if (resource && resource->_base.errors && errormessage) {
        dl_list_append(resource->_base.errors, (void*) errormessage);
    }
}