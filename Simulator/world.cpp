#include "world.h"

world::world()
{
}

world::~world()
{
}

//runs the simulation 
void world::run(int turns) {

	loadWorld();

	renderCameraPerspective();

	p1.renderUI(renderer, cellSize);

	simLoop(turns);
}

//Main simulation loop
void world::simLoop(int a) {

	while (!quit) {
		inputReader(a);
	
		if (a == 0) {
			break;
		}
	}
	quit = SDL_TRUE;
}

//Generates terrain using n_size footprints and more
int world::loadWorld() {
	int groundCoord[100][2], groundCount = 0;
	int waterCoord[100][2], waterCount = 0;
	random_device rd;           //Will be used to obtain a seed for the random number engine
	mt19937 gen(rd());          //Standard mersenne_twister_engine seeded with rd()
	uniform_real_distribution<> disSeedOnMap(0.4, 0.66);        //Distribution for seed probability on the map
	uniform_real_distribution<> disSeedInChunk(0.0, 11.0);		//Distribution for seed within selected chunk
	uniform_real_distribution<> disWaterSeed(0.0, 2.0);			//Distribution for water seed within the rest of the chunks
	double baseCoef = (double)((int)(disSeedOnMap(gen) * 100)) / 100;	//Probability of land 

	//marks which chunks will hold a seed for the world gen to use later
	for (int i = 0; i < chunksSize; i++) {
		for (int j = 0; j < chunksSize; j++) {
			double seedProb = (double)((int)(disSeedOnMap(gen) * 100)) / 100;
			//determines if the chunk gets a ground seed
			if (seedProb <= baseCoef) {
				chunks[i][j] = 1;
				//place ground seed inside chunk
				board[i * chunksSize + (int)disSeedInChunk(gen)][j * chunksSize + (int)disSeedInChunk(gen)][0] = 1;
				//save seed coord
				groundCoord[groundCount][0] = i * chunksSize + (int)disSeedInChunk(gen); //y
				groundCoord[groundCount][1] = j * chunksSize + (int)disSeedInChunk(gen); //x
				groundCount++;
			}
			//else it gets a water seed
			else {
				chunks[i][j] = -1;
				//place ground seed inside chunk
				board[i * chunksSize + (int)disSeedInChunk(gen)][j * chunksSize + (int)disSeedInChunk(gen)][0] = -1;
				//save seed coord
				waterCoord[waterCount][0] = i * chunksSize + (int)disSeedInChunk(gen); //y
				waterCoord[waterCount][1] = j * chunksSize + (int)disSeedInChunk(gen); //x
				waterCount++;
			}
		}
	}

	//loop through the ground coordinates and elevates terrain around each one
	uniform_real_distribution<> disHeight(0, 2.0);
	uniform_real_distribution<> disRange(2.0, 5.0);
	for (int n = 0; n < groundCount; n++) {
		int y = groundCoord[n][0], x = groundCoord[n][1];
		double time = (int)disRange(gen);
		for (int t = 0; t < time; t++) {
			for (int y2 = y - t; y2 <= 2 * y; y2++) {
				for (int x2 = x - t; x2 <= 2 * x; x2++) {
					if ((y2 >= 0 && y2 < boardSideSize) && (x2 >= 0 && x2 < boardSideSize)) {
						if (t == time - 1) {
							int var = (int)disHeight(gen);
							board[y2][x2][0] += var;
						}
						else	board[y2][x2][0] += 1;
					}
				}
			}
		}
	}

	//loop through the water coordinates and decrease terrain around each one
	uniform_real_distribution<> disDeep(-1.99, 0.0);
	uniform_real_distribution<> disRange2(2.0, 5.0);
	for (int n = 0; n < waterCount; n++) {
		int y = waterCoord[n][0], x = waterCoord[n][1];
		double time2 = (int)disRange2(gen);
		for (int t = 0; t < time2; t++) {
			for (int y2 = y - t; y2 <= 2 * y; y2++) {
				for (int x2 = x - t; x2 <= 2 * x; x2++) {
					if ((y2 >= 0 && y2 < boardSideSize) && (x2 >= 0 && x2 < boardSideSize)) {
						if (t == time2 - 1) {
							int var = (int)disDeep(gen);
							board[y2][x2][0] -= var;
						}
						else	board[y2][x2][0] -= 1;
					}
				}
			}
		}
	}

	//apple
	spawnItems(0.04, 13, 1);
	//fish
	spawnItems(0.04, 9, 2);
	//wood
	spawnItems(0.04, 14, 3);
	//rocks
	spawnItems(0.04, 16, 4);

	//spawn p1 and set camera bounds
	setPlayerSpawn();
	p1.curChunk_x = p1.col / 10, p1.curChunk_y = p1.row / 10;
	if (p1.curChunk_x % 2 != 0) {
		p1.curChunk_x--;
	}
	if (p1.curChunk_y % 2 != 0) {
		p1.curChunk_y--;
	}
	p1.camBound_xMin = p1.curChunk_x * chunksSize - 1;
	p1.camBound_xMax = p1.curChunk_x * chunksSize + 2*chunksSize;
	p1.camBound_yMin = p1.curChunk_y * chunksSize - 1;
	p1.camBound_yMax = p1.curChunk_y * chunksSize + 2*chunksSize;
	
	//SDL init checks 
	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Initialize SDL: %s",
			SDL_GetError());
		return EXIT_FAILURE;
	}
	if (SDL_CreateWindowAndRenderer(window_width, window_height + 60, 0, &window, &renderer) < 0) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
			"Create window and renderer: %s", SDL_GetError());
		return EXIT_FAILURE;
	}

	//load NPCs in the world
	loadHumans();

	return 1;
}


