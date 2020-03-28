#pragma once
#include <iostream>
#include <random>
#include <SDL.h>
#include <vector>
#include <algorithm>
#include "human.h"
#include "player.h"

using namespace std;

class world
{
public:
	int board[100][100][3] = { 0 }, boardSideSize = size(board);		//Full map and map side size
	int chunks[10][10], chunksSize = size(chunks);	//Chunks map and chunks side size
	int camSize = size(chunks) * 2;		//Camera view size
	int turnsElapsed = 0;				//Keeps track of tunr number for scheduled events

	player p1;							//player controlled avatar
	vector<human> humanList;			//list containing different npcs

	//palette for map display
	SDL_Color palette[20] = { {25, 25, 112, 0}, {0, 0, 128, 0}, {0, 0, 205, 0}, {0, 0, 225, 0}, {0, 0, 255, 0}, {45, 100, 245, 0},
								{51, 171, 240, 0}, {82, 219, 255, 0}, {110, 255, 255, 0}, {168, 255, 255}, {227, 220, 192, 0},
								{219, 173, 114, 0}, {124, 252, 0, 0}, {34, 139, 34, 0}, {0, 100, 0, 0},
								{193, 210, 214, 0}, {174, 187, 199, 0}, {106, 125, 142, 0}, {105, 105, 105, 0}, {240, 240, 240, 0} };
	int cellSize = 24;									//individual cell size
	int grid_width = 100, grid_height = 100;	
	//window dimensions and renderer refs
	int window_height = (cellSize * camSize) + 1;		
	int window_width = (cellSize * camSize) + 1;
	SDL_Window *window;
	SDL_Renderer *renderer;
	SDL_bool quit = SDL_FALSE;

	world();
	~world();

	void run(int turns);

	int loadWorld();
	
	void loadHumans();
	
	int setNPCSpawn(human * npc, int mult);

	int setPlayerSpawn();
	
	void NPCsDirectives();
	
	void renderCameraPerspective();
	
	int tileComparator(int x);
	
	void renderPlayer();
	
	void simLoop(int a);
	
	void displayWorldMap();
	
	void spawnItems(double prob, int tileID, int type);
	
	void inputReader(int a);
	
	void eventHandler(int dir, int a);
	
	void renderRect(int x, int y, int w, int h, int r, int g, int b);
	
	void clearScreen();
	
	void spawnTimer();
	
	void renderBMP(const char * path, int x, int y, int w, int h);
};

