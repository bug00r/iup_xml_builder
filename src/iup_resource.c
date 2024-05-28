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

static IupResource* __iup_res_new() {
    IupResource *new_res = malloc(sizeof(IupResource));
    return new_res;
}

/*
    ############ PUBLIC #############
*/

IupResource* iup_resource_new() {
    IupResource* new_res = __iup_res_new();
    iup_resource_init(new_res);
    return new_res;
}

IupResource* iup_resource_new_default() {
    IupResource* new_res = __iup_res_new();
    iup_resource_init_default(new_res);
    return new_res;
}

void iup_resource_free(IupResource** resource) {  
	if ( resource != NULL && *resource != NULL ) {
		IupResource *to_delete = *resource;
		
		iup_resource_cleanup(to_delete);

		free(to_delete);
		*resource = NULL;
	}
}


void iup_resource_init(IupResource* resource) {
    if (resource) {
        resource->_base.errors = NULL;
        resource->init_params = NULL;
        resource->res_obj = NULL;
        resource->init = NULL;
        resource->get = NULL;
        resource->cleanup = NULL;
    }
}

void iup_resource_init_default(IupResource* resource) {
    if (resource) {
        resource->_base.errors = dl_list_new();
        resource->init_params = NULL;
        resource->res_obj = NULL;
        resource->init = __iup_res_init_default;
        resource->get = __iup_res_get_default;
        resource->cleanup = __iup_res_cleanup_default;
    }
}

void iup_resource_cleanup(IupResource* resource) {
    if (resource) {
        dl_list_free(&resource->_base.errors);
        iup_resource_init(resource);
    }
}

void iup_resource_add_error(IupResource* resource, const unsigned char* errormessage) {
    if (resource && resource->_base.errors && errormessage) {
        dl_list_append(resource->_base.errors, (void*) errormessage);
    }
}