void world::loadHumans() {
	//number of NPCs is also random
	random_device rd;           
	mt19937 gen(rd());          
	uniform_real_distribution<> numOfNPC(8, 17);
	double numberOfNPCs = (int)(numOfNPC(gen));	

	//NPCs stored in a vector for simple access to each then spawn is set
	for (int i = 0; i < numberOfNPCs; i++) {
		humanList.push_back(*(new human));
	}
	for (int i = 0; i < humanList.size(); i++) {
		setNPCSpawn(&humanList[i], i);
	}
}


int world::setNPCSpawn(human *npc, int mult) {
	//NPCs are spawned at random locations on land or shallow water tiles
	random_device rd;          
	mt19937 gen(rd());          
	uniform_real_distribution<> disSpawn(0, 1);


	for (int i = 0 + 3 * mult; i < boardSideSize; i++) {
		for (int j = 0 * mult + mult; j < boardSideSize; j++) {

			if (board[i][j][0] >= -5 && board[i][j][0] <= 25 && board[i][j][1] == 0) {
				double NPCspawn = (disSpawn(gen));
				if (NPCspawn <= 0.00075) {
					board[i][j][2] = 1;
					npc->setY(i);
					npc->setX(j);
					return 1;
				}
			}
		}
	}
}


int world::setPlayerSpawn() {
	//NPCs are spawned at random locations on land or shallow water tiles
	random_device rd;
	mt19937 gen(rd());
	uniform_real_distribution<> disSpawn(0, 1);


	for (int i = 0; i < boardSideSize; i++) {
		for (int j = 0; j < boardSideSize; j++) {

			if (board[i][j][0] >= -5 && board[i][j][0] <= 25) {
				double PlayerSpawn = (disSpawn(gen));
				if (PlayerSpawn <= 0.00075) {
					p1.row = i;
					p1.col = j;
					return 1;
				}
			}
		}
	}
}


void world::NPCsDirectives() {
	for (int i = 0; i < humanList.size(); i++) {
		human *npc = &humanList[i];

		//the NPC has to be alive to make a decision
		if (npc->isDead == false) {

			//implementation incomplete: inactive
			//npc->escapeDanger(board);	

			//npc tries to eat if hungry
			if (npc->hunger < 3.0) {
				npc->eat();
			}

			//implementation incomplete: inactive
			if (npc->dangerDeltaX != NULL || npc->dangerDeltaY != NULL) {
				npc->moveNPCs(board, npc->col - npc->dangerDeltaX, npc->row - npc->dangerDeltaY);
			}
			//if the NPC gets hungry it tries to find food and move towards it
			else if (npc->decisionCoef <= 0.7) {
				npc->findFood(board);
				npc->moveNPCs(board, npc->foodTargetX, npc->foodTargetY);
			}
			//else it tries to find material
			else {
				npc->findMat(board);
				npc->moveNPCs(board, npc->matTargetX, npc->matTargetY);
			}

			//update the hunger and health stats of the NPC
			npc->updateStatsNPC(board);
		}
	}
}


