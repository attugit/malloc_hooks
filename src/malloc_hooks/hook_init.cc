#include <malloc_hooks.hpp>

void (*__malloc_initialize_hook)(void) = malloc_hooks::init_hook;
