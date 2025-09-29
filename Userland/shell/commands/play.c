#include "unistd.h"
#include "commands.h"
#include "stdio.h"
#include "stddef.h"

// Dirección de memoria donde se carga el programa minigolf
// Esta dirección corresponde a 0xB00000 (definida en minigolf.ld)
#define MINIGOLF_ADDRESS 0xB00000

int play(void) {
    // Ejecutar el programa minigolf usando sys_exec
    sys_exec(MINIGOLF_ADDRESS);
    
    // Cuando el programa termine, limpiar la pantalla y volver a la consola
    sys_clear_screen(0x000000);
    sys_rerender_console();
    
    return 0;
}

command play_cmd = {
    "play",
    play,
    "Play a minigolf game",
    "PLAY(1)                     User Commands                     PLAY(1)\n\n"
    "NAME\n"
    "       play - launch the minigolf game\n\n"
    "SYNOPSIS\n"
    "       play\n\n"
    "DESCRIPTION\n"
    "       This command launches the built-in minigolf game.\n\n"
    "       The game provides an interactive minigolf experience\n\n"
    "       with graphics and user controls.\n\n"
    "       Upon game completion or exit, the screen will be cleared and\n"
    "       the console will be restored to its normal state.\n\n"
};