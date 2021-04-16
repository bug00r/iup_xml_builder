#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>

#include "defs.h"

#include "iup_resource.h"

static void test_iup_resource_alloc_free() {
    DEBUG_LOG(">>>\n");

    iup_resource_t *resource = iup_resource_new();

    assert(resource != NULL);
    assert(resource->_base.errors == NULL);
    assert(resource->init_params == NULL);
    assert(resource->res_obj == NULL);
    assert(resource->init == NULL);
    assert(resource->cleanup == NULL);
    assert(resource->get == NULL);

    iup_resource_free(&resource);
    assert(resource == NULL);

    resource = iup_resource_new_default();

    assert(resource != NULL);
    assert(resource->_base.errors != NULL);
    assert(resource->init_params == NULL);
    assert(resource->res_obj == NULL);
    assert(resource->init != NULL);
    assert(resource->cleanup != NULL);
    assert(resource->get != NULL);

    iup_resource_free(&resource);
    assert(resource == NULL);

    DEBUG_LOG("<<<\n");
}

static void test_iup_resource_init() {
    DEBUG_LOG(">>> \n");

    iup_resource_t resource;

    iup_resource_init(&resource);

    assert(resource._base.errors == NULL);
    assert(resource.init_params == NULL);
    assert(resource.res_obj == NULL);
    assert(resource.init == NULL);
    assert(resource.cleanup == NULL);
    assert(resource.get == NULL);

    iup_resource_init_default(&resource);

    assert(resource._base.errors != NULL);
    assert(resource.init_params == NULL);
    assert(resource.res_obj == NULL);
    assert(resource.init != NULL);
    assert(resource.cleanup != NULL);
    assert(resource.get != NULL);

    iup_resource_cleanup(&resource);
    
    assert(resource._base.errors == NULL);
    assert(resource.init_params == NULL);
    assert(resource.res_obj == NULL);
    assert(resource.init == NULL);
    assert(resource.cleanup == NULL);
    assert(resource.get == NULL);

    DEBUG_LOG("<<<\n");
}

static void test_iup_resource_add_error_stack() {
    
    DEBUG_LOG(">>> \n");

    iup_resource_t resource;
    iup_resource_init_default(&resource);

    iup_resource_add_error(&resource, (const unsigned char*)"error1");
    iup_resource_add_error(&resource, (const unsigned char*)"error2");
    iup_resource_add_error(&resource, (const unsigned char*)"error3");

    assert(resource._base.errors->cnt == 3);

    iup_resource_cleanup(&resource);
    assert(resource._base.errors == NULL);


    DEBUG_LOG("<<<\n");
}

static void _cleanup_error(void **data) {
	if (data && *data) {
		free(*data);
		*data = NULL;
	}
}

static void test_iup_resource_add_error_heap() {
    
    DEBUG_LOG(">>> \n");

    iup_resource_t *resource = iup_resource_new_default();

    iup_resource_add_error(resource, (const unsigned char*)"error1");
    iup_resource_add_error(resource, (const unsigned char*)"error2");
    iup_resource_add_error(resource, (const unsigned char*)"error3");

    assert(resource->_base.errors->cnt == 3);

    iup_resource_free(&resource);

    assert(resource == NULL);

    const unsigned char* base_txt = (const unsigned char*)"This is a Test";
    size_t txt_size = (strlen((const char*)base_txt) + 1) * sizeof(unsigned char*);
    unsigned char* copy_txt = malloc(txt_size);
    memcpy( copy_txt, base_txt, txt_size);

    resource = iup_resource_new_default();

    iup_resource_add_error(resource, (const unsigned char*)copy_txt);

    assert(resource->_base.errors->cnt == 1);

    dl_list_each(resource->_base.errors, _cleanup_error);
    iup_resource_free(&resource);

    assert(resource == NULL);

    DEBUG_LOG("<<<\n");
}

int main(int argc, char **argv) {
    UNUSED(argc);
    UNUSED(argv);

	DEBUG_LOG(">> Start xml builder tests:\n");

    test_iup_resource_alloc_free();

    test_iup_resource_init();

    test_iup_resource_add_error_stack();
    test_iup_resource_add_error_heap();

	DEBUG_LOG("<< end builder tests:\n");

	return 0;
}