void world::renderCameraPerspective() {

	//Loops through a 20x20 area of the main world where the player stands
	int x = 0, y = 0;
	for (int i = p1.curChunk_y*chunksSize; i < p1.curChunk_y*chunksSize + camSize; i++) {
		x = 0;
		for (int j = p1.curChunk_x*chunksSize; j < p1.curChunk_x*chunksSize + camSize; j++) {

			//Prepare for render the tile
			SDL_Rect tile = { (x * cellSize), (y * cellSize), cellSize, cellSize };
			int colorPos = tileComparator(board[i][j][0]);
			SDL_SetRenderDrawColor(renderer, palette[colorPos].r, palette[colorPos].g, palette[colorPos].b, 0);
			SDL_RenderFillRect(renderer, &tile);

			//On top, if applicable, render the object
			if (board[i][j][1] == 1) {
				renderBMP("Assets/apple.bmp", x*cellSize, y*cellSize + 1, 24, 24);
			}
			else if (board[i][j][1] == 2) {
				renderBMP("Assets/fish.bmp", x*cellSize, y*cellSize + 1, 24, 24);
			}
			else if (board[i][j][1] == 3) {
				renderBMP("Assets/wood.bmp", x*cellSize, y*cellSize + 1, 24, 24);
			}
			else if (board[i][j][1] == 4) {
				renderBMP("Assets/rocks.bmp", x*cellSize, y*cellSize + 1, 24, 24);
			}

			//On top, if applicable, render an NPC
			if (board[i][j][2] == 1) {
				renderBMP("Assets/human.bmp", x*cellSize, y*cellSize + 1, 24, 24);
			}
			x++;
		}
		y++;
	}

	//render the player
	renderPlayer();

	//set line color for rendering grid
	SDL_Color grid_line_color = { 255, 255, 255, 255 };
	SDL_SetRenderDrawColor(renderer, grid_line_color.r, grid_line_color.g,
		grid_line_color.b, grid_line_color.a);

	//draw vertical lines in backbuffer
	for (int x = 0; x < 1 + cellSize * 20; x += cellSize) {
		SDL_RenderDrawLine(renderer, x, 0, x, window_height);
	}
	//draw horizontal lines in backbuffer
	for (int y = 0; y < 1 + cellSize * camSize; y += cellSize) {
		SDL_RenderDrawLine(renderer, 0, y, window_width, y);
	}

	SDL_RenderPresent(renderer); //Render backbuffer to the screen
}

//returns a value representing a tile
int world::tileComparator(int x) {
	if (x <= -41) {
		return 0;
	}
	else if (x <= -36 && x >= -40) {
		return 1;
	}
	else if (x <= -31 && x >= -35) {
		return 2;
	}
	else if (x <= -26 && x >= -30) {
		return 3;
	}
	else if (x <= -21 && x >= -25) {
		return 4;
	}
	else if (x <= -16 && x >= -20) {
		return 5;
	}
	else if (x <= -11 && x >= -15) {
		return 6;
	}
	else if (x <= -6 && x >= -10) {
		return 7;
	}
	else if (x <= -3 && x >= -5) {
		return 8;
	}
	else if (x <= -1 && x >= -2) {
		return 9;
	}
	else if (x == 0) {
		return 10;
	}
	else if (x >= 1 && x <= 2) {
		return 11;
	}
	else if (x >= 2 && x <= 5) {
		return 12;
	}
	else if (x >= 6 && x <= 10) {
		return 13;
	}
	else if (x >= 11 && x <= 15) {
		return 14;
	}
	else if (x >= 16 && x <= 20) {
		return 15;
	}
	else if (x >= 21 && x <= 25) {
		return 16;
	}
	else if (x >= 26 && x <= 32) {
		return 17;
	}
	else if (x >= 33 && x <= 40) {
		return 18;
	}
	else {
		return 19;
	}
}

//Renders player sprite
void world::renderPlayer() {
	renderBMP("Assets/player.bmp", ((p1.col - p1.curChunk_x*chunksSize)*cellSize), 
		((p1.row - p1.curChunk_y*chunksSize)*cellSize), 24, 24);
}

