#ifndef MENU_APP_BASE_H
#define MENU_APP_BASE_H

#include <App/app.h>

namespace hitcon {

typedef void (*menu_callback_t)();

struct menu_entry_t {
  const char *name;
  App *app;
  menu_callback_t func;
};

class MenuApp : public App {
 public:
  MenuApp() = default;
  MenuApp(const menu_entry_t *, const int);
  virtual ~MenuApp() = default;

  void OnEntry() override;
  void OnExit() override;
  void OnButton(button_t button) override;
  virtual void OnButtonMode() = 0;
  virtual void OnButtonBack() = 0;
  virtual void OnButtonLongBack() = 0;

 protected:
  const menu_entry_t *menu_entries;
  int menu_entry_index;
  const int menu_entry_size;
};

}  // namespace hitcon

#endif  // MENU_APP_BASE_H