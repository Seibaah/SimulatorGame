#include "player.h"

player::player()
{
}

player::~player()
{
}


void player::walk(int dir, int boardSideSize) {

	//move player coordinates. Will not go out of world bounds.
	switch (dir) {
	case 0:
		if (row != 0)					this->row--; break;
	case 1:
		if (col != boardSideSize - 1)	this->col++; break;
	case 2:
		if (row != boardSideSize - 1)	this->row++; break;
	case 3:
		if (col != 0)					this->col--; break;
	}
}


void player::updateCamera(int chunkSize) {

	//update camera bounds to correctly display camera view 
	if (this->col <= this->camBound_xMin) {
		this->camBound_xMin -= 2 * chunkSize;
		this->camBound_xMax -= 2 * chunkSize;
		this->curChunk_x -= 2;
	}
	else if (this->col >= this->camBound_xMax) {
		this->camBound_xMin += 2 * chunkSize;
		this->camBound_xMax += 2 * chunkSize;
		this->curChunk_x += 2;
	}
	else if (this->row <= this->camBound_yMin) {
		this->camBound_yMin -= 2 * chunkSize;
		this->camBound_yMax -= 2 * chunkSize;
		this->curChunk_y -= 2;
	}
	else if (this->row >= this->camBound_yMax) {
		this->camBound_yMin += 2 * chunkSize;
		this->camBound_yMax += 2 * chunkSize;
		this->curChunk_y += 2;
	}
}


void player::objectPickup(int(&map)[100][100][3]) {

	//can only pickup item if there is inventory space and if there is an item on the tile
	if (this->inventoryLoad < this->inventorySize && map[this->row][this->col][1] != 0) {
		for (int i = 0; i < this->inventorySize; i++) {

			//add item to empty slot
			if (this->inventory[i] == 0) {

				this->inventory[i] = map[this->row][this->col][1];

				//delete item from map
				map[this->row][this->col][1] = 0;

				//update inventory load
				this->inventoryLoad++;
				break;
			}
		}
	}
}


void player::updateStats(SDL_bool &quit) {

	//every turn hunger countdown goes down 1 unit
	this->hungerCountdown--;

	//if hunger bar is full but hearts aren't then regenerate hearts at 1 per turn
	if (this->hearts < 3 && this->hunger == 3) {
		this->hearts++;
	}

	//reduce hunger bar by 1 and reset countdown
	if (this->hungerCountdown == 0) {
		this->hunger--;
		this->hungerCountdown = 14;
	}

	//player loses a heart if hunger bar is at 0, if hearts at 0 then kill player
	if (this->hunger == 0) {
		if (this->hearts == 1) {
			quit = SDL_TRUE;
			std::cout << "you died" << std::endl;
		}
		else this->hearts--;
	}
}


void player::eat() {

	//can only eat if hungry
	if (this->hunger < 3) {
		for (int i = 0; i < this->inventorySize; i++) {

			//eat a fish or apple is possible
			if (this->inventory[i] == 1 || this->inventory[i] == 2) {
				this->inventory[i] = 0;
				this->inventoryLoad--;
				this->hunger++;
				this->hungerCountdown = 15;
				break;
			}
		}
	}
	else std::cout << "you are not hungry..." << std::endl;
}


void player::crafting(SDL_Renderer *renderer) {
	//crafting method - hardcoded: can only craft a spear right now
	int rocksForSpear = 1, woodForSpear = 2, isFish = 0, isApple = 0;
	for (auto i : this->selectedItems) {
		if (this->inventory[i] == 3) {
			woodForSpear--;
		}
		else if (this->inventory[i] == 4) {
			rocksForSpear--;
		}
		else if (this->inventory[i] == 1) {
			isApple++;
		}
		else if (this->inventory[i] == 2) {
			isFish++;
		}
	}
	if (rocksForSpear == 0 && woodForSpear == 0 && isApple == 0 && isFish == 0) {
		this->spear = true;
		for (auto i : this->selectedItems) {
			this->inventory[i] = 0;
			this->inventoryLoad--;
		}
		renderBMP(renderer, "Assets/spear.bmp", 24, 504, 24, 24);
	}
}


