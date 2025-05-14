#include "Game/Game.hpp"


int main(int argc, char* argv[]) {
	Game MyGame;

	MyGame.Initialize();
	MyGame.Run();
	MyGame.Destroy();

	return 0;
}
