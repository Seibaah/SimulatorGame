#pragma once
#include <iostream>
#include <random>
#include <SDL.h>
#include <vector>
#include <algorithm>
#include "human.h"

class player
{
public:
	int row, col;	//player coordinates
	int curChunk_x, curChunk_y;		//top left chunk to update camera correctly
	int camBound_xMin, camBound_xMax, camBound_yMin, camBound_yMax;	//camera bounds

	//inventory array
	int inventory[5], inventorySize = std::size(inventory), inventoryLoad = 0;	

	//selecetd items vector, used for crafting
	std::vector<int> selectedItems;

	//player stats
	int hearts = 3, hunger = 3, hungerCountdown = 14;
	bool spear = false;

	player();
	~player();

	void walk(int dir, int boardSideSize);

	void updateCamera(int chunkSize);

	void objectPickup(int (&map)[100][100][3]);

	void updateStats(SDL_bool &quit);

	void eat();

	void renderBMP(SDL_Renderer * renderer, const char* path, int x, int y, int w, int h);

	void attack(SDL_Renderer *renderer, SDL_bool &quit, int(&map)[100][100][3], int cellSize, int chunkSize);

	void damageNPC(int(&map)[100][100][3], int x, int y, bool spear);

	void renderAttack(SDL_Renderer *renderer, int chunkSize, int cellSize, int x, int y, int dir);

	void renderUI(SDL_Renderer *renderer, int cellSize);

	void renderInventory(SDL_Renderer *renderer, int cellSize);

	void renderSlot(SDL_Renderer *renderer, int itemType, int itemPos, int cellSize);

	void renderRect(SDL_Renderer *renderer, int x, int y, int w, int h, int r, int g, int b);

	void renderEmptyRect(SDL_Renderer *renderer, int x, int y, int w, int h, int r, int g, int b);

	void renderStats(SDL_Renderer *renderer, const char* path, int cellSize, int max, int x, int y, int w, int h);

	void clearScreen(SDL_Renderer *renderer);

	void selectItems(SDL_Renderer *renderer, int cellSize);

	void crafting(SDL_Renderer *renderer);
		
};