void player::renderBMP(SDL_Renderer *renderer, const char* path, int x, int y, int w, int h) {

	SDL_Surface* icon = SDL_LoadBMP(path);
	SDL_Texture* texture = NULL;
	SDL_Rect rect = { x, y, w, h };

	if (icon == NULL)
	{
		printf("Unable to load image ", SDL_GetError());
	}

	texture = SDL_CreateTextureFromSurface(renderer, icon);
	SDL_FreeSurface(icon);

	if (texture == NULL)
	{
		printf("Unable to create texture from surface ", SDL_GetError());
	}

	SDL_RenderCopy(renderer, texture, NULL, &rect);
	SDL_RenderPresent(renderer);
}


void player::attack(SDL_Renderer *renderer, SDL_bool &quit, int(&map)[100][100][3], int cellSize, int chunkSize) {
	
	while (!quit) {
		SDL_Event event;
		
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT) {
				quit = SDL_TRUE;
			}
			else if (event.type == SDL_KEYDOWN) {

				//renders spear in attack direction
				switch (event.key.keysym.sym) {
				case SDLK_UP:
					//NPC damage inactive
					//damageNPC(map, this->col, this->row - 1, this->spear);
					if (this->spear == true) {
						renderAttack(renderer, chunkSize, cellSize, this->col*cellSize, this->row*cellSize, 0);
						SDL_Delay(1000);
					}
					quit = SDL_TRUE;
					break;
				case SDLK_RIGHT:
					//damageNPC(map, this->col + 1, this->row, this->spear);
					if (this->spear == true) {
						renderAttack(renderer, chunkSize, cellSize, this->col*cellSize, this->row*cellSize, 1);
						SDL_Delay(1000);
					}
					quit = SDL_TRUE;
					break;
				case SDLK_DOWN:
					//damageNPC(map, this->col, this->row + 1, this->spear);
					if (this->spear == true) {
						renderAttack(renderer, chunkSize, cellSize, this->col*cellSize, this->row*cellSize, 2);
						SDL_Delay(1000);
					}
					quit = SDL_TRUE;
					break;
				case SDLK_LEFT:
					//damageNPC(map, this->col - 1, this->row, this->spear);
					if (this->spear == true) {
						renderAttack(renderer, chunkSize, cellSize, this->col*cellSize, this->row*cellSize, 3);
						SDL_Delay(1000);
					}
					quit = SDL_TRUE;
					break;
				case SDLK_a:
					break;
				}
			}
		}
	}
	quit = SDL_FALSE;
}

//inactive
void player::damageNPC(int(&map)[100][100][3], int x, int y, bool spear) {
	int mult = 1;
	if (spear == true) {
		mult = 2;
	}
	if (map[y][x][2] != 0) {
		//Todo: apply damage to NPC, use mult as multiplier
	}
}


void player::renderAttack(SDL_Renderer *renderer, int chunkSize, int cellSize, int x, int y, int dir) {

	SDL_Surface* icon = SDL_LoadBMP("Assets/spear_attack.bmp");
	SDL_Texture* texture = NULL;
	SDL_Rect rect;

	switch (dir) {
	case 0:
		rect = { (this->col - this->curChunk_x*chunkSize)*cellSize,
			((this->row - this->curChunk_y*chunkSize)*cellSize) - 20, 24, 24 };
		texture = SDL_CreateTextureFromSurface(renderer, icon);
		SDL_FreeSurface(icon);
		SDL_RenderCopy(renderer, texture, NULL, &rect);
		break;
	case 1:
		rect = { (this->col - this->curChunk_x*chunkSize)*cellSize + 20,
			((this->row - this->curChunk_y*chunkSize)*cellSize), 24, 24 };
		texture = SDL_CreateTextureFromSurface(renderer, icon);
		SDL_FreeSurface(icon);
		SDL_RenderCopyEx(renderer, texture, NULL, &rect, 270, NULL, SDL_FLIP_VERTICAL);
		break;
	case 2:
		rect = { (this->col - this->curChunk_x*chunkSize)*cellSize,
			((this->row - this->curChunk_y*chunkSize)*cellSize) + 20, 24, 24 };
		texture = SDL_CreateTextureFromSurface(renderer, icon);
		SDL_FreeSurface(icon);
		SDL_RenderCopyEx(renderer, texture, NULL, &rect, 180, NULL, SDL_FLIP_HORIZONTAL);
		break;
	case 3:
		rect = { (this->col - this->curChunk_x*chunkSize)*cellSize - 20,
			((this->row - this->curChunk_y*chunkSize)*cellSize), 24, 24 };
		texture = SDL_CreateTextureFromSurface(renderer, icon);
		SDL_FreeSurface(icon);
		SDL_RenderCopyEx(renderer, texture, NULL, &rect, 90, NULL, SDL_FLIP_VERTICAL);
		break;
	}
	SDL_RenderPresent(renderer);
}


