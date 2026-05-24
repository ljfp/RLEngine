#include "Game/Game.hpp"
#include <SDL3/SDL_main.h>

int main(int, char*[]) {
	Game MyGame;

	MyGame.Initialize();
	MyGame.Run();
	MyGame.Destroy();

	return 0;
}