//Display world map when pressing m, toggle off by pressing m again
void world::displayWorldMap() {

	clearScreen(); //clear screen

	//coded dimensions to fit whole map in current window
	int cellSize2 = 4, mapSize = 100;

	for (int i = 0; i < mapSize; i++) {
		for (int j = 0; j < mapSize; j++) {

			SDL_Rect tile = { j*cellSize2, i*cellSize2, cellSize2, cellSize2 };

			//render player position with a pink square
			if (i == p1.row && j == p1.col) {
				SDL_SetRenderDrawColor(renderer, 228, 0, 224, 0);
			}
			//otherwise render terrain tiles
			else {
				int colorPos = tileComparator(board[i][j][0]);
				SDL_SetRenderDrawColor(renderer, palette[colorPos].r, palette[colorPos].g, palette[colorPos].b, 0);
			}
			SDL_RenderFillRect(renderer, &tile);
		}
	}

	//render ui
	p1.renderUI(renderer, cellSize);

	SDL_RenderPresent(renderer); //Render backbuffer to the screen

	//handle input to go back to camera view
	while (!quit) {
		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT) {
				quit = SDL_TRUE;
			}
			else if (event.type == SDL_KEYDOWN) {
				switch (event.key.keysym.sym) {
				case SDLK_m:
					renderCameraPerspective();
					quit = SDL_TRUE;
					break;
				}
			}
		}
	}
	quit = SDL_FALSE;
}


void world::spawnItems(double prob, int tileID, int type) {
	
	//items are spawned at random locations on their specific tile type
	random_device rd;
	mt19937 gen(rd());
	uniform_real_distribution<> disItem(0.0, 1.0);
	for (int i = 0; i < boardSideSize; i++) {
		for (int j = 0; j < boardSideSize; j++) {

			int val = board[i][j][0];
			if (tileComparator(val) == tileID) {
				double itemChance = (double)((int)(disItem(gen) * 100)) / 100;
				if (itemChance <= prob) {
					//items are saved in the second level of the world matrix
					board[i][j][1] = type;
				}
			}
		}
	}
}


void world::inputReader(int a) {

	//polling for player input
	SDL_Event event;
	while (SDL_PollEvent(&event)) {
		if (event.type == SDL_QUIT) {
			quit = SDL_TRUE;
		}
		//on key pressed down
		else if (event.type == SDL_KEYDOWN) {
			switch (event.key.keysym.sym) {

			//use arrow keys to move
			case SDLK_UP:
				eventHandler(0, a);
				break;
			case SDLK_RIGHT:
				eventHandler(1, a);
				break;
			case SDLK_DOWN:
				eventHandler(2, a);
				break;
			case SDLK_LEFT:
				eventHandler(3, a);
				break;
			//m to display world map
			case SDLK_m:
				displayWorldMap();
				break;
			//e to eat a food item in inventory
			case SDLK_e:
				p1.eat();
				eventHandler(-1, a);
				break;
			//s to enter inventory select mode
			case SDLK_s:
				p1.selectItems(renderer, cellSize);
				eventHandler(-1, a);
				break;
			//a to enter attack mode
			case SDLK_a:
				p1.attack(renderer, quit, board, cellSize, chunksSize);
				turnsElapsed++;
				eventHandler(-1, a--);
				break;
			}
		}
		//ignore other input
		else continue;
	}
}


void world::eventHandler(int dir, int a) {

	//block for movement actions
	if (dir != -1) {

		//walk
		p1.walk(dir, boardSideSize);

		//picup object if there is an item and if there is inventory space available
		p1.objectPickup(board);

		//update camera bounds (changes visible of player leaves visiable chunks
		p1.updateCamera(chunksSize);

		//keep track for scheduled events
		a--;
		turnsElapsed++;
	}

	//clear screen and render camera again
	clearScreen();
	renderCameraPerspective();
	
	//every 20 turns items spawn and timer resets
	if (turnsElapsed == 19) {
		spawnTimer();
		turnsElapsed = 0;
	}

	//update player hunger and health every turns; render updated ui after
	p1.updateStats(quit);
	p1.renderUI(renderer, cellSize);

	//process NPC's actions
	NPCsDirectives();
	
}

//pass to render backbuffer a filled rectangle at x,y coordinates; h,w are height and width; r,g,b are color values
void world::renderRect(int x, int y, int w, int h, int r, int g, int b) {
	SDL_Rect rect = { x, y, w, h };
	SDL_SetRenderDrawColor(renderer, r, g, b, 0);
	SDL_RenderFillRect(renderer, &rect);
}


void world::clearScreen() {
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
	SDL_RenderClear(renderer);
}


void world::spawnTimer() {
	//apple
	spawnItems(0.0001, 13, 1);
	//fish
	spawnItems(0.0001, 9, 2);
	//wood
	spawnItems(0.0001, 14, 3);
	//rocks
	spawnItems(0.0001, 16, 4);
}

//render bmp at given path on x,y coords
void world::renderBMP(const char* path, int x, int y, int w, int h) {

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

	//pass to backbuffer
	SDL_RenderCopy(renderer, texture, NULL, &rect);
}
