#include <GL/freeglut.h>
#include "Game.h"

int main(int argc, char** argv) {
    // Command-line arguments are parsed in Game constructor
    Game game(800, 600, argc, argv);
    return 0;
}
