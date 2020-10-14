#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <dlfcn.h>
#include "main.h"


// /* The functions we will find in the plugin */
// typedef void (*init_f) ();
// init_f init;
// typedef char *(*query_f) (char *);
// query_f query;


typedef struct plugin {
	char *path;
	void *plugin;
	// struct *tags;
	void (*init)();
	void (*query)();
	void (*deinit)();
} plugin_t;


int is_ends_with(const char *str, const char *suffix) {
    if (!str || !suffix)
        return 0;
    size_t lenstr = strlen(str);
    size_t lensuffix = strlen(suffix);
    if (lensuffix > lenstr)
        return 0;
    return strncmp(str + lenstr - lensuffix, suffix, lensuffix) == 0;
}

int is_plugin_ext(const char *str) {
	return is_ends_with(str, ".so");
}

int main(const int argc, char const *argv[]) {
	// char plugin_name[] = ;
	// char file_name[80];
	void *plugin;

	DIR *dir;
	struct dirent *ent;
	if ((dir = opendir(PLUGINS_PATH)) != NULL) {
		while ((ent = readdir(dir)) != NULL) {
			if (is_plugin_ext(ent->d_name)) {

				printf("%s%s\n", PLUGINS_PATH, ent->d_name);
			}
		}
		closedir(dir);
	} else {
		/* could not open directory */
		perror("");
		return EXIT_FAILURE;
	}

	// plugin = dlopen(plugin_name, RTLD_NOW);
	// if (!plugin) {
	// 	printf("Cannot load %s: %s", plugin_name, dlerror());
	// 	return EXIT_FAILURE;
	// } else {
	// 	init = *(init_f)dlsym(plugin, "init");
	// 	char *result = dlerror();
	// 	if (result) {
	// 	   printf("Cannot find init in %s: %s", plugin_name, result);
	// 	   return EXIT_FAILURE;
	// 	}
	// 	init();


	// 	query = dlsym (plugin, "query");
	// 	result = dlerror();
	// 	if (result) {
	// 	    printf("Cannot find query in %s: %s", plugin_name, result);
	// 	   return EXIT_FAILURE;
	// 	}
	// 	char msg[100] = "Hello";
	// 	printf("Result of plugin %s is %s\n", plugin_name, query(msg));

	// 	dlclose(plugin);
	// }
	return 0;
}
