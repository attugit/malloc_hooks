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

static constexpr auto const nframes = 100u;

void print_trace(void const *const ptr, size_t msize) {
  void *array[nframes];
  char **strings;

  auto size = backtrace(array, nframes);
  strings = backtrace_symbols(array, size);
  char buff[1024];
  sprintf(buff, "malloc_hook <pointer=%p> <size=%u>", ptr, (unsigned int)msize);
  for (auto i = 2; i < size; i++)
    fprintf(stderr, "%s <frame=%u> <symbol=%s>\n", buff, i - 2, strings[i]);
  free(strings);
}

static void *my_malloc_hook(size_t size, const void *) {
  auto scope = scoped_hooks(real_malloc_hook, real_free_hook);
  auto ptr = malloc(size);
  print_trace(ptr, size);
  return ptr;
}

static void my_free_hook(void *ptr, const void *) {
  auto scope = scoped_hooks(real_malloc_hook, real_free_hook);
  free(ptr);
  fprintf(stderr, "malloc_hook freed pointer <free=%p>\n", ptr);
}

static void my_init_hook(void) {
  if (real_malloc_hook == nullptr)
    real_malloc_hook = __malloc_hook;
  if (real_free_hook == nullptr)
    real_free_hook = __free_hook;
  __malloc_hook = my_malloc_hook;
  __free_hook = my_free_hook;
}

void (*__malloc_initialize_hook)(void) = my_init_hook;