void player::renderUI(SDL_Renderer *renderer, int cellSize) {

	renderInventory(renderer, cellSize);
	renderStats(renderer, "Assets/heart.bmp", cellSize, this->hearts, 144, 480, 24, 24);		
	renderStats(renderer, "Assets/hunger.bmp", cellSize, this->hunger, 144, 504, 24, 24);		
	if (spear == true) {
		renderBMP(renderer, "Assets/spear.bmp", 24, 504, 24, 24);
	}
}


void player::renderInventory(SDL_Renderer *renderer, int cellSize) {
	for (int i = 0; i < this->inventorySize; i++) {
		renderSlot(renderer, this->inventory[i], i, cellSize);
	}
}


void player::renderSlot(SDL_Renderer *renderer, int itemType, int itemPos, int cellSize) {
	switch (itemType) {
	case 1:
		renderBMP(renderer, "Assets/apple.bmp", itemPos*cellSize, cellSize*20 + 1, cellSize,
			cellSize);
		break;
	case 2:
		renderBMP(renderer, "Assets/fish.bmp", itemPos*cellSize, cellSize*20 + 1, cellSize,
			cellSize);
		break;
	case 3:
		renderBMP(renderer, "Assets/wood.bmp", itemPos*cellSize, cellSize*20 + 1, cellSize,
			cellSize);
		break;
	case 4:
		renderBMP(renderer, "Assets/rocks.bmp", itemPos*cellSize, cellSize*20 + 1, cellSize,
			cellSize);;
		break;
	case 0:
		renderRect(renderer, itemPos*cellSize, cellSize*20 + 1, cellSize,
			cellSize, 0, 0, 0);
		break;
	}
}

//render filled rectangle
void player::renderRect(SDL_Renderer *renderer, int x, int y, int w, int h, int r, int g, int b) {
	SDL_Rect rect = { x, y, w, h };
	SDL_SetRenderDrawColor(renderer, r, g, b, 0);
	SDL_RenderFillRect(renderer, &rect);
	SDL_RenderPresent(renderer); //Render backbuffer
}

//render empty rectangle
void player::renderEmptyRect(SDL_Renderer *renderer, int x, int y, int w, int h, int r, int g, int b) {
	SDL_Rect rect = { x, y, w, h };
	SDL_SetRenderDrawColor(renderer, r, g, b, 0);
	SDL_RenderDrawRect(renderer, &rect);
	SDL_RenderPresent(renderer); 
}

//render health and hunger stats
void player::renderStats(SDL_Renderer *renderer, const char* path, int cellSize, int max, int x, int y, int w, int h) {
	for (int i = 0; i < max; i++) {
		renderBMP(renderer, path, x + i * cellSize, y, w, h);
	}
}


void player::clearScreen(SDL_Renderer *renderer) {
	SDL_SetRenderDrawColor(renderer, 255, 255, 255, 0);
	SDL_RenderClear(renderer);
}


