#include <App/DinoApp.h>
#include <App/ShowNameApp.h>
#include <App/SnakeApp.h>
#include <App/TetrisApp.h>
#include <Logic/BadgeController.h>

#include "MenuApp.h"

namespace hitcon {

using hitcon::app::dino::dino_app;
using hitcon::app::snake::snake_app;
using hitcon::app::tetris::tetris_app;

constexpr menu_entry_t main_menu_entries[] = {
    // TODO : change app
    {"BadUSB", nullptr, nullptr},
    {"Snake", &snake_app, &hitcon::app::snake::SetSingleplayer},
    {"Dino", &dino_app, nullptr},
    {"Tetris", &tetris_app, &hitcon::app::tetris::SetSingleplayer},
};

constexpr int main_menu_entries_len =
    sizeof(main_menu_entries) / sizeof(menu_entry_t);

class MainMenuApp : public MenuApp {
 public:
  MainMenuApp() : MenuApp(main_menu_entries, main_menu_entries_len) {}
  void OnButtonMode() override { badge_controller.change_app(&show_name_app); }
  void OnButtonBack() override { badge_controller.change_app(&show_name_app); }
  void OnButtonLongBack() override {
    badge_controller.change_app(&show_name_app);
  }
};

extern MainMenuApp main_menu;

}  // namespace hitcon
