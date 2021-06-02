#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <assert.h>
#include <string.h>

void* load_library(void);

/* declare pointers to functions */
int (*establish_connection) (const char * group_id, const char * secret);
int (*put_value)(char * key, char * value);
int (*get_value)(char * key, char ** value);
int (*delete_value)(char * key);
int (*register_callback)(char * key, void (*callback_function)(char *));
int (*close_connection)(void);

int main(){
	int a;
	char line[100];
	char library_name[100];

	void* handle = load_library();

    assert(establish_connection("a", "b") == 0);

    char* val;
    put_value("teste1", "teste2");

	for(int i = 0; i<1000; i++)
	{
		get_value("teste1", &val);
		assert(strcmp(val, "teste2") == 0);
		free(val);
	}

	dlclose(handle);
	
	exit(0);
}

void* load_library(void)
{
    void *handle;
	char *error;

	/* load library from name library_name */
	handle = dlopen("../../KVS-lib/KVS-lib.so", RTLD_LAZY);
	if(!handle)
	{
		fputs(dlerror(), stderr);
		exit(1);
	}

	establish_connection = dlsym(handle, "establish_connection");
	if ((error = dlerror()) != NULL)  {
		fputs(error, stderr);
		exit(1);
	}

	put_value = dlsym(handle, "put_value");
	if ((error = dlerror()) != NULL)  {
		fputs(error, stderr);
		exit(1);
	}
    get_value = dlsym(handle, "get_value");
	if ((error = dlerror()) != NULL)  {
		fputs(error, stderr);
		exit(1);
	}

	delete_value = dlsym(handle, "delete_value");
	if ((error = dlerror()) != NULL)  {
		fputs(error, stderr);
		exit(1);
	}
    register_callback = dlsym(handle, "register_callback");
	if ((error = dlerror()) != NULL)  {
		fputs(error, stderr);
		exit(1);
	}

	close_connection = dlsym(handle, "close_connection");
	if ((error = dlerror()) != NULL)  {
		fputs(error, stderr);
		exit(1);
	}
    

    return handle;
}