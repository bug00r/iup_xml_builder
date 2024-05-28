#ifndef IUP_RESOURCE_H
#define IUP_RESOURCE_H

#include <stdbool.h>

#include "defs.h"
#include "dl_list.h"

/*
    The Iup Resource Base is a collection of needed basic data.

    IMPORTANT:
    The errors Container only holds the given messages but not free them.
    If they are no binary based text constants you can handle it like this:

    Example:

    static void _cleanup_error(void **data) {
	if (data && *data) {
		free(*data);
		*data = NULL;
	}

    dl_list_each(resource->_base.errors, _cleanup_error);
    

*/
typedef struct _IupResourceBase {
    /*
        A List with all occured errors.
    */
    DlList *errors;
} IupResourceBase;

typedef struct _IupResource {
    /*  
        should be available like this, otherwise you will got segfaults.
    */
    IupResourceBase _base; 
    /* 
        This is the created resource Object and the return value of the init method.
        You have to mangage the memory by yourself and should be free'd in cleanup.
    */
    void* res_obj;
    /* 
        These are the needed init params for the resource.
        You have to mangage the memory by yourself and should be free'd in cleanup.
    */
    void* init_params;
    /* 
        This Functions initialized the Resource Object with the given init Parameter. 
        This will be called during "builder_add_resource".
    */
    void (*init)   (struct _IupResource* resource, void* init_params);
    /*  
        This is the cleanup method for housekeeping memory.
    */
    void (*cleanup)(struct _IupResource* resource);
    /* 
        This function took a resource name and return the spcific resource as 
        implemented in get, usually multiply types.
    */
    void* (*get)    (struct _IupResource* resource, unsigned char* res_name);
} IupResource;

/*
    This function allocates memory and calls iup_resource_init.
*/
IupResource* iup_resource_new();

/*
    This function allocates memory and calls iup_resource_init_default.
*/
IupResource* iup_resource_new_default();

/*
    Frees all memory which was allocated with new and init.

    THIS will not free the allocated memory for the error messages. This has to be done by yourself.

*/
void iup_resource_free(IupResource** resource);

/*
    This function inits resource object. You should use it to be sure of undefined behaviour
    prevention.

    Intern all parameter will be set to NULL values.
*/
void iup_resource_init(IupResource* resource);

/*
    This function inits resource object. You should use it to be sure of undefined behaviour
    prevention.

    Intern all parameter will be set to NULL values. The Functions will be set with the intern
    default functions and the error Container will be initialized.
*/
void iup_resource_init_default(IupResource* resource);

/*
    This function cleans all initialized resource.

    THIS is NOT CALLING resource->cleanup(...) and should only be called after the resource cleaning.
*/
void iup_resource_cleanup(IupResource* resource);

/*
    this function adds an error message to resource object for later usage.
    This is only a collection container and you have to clean up message
    memory by yourself.
*/
void iup_resource_add_error(IupResource* resource, const unsigned char* errormessage);

#endif