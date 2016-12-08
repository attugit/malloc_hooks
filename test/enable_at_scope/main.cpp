#include <malloc_hooks.hpp>
#include <iostream>
#include <memory>

int main()
{
  std::cout << "before scopes\n";
  {
    std::cout << "disabled scope\n";
    auto x = std::make_unique<int>(7);
  }
  {
    auto scope = malloc_hooks::enable_at_scope();
    std::cout << "enabled scope\n";
    auto x = std::make_unique<int>(7);
  }
  {
    std::cout << "another disabled scope\n";
    auto x = std::make_unique<int>(7);
  }
  std::cout << "after scopes\n";
  return 0;
}
