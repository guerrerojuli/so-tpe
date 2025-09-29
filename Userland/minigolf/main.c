#include "game.h"
#include "stdio.h"
#include "unistd.h"

int main() {
  char choice;
  int running = 1;
  while (running) {
    show_menu();

    // Get user input
    do {
      choice = getchar();

      switch (choice)
      {
      case '1':
        start_singleplayer();
        break;
      case '2':
        start_multiplayer();
        break;
      case '3':
        running = 0;
        sys_clear_screen(0x000000);
    sys_render_screen();
        sys_rerender_console();
        break;
      }
    } while (choice < '1' || choice > '3');
  }

  return 0;
}