#include <execinfo.h>
#include <malloc.h>
#include <stdio.h>
#include <stdlib.h>

decltype(__malloc_hook) real_malloc_hook = nullptr;
decltype(__free_hook) real_free_hook = nullptr;

struct scoped_hooks {
  using mhook_t = decltype(__malloc_hook);
  using fhook_t = decltype(__free_hook);
  mhook_t mhook;
  fhook_t fhook;

  scoped_hooks(mhook_t mh, fhook_t fh)
      : mhook(__malloc_hook), fhook(__free_hook) {
    __malloc_hook = mh;
    __free_hook = fh;
  }
  ~scoped_hooks() {
    __malloc_hook = mhook;
    __free_hook = fhook;
  }
};

void print_trace(void) {
  void *array[10];
  size_t size;
  char **strings;
  size_t i;

  size = backtrace(array, 10);
  strings = backtrace_symbols(array, size);

  for (i = 2; i < size; i++)
    fprintf(stderr, "[%02d] %s\n", i - 2, strings[i]);
  free(strings);
}

static void *my_malloc_hook(size_t size, const void *caller) {
  void *result;
  auto scope = scoped_hooks(real_malloc_hook, real_free_hook);
  result = malloc(size);
  fprintf(stderr, "malloc (%u) returns %p\n", (unsigned int)size, result);
  print_trace();
  return result;
}

static void my_free_hook(void *ptr, const void *caller) {
  auto scope = scoped_hooks(real_malloc_hook, real_free_hook);
  free(ptr);
  fprintf(stderr, "freed pointer %p\n", ptr);
  print_trace();
}

static void my_init_hook(void) {
  real_malloc_hook = __malloc_hook;
  real_free_hook = __free_hook;
  __malloc_hook = my_malloc_hook;
  __free_hook = my_free_hook;
}

void (*__malloc_initialize_hook)(void) = my_init_hook;
