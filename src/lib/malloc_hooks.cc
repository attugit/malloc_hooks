#include <execinfo.h>
#include <malloc_hooks.hpp>
#include <stdio.h>
#include <cstdlib>

namespace malloc_hooks
{
  scoped_hooks::scoped_hooks(mhook_t mh, fhook_t fh) noexcept
      : mhook(::__malloc_hook), fhook(::__free_hook)
  {
    ::__malloc_hook = mh;
    ::__free_hook = fh;
  }

  scoped_hooks::~scoped_hooks() noexcept
  {
    ::__malloc_hook = mhook;
    ::__free_hook = fhook;
  }

  namespace
  {
    void* unset_malloc_hook(size_t, const void*) { return nullptr; }
    void unset_free_hook(void*, const void*) {}
  }

  thread_local decltype(::__malloc_hook) real_malloc_hook = unset_malloc_hook;
  thread_local decltype(::__free_hook) real_free_hook = unset_free_hook;

  namespace
  {
    void* malloc_hook(std::size_t, const void*);
    void free_hook(void*, const void*);

    void backup_real_hooks()
    {
      if (real_malloc_hook == unset_malloc_hook) real_malloc_hook = ::__malloc_hook;
      if (real_free_hook == unset_free_hook) real_free_hook = ::__free_hook;
    }

    void assign_custom_hooks()
    {
      if (::__malloc_hook == real_malloc_hook) ::__malloc_hook = malloc_hook;
      if (::__free_hook == real_free_hook) ::__free_hook = free_hook;
    }
    static constexpr auto const nframes = 100u;
    void print_trace(void const* const ptr, std::size_t msize)
    {
      void* array[nframes];
      char** strings;

      auto size = ::backtrace(array, nframes);
      strings = ::backtrace_symbols(array, size);
      char buff[1024];
      ::sprintf(buff, "malloc_hook <pointer=%p> <size=%lu>", ptr, msize);
      for (auto i = 2; i < size; i++)
        ::fprintf(stderr, "%s <frame=%u> <symbol=%s>\n", buff, i - 2, strings[i]);
      ::free(strings);
    }

    void* malloc_hook(std::size_t size, const void*)
    {
      auto scope = disable_at_scope();
      auto ptr = ::malloc(size);
      print_trace(ptr, size);
      return ptr;
    }

    void free_hook(void* ptr, const void*)
    {
      auto scope = disable_at_scope();
      ::free(ptr);
      ::fprintf(stderr, "malloc_hook freed pointer <free=%p>\n", ptr);
    }
  }

  scoped_hooks disable_at_scope()
  {
    backup_real_hooks();
    return scoped_hooks(real_malloc_hook, real_free_hook);
  }

  scoped_hooks enable_at_scope()
  {
    backup_real_hooks();
    return scoped_hooks(malloc_hook, free_hook);
  }
  void init_hook()
  {
    backup_real_hooks();
    assign_custom_hooks();
  }
}
