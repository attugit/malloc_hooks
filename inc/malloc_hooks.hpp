#pragma once
#include <malloc.h>

namespace malloc_hooks
{
  class scoped_hooks {
    using mhook_t = decltype(::__malloc_hook);
    using fhook_t = decltype(::__free_hook);
    mhook_t const mhook;
    fhook_t const fhook;

  public:
    explicit scoped_hooks(mhook_t mh, fhook_t fh) noexcept;
    ~scoped_hooks() noexcept;
  };

  scoped_hooks disable_at_scope();
  scoped_hooks enable_at_scope();
  void init_hook();
}