void player::selectItems(SDL_Renderer *renderer, int cellSize) {
	int flag1 = 0, flag2 = 0, flag3 = 0, flag4 = 0, flag5 = 0;	//flags to test if item has been selected
	SDL_bool quit = SDL_FALSE;
	//allows to select an item and add it to a crafting vector that is checked upon crafting
	//triggers render event to let user know he's selecting or de-selecting an item
	while (!quit) {
		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT) {
				quit = SDL_TRUE;
			}
			else if (event.type == SDL_KEYDOWN) {
				switch (event.key.keysym.sym) {
				case SDLK_1:
					if (flag1 == 0) {
						this->selectedItems.push_back(0);
						flag1 = 1;
						renderEmptyRect(renderer, 0 * cellSize, cellSize*20, cellSize,
							cellSize, 255, 255, 255);
					}
					else if (flag1 == 1) {
						flag1 = 0;
						renderSlot(renderer, this->inventory[0], 0, cellSize);
						renderEmptyRect(renderer, 0 * cellSize, cellSize * 20, cellSize,
							cellSize, 0, 0, 0);
						remove_copy(this->selectedItems.begin(), this->selectedItems.end(), this->selectedItems.begin(), 0);
					}
					break;
				case SDLK_2:
					if (flag2 == 0) {
						this->selectedItems.push_back(1);
						flag2 = 1;
						renderEmptyRect(renderer, 1 * cellSize, cellSize*20, cellSize,
							cellSize, 255, 255, 255);
					}
					else if (flag2 == 1) {
						flag2 = 0;
						renderSlot(renderer, this->inventory[1], 1, cellSize);
						renderEmptyRect(renderer, 1 * cellSize, cellSize * 20, cellSize,
							cellSize, 0, 0, 0);
						remove_copy(this->selectedItems.begin(), this->selectedItems.end(), this->selectedItems.begin(), 1);
					}
					break;
				case SDLK_3:
					if (flag3 == 0) {
						this->selectedItems.push_back(2);
						flag3 = 1;
						renderEmptyRect(renderer, 2 * cellSize, cellSize*20, cellSize,
							cellSize, 255, 255, 255);
					}
					else if (flag3 == 1) {
						flag3 = 0;
						renderSlot(renderer, this->inventory[2], 2, cellSize);
						renderEmptyRect(renderer, 2 * cellSize, cellSize * 20, cellSize,
							cellSize, 0, 0, 0);
						remove_copy(this->selectedItems.begin(), this->selectedItems.end(), this->selectedItems.begin(), 2);
					}
					break;
				case SDLK_4:
					if (flag4 == 0) {
						this->selectedItems.push_back(3);
						flag4 = 1;
						renderEmptyRect(renderer, 3 * cellSize, cellSize*20, cellSize,
							cellSize, 255, 255, 255);
					}
					else if (flag4 == 1) {
						flag4 = 0;
						renderSlot(renderer, this->inventory[3], 3, cellSize);
						renderEmptyRect(renderer, 3 * cellSize, cellSize * 20, cellSize,
							cellSize, 0, 0, 0);
						remove_copy(this->selectedItems.begin(), this->selectedItems.end(), this->selectedItems.begin(), 3);
					}
					break;
				case SDLK_5:
					if (flag5 == 0) {
						this->selectedItems.push_back(4);
						flag5 = 1;
						renderEmptyRect(renderer, 4 * cellSize, cellSize*20, cellSize,
							cellSize, 255, 255, 255);
					}
					else if (flag5 == 1) {
						flag5 = 0;
						renderSlot(renderer, this->inventory[4], 4, cellSize);
						renderEmptyRect(renderer, 4 * cellSize, cellSize * 20, cellSize,
							cellSize, 0, 0, 0);
						remove_copy(this->selectedItems.begin(), this->selectedItems.end(), this->selectedItems.begin(), 4);
					}
					break;
				case SDLK_s:
					this->selectedItems.clear();
					renderUI(renderer, cellSize);
					quit = SDL_TRUE;
					break;
				case SDLK_d:
					for (auto i : this->selectedItems) {
						this->inventory[i] = 0;
						this->inventoryLoad--;
					}
					this->selectedItems.clear();
					renderUI(renderer, cellSize);
					quit = SDL_TRUE;
					break;
				case SDLK_c:
					this->crafting(renderer);
					this->selectedItems.clear();
					renderUI(renderer, cellSize);
					quit = SDL_TRUE;
					break;
				}
			}
		}
	}
	quit = SDL_FALSE;
}
