#include <malloc_hooks.hpp>
#include <iostream>
#include <memory>

int main()
{
  malloc_hooks::init_hook();
  std::cout << "before scopes\n";
  {
    std::cout << "enabled scope\n";
    auto x = std::make_unique<int>(7);
  }
  {
    auto scope = malloc_hooks::disable_at_scope();
    std::cout << "disabled scope\n";
    auto x = std::make_unique<int>(7);
  }
  {
    std::cout << "another enabled scope\n";
    auto x = std::make_unique<int>(7);
  }
  std::cout << "after scopes\n";
  return 0;
}
