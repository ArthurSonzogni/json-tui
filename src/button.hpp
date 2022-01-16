#include <ftxui/component/component.hpp>
#include <functional>

ftxui::Component MyButton(const char* prefix,
                          const char* title,
                          std::function<void()>);
