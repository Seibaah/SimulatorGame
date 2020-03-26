#include <iostream>
#include <random>
#include <SDL.h>
#include <vector>
#include <algorithm>
#include "human.h"

#undef main


using namespace std;

class player {
public:
	int row, col;	//player coordinates
	int curChunk_x, curChunk_y;		//top left camera chunk
	int camBound_xMin, camBound_xMax, camBound_yMin, camBound_yMax;	//camera bounds
	enum direction { up = 0, right = 1, down = 2, left = 3 };		//orientation
	int inventory[5] , inventorySize = size(inventory), inventoryLoad = 0;
	vector<int> selectedItems;
	int hearts = 3, hunger = 3, hungerCountdown = 14;
	bool spear = false;
	
	player() {}
	~player() {}

	//bounds hardcoded for 100x100 grid
	void walk(int dir) {
		switch (dir) {
			case 0:
				if (row != 0)	this->row--; break;
			case 1:
				if (col != 99)	this->col++; break;
			case 2:
				if (row != 99)	this->row++; break;
			case 3:
				if (col != 0) this->col--; break;
		}
	}
};

class world2D {
public:
	int m[100][100][3] = { 0 }, mSize = size(m);		//Full map and map side size
	int chunks[10][10], chunksSize = size(chunks);	//Chunks map and chunks side size
	int camSize = size(chunks)*2;		//Camera view size
	int turnsElapsed = 0;
	player p1;
	vector<human> humanList;
	
	SDL_Color palette[20] = { {25, 25, 112, 0}, {0, 0, 128, 0}, {0, 0, 205, 0}, {0, 0, 225, 0}, {0, 0, 255, 0}, {45, 100, 245, 0},
								{51, 171, 240, 0}, {82, 219, 255, 0}, {110, 255, 255, 0}, {168, 255, 255}, {227, 220, 192, 0},
								{219, 173, 114, 0}, {124, 252, 0, 0}, {34, 139, 34, 0}, {0, 100, 0, 0},
								{193, 210, 214, 0}, {174, 187, 199, 0}, {106, 125, 142, 0}, {105, 105, 105, 0}, {240, 240, 240, 0} };
	int grid_cell_size = 20;
	int grid_width = 100, grid_height = 100;
	int window_height = (grid_cell_size * camSize) + 1;
	int window_width = (grid_cell_size * camSize) + 1;
	SDL_Window *window;
	SDL_Renderer *renderer;
	SDL_bool quit = SDL_FALSE;

	

	world2D() {}
	~world2D() {
		SDL_DestroyRenderer(renderer);
		SDL_DestroyWindow(window);
		SDL_Quit();
	}
	 
	//run the simulation 
	void run(int turns) {
		loadWorld();
		renderCameraPerspective();
		renderUI();
		simLoop(turns);
	}

	//Algorithm to generate terrain using n_size footprint
	int loadWorld() {
		int groundCoord[100][2], groundCount = 0;
		int waterCoord[100][2], waterCount = 0;
		random_device rd;           //Will be used to obtain a seed for the random number engine
		mt19937 gen(rd());          //Standard mersenne_twister_engine seeded with rd()
		uniform_real_distribution<> disSeedOnMap(0.4, 0.66);        //Distribution for seed probability on the map
		uniform_real_distribution<> disSeedInChunk(0.0, 11.0);		//Distribution for seed within selected chunk
		uniform_real_distribution<> disWaterSeed(0.0, 2.0);			//Distribution for water seed within the rest of the chunks
		double baseCoef = (double)((int)(disSeedOnMap(gen) * 100)) / 100;
		double waterCoef = (double)((int)(disWaterSeed(gen) * 100)) / 100;

		//marks which chunks will hold a seed for the world gen to use later
		for (int i = 0; i < chunksSize; i++) {
			for (int j = 0; j < chunksSize; j++) {
				double seedProb = (double)((int)(disSeedOnMap(gen) * 100)) / 100;
				//determines if the chunk gets a ground seed
				if (seedProb <= baseCoef) {
					chunks[i][j] = 1;
					//place ground seed inside chunk
					m[i * chunksSize + (int)disSeedInChunk(gen)][j * chunksSize + (int)disSeedInChunk(gen)][0] = 1;
					//save seed coord
					groundCoord[groundCount][0] = i * chunksSize + (int)disSeedInChunk(gen); //y
					groundCoord[groundCount][1] = j * chunksSize + (int)disSeedInChunk(gen); //x
					groundCount++;
				}
				//else it gets a water seed
				else {
					chunks[i][j] = -1;
					//place ground seed inside chunk
					m[i * chunksSize + (int)disSeedInChunk(gen)][j * chunksSize + (int)disSeedInChunk(gen)][0] = -1;
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
				for (int y2 = y - t; y2 <= 2*y; y2++) {
					for (int x2 = x - t; x2 <= 2*x; x2++) {
						if ((y2 >= 0 && y2 < mSize) && (x2 >= 0 && x2 < mSize)) {
							if (t == time - 1) {
								int var = (int)disHeight(gen);
								m[y2][x2][0] += var;
							} else	m[y2][x2][0] += 1;
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
				for (int y2 = y - t; y2 <= 2*y; y2++) {
					for (int x2 = x - t; x2 <= 2*x; x2++) {
						if ((y2 >= 0 && y2 < mSize) && (x2 >= 0 && x2 < mSize)) {
							if (t == time2 - 1) {
								int var = (int)disDeep(gen);
								m[y2][x2][0] -= var;
							}
							else	m[y2][x2][0] -= 1;
						}
					}
				}
			}
		}

		//apple
		spawnItems(0.05, 13, 1);
		//fish
		spawnItems(0.05, 9, 2);
		//wood
		spawnItems(0.05, 14, 3);
		//rocks
		spawnItems(0.05, 16, 4);

		//spawn p1 and set camera bounds
		p1.col = mSize/2-1; p1.row = mSize/2-1; p1.down;
		p1.curChunk_x = p1.col / 10, p1.curChunk_y = p1.row / 10;
		p1.camBound_xMin = p1.col - chunksSize;
		p1.camBound_xMax = p1.col + chunksSize+1;
		p1.camBound_yMin = p1.row - chunksSize;
		p1.camBound_yMax = p1.row + chunksSize+1;

		//SDL init checks 
		if (SDL_Init(SDL_INIT_VIDEO) < 0) {
			SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Initialize SDL: %s",
				SDL_GetError());
			return EXIT_FAILURE;
		}
		if (SDL_CreateWindowAndRenderer(window_width, window_height+60, 0, &window, &renderer) < 0) {
			SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
				"Create window and renderer: %s", SDL_GetError());
			return EXIT_FAILURE;
		}
		
		loadHumans();

		return 0;
	}

	//loads human NPCs
	void loadHumans() {
		random_device rd;           //Will be used to obtain a seed for the random number engine
		mt19937 gen(rd());          //Standard mersenne_twister_engine seeded with rd()
		uniform_real_distribution<> numOfNPC(8, 17);
		double numberOfNPCs = (int)(numOfNPC(gen));
		cout << numberOfNPCs << endl;
		for (int i = 0; i < numberOfNPCs; i++) {
			humanList.push_back(*(new human));
		}
		for (int i = 0; i < humanList.size(); i++) {
			setSpawn(&humanList[i], i);
		}
	}

	//set spawn coordinates
	int setSpawn(human *npc, int mult) {
		random_device rd;           //Will be used to obtain a seed for the random number engine
		mt19937 gen(rd());          //Standard mersenne_twister_engine seeded with rd()
		uniform_real_distribution<> disSpawn(0, 1);  
		
		
		for (int i = 0+3*mult; i < mSize; i++) {
			for (int j = 0*mult+mult; j < mSize; j++) {

				if (m[i][j][0] >= -5 && m[i][j][0] <= 25) {
					double NPCspawn = (disSpawn(gen));
					if (NPCspawn <= 0.00075) {
						m[i][j][2] = 1;
						npc->setY(i);
						npc->setX(j);
						return 1;
					}
				}
			}
		}
	}

	//control center for NPCs
	void controlNPCs() {
		for (int i = 0; i < humanList.size(); i++) {
			human *npc = &humanList[i];
			if (npc->isDead == false) {
				escapeDanger(npc);
				if (npc->hunger < 3.0) {
					npc->eat();
				}
				if (npc->dangerDeltaX != NULL || npc->dangerDeltaY != NULL) {
					moveNPCs(npc, npc->col - npc->dangerDeltaX, npc->row - npc->dangerDeltaY);
				}
				else if (npc->decisionCoef <= 0.7) {
					findFood(npc);
					moveNPCs(npc, npc->foodTargetX, npc->foodTargetY);
				}
				else {
					findMat(npc);
					moveNPCs(npc, npc->matTargetX, npc->matTargetY);
				}
				updateStatsNPC(npc);
				npc->print();
			}
		}
		cout << "=========================" << endl;
	}

	//move each NPC
	void moveNPCs(human *npc, int targetX, int targetY) {
		random_device rd;           
		mt19937 gen(rd());          
		uniform_real_distribution<> disDir(0, 4);
		findFood(npc);
		m[npc->row][npc->col][2] = 0;
		if (targetX == -1 || targetY == -1) {
			int dir = (disDir(gen));
			npc->move(dir);
		}
		else {
			if (npc->row < targetY) {
				npc->move(2);
			}
			else if (npc->row > targetY) {
				npc->move(0);
			}
			else if (npc->col < targetX) {
				npc->move(1);
			}
			else if (npc->col > targetX) {
				npc->move(3);
			}
		}
		if ((npc->row == targetY) && (npc->col == targetX)) {
			if (npc->inventoryLoad == npc->inventorySize) {
				npc->dropItem();
			}
			npc->foodTargetX = -1;
			npc->foodTargetY = -1;
			npc->matTargetX = -1;
			npc->matTargetY = -1;
			objectPickupNPC(npc);
		}
		m[npc->row][npc->col][2] = 1;
		npc->setView();
	}

	//find food sets target
	void findFood(human *npc) {
		for (int i = npc->viewY; i < npc->viewY + npc->viewRange*2; i++) {
			for (int j = npc->viewX; j < npc->viewX + npc->viewRange * 2; j++) {
				if ((i >= 0 || i <= 99) && (j >= 0 || j <= 99)) {
					if (m[i][j][1] == 1 || m[i][j][1] == 2) {
						npc->foodTargetY = i;
						npc->foodTargetX = j;
					}
				}
			}
		}
	}

	//find material
	void findMat(human *npc) {
		for (int i = npc->viewY; i < npc->viewY + npc->viewRange * 2; i++) {
			for (int j = npc->viewX; j < npc->viewX + npc->viewRange * 2; j++) {
				if ((i >= 0 || i <= 99) && (j >= 0 || j <= 99)) {
					if ((m[i][j][1] == 3 ) || (m[i][j][1] == 4)) {
						npc->matTargetY = i;
						npc->matTargetX = j;
					} 
				}
			}
		}
	}

	//escape danger
	void escapeDanger(human *npc) {
		for (int i = npc->viewY; i < npc->viewY + npc->viewRange * 2; i++) {
			for (int j = npc->viewX; j < npc->viewX + npc->viewRange * 2; j++) {
				if ((i >= 0 || i <= 99) && (j >= 0 || j <= 99)) {
					if (m[i][j][2] == 2) {	//jaguar
						npc->dangerDeltaY = npc->row - i;
						npc->dangerDeltaX = npc->col - j;
					}
				}
			}
		}
	}

	//update stats of npc
	void updateStatsNPC(human *npc) {
		if (npc->hearts == 0.0 && npc->isDead == false) {
			//cout << "npc died" << endl;
			m[npc->row][npc->col][2] = 0;
			npc->isDead = true;
		}
		if (npc->isDead == false) {
			if (npc->hunger == 0.0) {
				npc->hearts -= 1.0;
			}
			if (npc->hungerCountdown == 0) {
				npc->hunger -= 1.0;
				npc->decisionCoef -= 0.3;
				npc->hungerCountdown = 14;
			}
			else npc->hungerCountdown--;
		}
	}

	//Print world to console. For debug purposes.
	void renderCameraPerspective() {
		
		//SDL_RenderClear(renderer); //clear screen
		//Rendering individual squares. Color changes based on value.
		int x = 0, y = 0;
		for (int i = p1.curChunk_y*chunksSize; i < p1.curChunk_y*chunksSize+camSize; i++) {
			x = 0;
			for (int j = p1.curChunk_x*chunksSize; j < p1.curChunk_x*chunksSize+camSize; j++) {
				SDL_Rect tile = { (x * grid_cell_size), (y * grid_cell_size), grid_cell_size, grid_cell_size };
				int colorPos = tileComparator(m[i][j][0]);
				SDL_SetRenderDrawColor(renderer, palette[colorPos].r, palette[colorPos].g, palette[colorPos].b, 0);
				SDL_RenderFillRect(renderer, &tile);
				if (m[i][j][1] == 1) {
					SDL_Rect apple = { (x * grid_cell_size)+5, (y * grid_cell_size)+5, 5, 5 };
					SDL_SetRenderDrawColor(renderer, 255, 17, 0, 0);
					SDL_RenderFillRect(renderer, &apple);
				}
				else if (m[i][j][1] == 2) {
					SDL_Rect fish = { (x * grid_cell_size) + 5, (y * grid_cell_size) + 5, 5, 5 };
					SDL_SetRenderDrawColor(renderer, 251, 212, 185, 0);
					SDL_RenderFillRect(renderer, &fish);
				}
				else if (m[i][j][1] == 3) {
					SDL_Rect wood = { (x * grid_cell_size) + 5, (y * grid_cell_size) + 5, 5, 5 };
					SDL_SetRenderDrawColor(renderer, 165, 113, 78, 0);
					SDL_RenderFillRect(renderer, &wood);
				}
				else if (m[i][j][1] == 4) {
					SDL_Rect rock = { (x * grid_cell_size) + 5, (y * grid_cell_size) + 5, 5, 5 };
					SDL_SetRenderDrawColor(renderer, 128, 128, 128, 0);
					SDL_RenderFillRect(renderer, &rock);
				}
				if (m[i][j][2] == 1) {
					SDL_Rect npc = { (x * grid_cell_size) + 5, (y * grid_cell_size) + 5,
						grid_cell_size/2, grid_cell_size/2 };
					SDL_SetRenderDrawColor(renderer, 255, 127, 80, 0);
					SDL_RenderFillRect(renderer, &npc);
				}
				x++;
			}
			y++;
		}

		renderPlayer();

		//set line color for rendering
		SDL_Color grid_line_color = { 255, 255, 255, 255 }; //White
		SDL_SetRenderDrawColor(renderer, grid_line_color.r, grid_line_color.g,
			grid_line_color.b, grid_line_color.a);

		//draw vertical lines in backbuffer
		for (int x = 0; x < 1 + grid_cell_size * grid_cell_size; x += grid_cell_size) {
			SDL_RenderDrawLine(renderer, x, 0, x, window_height);
		}
		//draw horizontal lines in backbuffer
		for (int y = 0; y < 1 + grid_cell_size * grid_cell_size; y += grid_cell_size) {
			SDL_RenderDrawLine(renderer, 0, y, window_width, y);
		}
		
		SDL_RenderPresent(renderer); //Render backbuffer
	}

	//returns a value representing a tile
	int tileComparator(int x) {
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

	//Renders player camera perspective
	void renderPlayer() {
		SDL_Rect playerRender = { ((p1.col-p1.curChunk_x*chunksSize)*grid_cell_size)+grid_cell_size/4, 
			((p1.row-p1.curChunk_y*chunksSize)*grid_cell_size)+grid_cell_size/4, 
			grid_cell_size / 2, grid_cell_size / 2 };
		SDL_SetRenderDrawColor(renderer, 228, 0, 224, 0);
		SDL_RenderFillRect(renderer, &playerRender);
	}

	//Process player input and schedules world updates
	void simLoop(int a) {
		while (!quit) {
			inputReader(a);
			if (a == 0) {
				break;
			}		
		}
		quit = SDL_TRUE;
	}

	//Move camera when needed
	void updateCamera() {
		if (p1.col <= p1.camBound_xMin) {
			p1.camBound_xMin -= 2 * chunksSize;
			p1.camBound_xMax -= 2 * chunksSize;
			p1.curChunk_x-=2;
		} 
		else if (p1.col >= p1.camBound_xMax) {
			p1.camBound_xMin += 2 * chunksSize;
			p1.camBound_xMax += 2 * chunksSize;
			p1.curChunk_x+=2;
		}
		else if (p1.row <= p1.camBound_yMin) {
			p1.camBound_yMin -= 2 * chunksSize;
			p1.camBound_yMax -= 2 * chunksSize;
			p1.curChunk_y-=2;
		}
		else if (p1.row >= p1.camBound_yMax) {
			p1.camBound_yMin += 2 * chunksSize;
			p1.camBound_yMax += 2 * chunksSize;
			p1.curChunk_y+=2;
		}
	}

	//Display world map when pressing m, toggle off by pressing m again. Movement disabled.
	void displayWorldMap() {
		SDL_RenderClear; //clear screen
		
		int grid_cell_size2 = 4, mapSize = 100;
		
		for (int i = 0; i < mapSize; i++) {
			for (int j = 0; j < mapSize; j++) {
				SDL_Rect tile = { j*grid_cell_size2, i*grid_cell_size2, grid_cell_size2, grid_cell_size2 };
				if (i == p1.row && j == p1.col) {
					SDL_SetRenderDrawColor(renderer, 228, 0, 224, 0);
				}
				else {
					int colorPos = tileComparator(m[i][j][0]);
					SDL_SetRenderDrawColor(renderer, palette[colorPos].r, palette[colorPos].g, palette[colorPos].b, 0);
				}
				SDL_RenderFillRect(renderer, &tile);
			}
		}
		SDL_RenderPresent(renderer); //Render backbuffer
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

	//spawn items
	void spawnItems(double prob, int tileID, int type) {
		random_device rd;           
		mt19937 gen(rd());
		uniform_real_distribution<> disItem(0.0, 1.0);
		for (int i = 0; i < mSize; i++) {
			for (int j = 0; j < mSize; j++) {
				int val = m[i][j][0];
				if (tileComparator(val) == tileID) {
					double itemChance = (double)((int)(disItem(gen) * 100)) / 100;
					if (itemChance <= prob) {
						m[i][j][1] = type;
					}
				}
			}
		}
	}

	//object pickup
	void objectPickup() {
		if (p1.inventoryLoad < p1.inventorySize && m[p1.row][p1.col][1] != 0) {
			for (int i = 0; i < p1.inventorySize; i++) {
				if (p1.inventory[i] == 0) {
					p1.inventory[i] = m[p1.row][p1.col][1];
					m[p1.row][p1.col][1] = 0;
					p1.inventoryLoad++;
					break;
				}
			}
		}
	}

	//object pickup NPC version
	void objectPickupNPC(human *npc) {
		if (npc->inventoryLoad < npc->inventorySize && m[npc->row][npc->col][1] != 0) {
			for (int i = 0; i < npc->inventorySize; i++) {
				if (npc->inventory[i] == 0) {
					npc->inventory[i] = m[npc->row][npc->col][1];
					m[npc->row][npc->col][1] = 0;
					npc->inventoryLoad++;
					break;
				}
			}
		}
	}

	//main input handler
	void inputReader(int a){
		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT) {
				quit = SDL_TRUE;
			}
			else if (event.type == SDL_KEYDOWN) {
				switch (event.key.keysym.sym) {
					case SDLK_UP:
						turnHandler(0, a);
						break;
					case SDLK_RIGHT:
						turnHandler(1, a);
						break;
					case SDLK_DOWN:
						turnHandler(2, a);
						break;
					case SDLK_LEFT:
						turnHandler(3, a);
						break;
					case SDLK_m:
						displayWorldMap();
						break;
					case SDLK_e:
						eat();
						turnHandler(-1, a);
						break;
					case SDLK_s:
						selectItems();
						turnHandler(-1, a);
						break;
					case SDLK_a:
						attack();
						turnsElapsed++;
						turnHandler(-1, a--);
						break;
				}
			}
			else continue;
		}
	}

	//turn event handler
	void turnHandler(int dir, int a) {
		if (dir != -1) {
			p1.walk(dir);
			objectPickup();
			updateCamera();
			a--;
			turnsElapsed++;
			if (turnsElapsed == 19) {
				spawnControl();
				turnsElapsed = 0;
			}
			clearScreen();
			renderCameraPerspective();
		}
		else if (dir == -1) {
			clearScreen();
			renderCameraPerspective();
		}
		if (turnsElapsed == 19) {
			spawnControl();
			turnsElapsed = 0;
		}
		if (p1.spear == true) {
			renderImage();
		}
		controlNPCs();
		trackStats();
		renderUI();
	}

	//renderUI
	void renderUI() {
		renderInventory();
		renderBars(p1.hearts, 120, 405, grid_cell_size, grid_cell_size / 2, 255, 17, 0);		//render health bar - red
		renderBars(p1.hunger, 120, 425, grid_cell_size, grid_cell_size / 2, 255, 105, 180);		//render hunger bar - pink
	}

	//render inventory
	void renderInventory() {
		for (int i = 0; i < p1.inventorySize; i++) {
			renderSlot(p1.inventory[i], i);
		}
	}

	//render slot
	void renderSlot(int n, int i) {
		switch (n) {
			case 1:
				renderRect(i*grid_cell_size, grid_cell_size*grid_cell_size + 1, grid_cell_size,
					grid_cell_size, 255, 17, 0);
				break;
			case 2:
				renderRect(i*grid_cell_size, grid_cell_size*grid_cell_size + 1, grid_cell_size,
					grid_cell_size, 251, 212, 185);
				break;
			case 3:
				renderRect(i*grid_cell_size, grid_cell_size*grid_cell_size + 1, grid_cell_size,
					grid_cell_size, 165, 113, 78);
				break;
			case 4:
				renderRect(i*grid_cell_size, grid_cell_size*grid_cell_size + 1, grid_cell_size,
					grid_cell_size, 128, 128, 128);
				break;
			case 0:
				renderRect(i*grid_cell_size, grid_cell_size*grid_cell_size + 1, grid_cell_size,
					grid_cell_size, 0, 0, 0);
				break;
		}
	}

	//render filled rectangle
	void renderRect(int x, int y, int w, int h, int r, int g, int b) {
		SDL_Rect rect = {x, y, w, h};
		SDL_SetRenderDrawColor(renderer, r, g, b, 0);
		SDL_RenderFillRect(renderer, &rect);
		SDL_RenderPresent(renderer); //Render backbuffer
	}

	//render empty rectangle
	void renderEmptyRect(int x, int y, int w, int h, int r, int g, int b) {
		SDL_Rect rect = {x, y, w, h};
		SDL_SetRenderDrawColor(renderer, r, g, b, 0);
		SDL_RenderDrawRect(renderer, &rect);
		SDL_RenderPresent(renderer); //Render backbuffer
	}

	//render health and hunger bars
	void renderBars(int max, int x0, int y0, int w, int h, int r, int g, int b) {
		for (int i = 0; i < max; i++) {
			renderRect(x0+i*grid_cell_size, y0, w, h, r, g, b);
		}
	}

	//manage stat reduction on turn
	void statDelta(int &stat, int delta) {
		stat+=delta;
	}

	//clear creen with black color
	void clearScreen() {
		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
		SDL_RenderClear(renderer); 
	}

	//track status numbers
	void trackStats() {
		statDelta(p1.hungerCountdown, -1);
		if (p1.hearts < 3 && p1.hunger == 3) {
			statDelta(p1.hearts, 1);
		}
		if (p1.hungerCountdown == 0) {
			statDelta(p1.hunger, -1);
			p1.hungerCountdown = 14;
		}
		if (p1.hunger == 0) {
			if (p1.hearts == 1) {
				//quit = SDL_TRUE;
				cout << "you died" << endl;
			} else statDelta(p1.hearts, -1);
		}
	}

	//eat a fish or apple is possible - should be a player method
	void eat() {
		if (p1.hunger < 3) {
			for (int i = 0; i < p1.inventorySize; i++) {
				if (p1.inventory[i] == 1 || p1.inventory[i] == 2) {
					p1.inventory[i] = 0;
					p1.inventoryLoad--;
					p1.hunger++;
					p1.hungerCountdown = 15;
					break;
				}
			}
		}
		else cout << "you are not hungry..." << endl;
	}

	//spawn items during simulation
	void spawnControl() {
		//apple
		spawnItems(0.0001, 13, 1);
		//fish
		spawnItems(0.0001, 9, 2);
		//wood
		spawnItems(0.0001, 14, 3);
		//rocks
		spawnItems(0.0001, 16, 4);
	}

	//select method
	void selectItems() {
		int flag1 = 0, flag2 = 0, flag3 = 0, flag4 = 0, flag5 = 0;	//flags to test if item has been selected
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
								p1.selectedItems.push_back(0);
								flag1 = 1;
								renderEmptyRect(0*grid_cell_size, grid_cell_size*grid_cell_size, grid_cell_size,
									grid_cell_size, 255, 255, 255);
							}
							else if (flag1 == 1) {
								flag1 = 0;
								renderSlot(p1.inventory[0], 0);
								remove_copy(p1.selectedItems.begin(), p1.selectedItems.end(), p1.selectedItems.begin(), 0);
							}
							break;
						case SDLK_2:
							if (flag2 == 0) {
								p1.selectedItems.push_back(1);
								flag2 = 1;
								renderEmptyRect(1 * grid_cell_size, grid_cell_size*grid_cell_size, grid_cell_size,
									grid_cell_size, 255, 255, 255);
							}
							else if (flag2 == 1) {
								flag2 = 0;
								renderSlot(p1.inventory[1], 1);
								remove_copy(p1.selectedItems.begin(), p1.selectedItems.end(), p1.selectedItems.begin(), 1);
							}
							break;
						case SDLK_3:
							if (flag3 == 0) {
								p1.selectedItems.push_back(2);
								flag3 = 1;
								renderEmptyRect(2 * grid_cell_size, grid_cell_size*grid_cell_size, grid_cell_size,
									grid_cell_size, 255, 255, 255);
							}
							else if (flag3 == 1) {
								flag3 = 0;
								renderSlot(p1.inventory[2], 2);
								remove_copy(p1.selectedItems.begin(), p1.selectedItems.end(), p1.selectedItems.begin(), 2);
							}
							break;
						case SDLK_4:
							if (flag4 == 0) {
								p1.selectedItems.push_back(3);
								flag4 = 1;
								renderEmptyRect(3 * grid_cell_size, grid_cell_size*grid_cell_size, grid_cell_size,
									grid_cell_size, 255, 255, 255);
							}
							else if (flag4 == 1) {
								flag4 = 0;
								renderSlot(p1.inventory[3], 3);
								remove_copy(p1.selectedItems.begin(), p1.selectedItems.end(), p1.selectedItems.begin(), 3);
							}
							break;
						case SDLK_5:
							if (flag5 == 0) {
								p1.selectedItems.push_back(4);
								flag5 = 1;
								renderEmptyRect(4 * grid_cell_size, grid_cell_size*grid_cell_size, grid_cell_size,
									grid_cell_size, 255, 255, 255);
							}
							else if (flag5 == 1) {
								flag5 = 0;
								renderSlot(p1.inventory[4], 4);
								remove_copy(p1.selectedItems.begin(), p1.selectedItems.end(), p1.selectedItems.begin(), 4);
							}
							break;
						case SDLK_s:
							p1.selectedItems.clear();
							renderUI();
							quit = SDL_TRUE;
							break;
						case SDLK_d:
							for (auto i: p1.selectedItems) {
								p1.inventory[i] = 0;
								p1.inventoryLoad--;
							}
							p1.selectedItems.clear();
							renderUI();
							break;
						case SDLK_c:
							crafting();
							p1.selectedItems.clear();
							renderUI();
							break;

					}
				}
			}
		}
		quit = SDL_FALSE;
	}

	//crafting method - can only craft a spear right now
	void crafting() {
		int rocksForSpear = 1, woodForSpear = 2, isFish = 0, isApple = 0;
		for (auto i : p1.selectedItems) {
			if (p1.inventory[i] == 3) {
				woodForSpear--;
			}
			else if (p1.inventory[i] == 4) {
				rocksForSpear--;
			}
			else if (p1.inventory[i] == 1) {
				isApple++;
			}
			else if (p1.inventory[i] == 2) {
				isFish++;
			}
		}
		if (rocksForSpear == 0 && woodForSpear == 0 && isApple == 0 && isFish == 0)  {
			p1.spear = true;
			for (auto i : p1.selectedItems) {
				p1.inventory[i] = 0;
				p1.inventoryLoad--;
			}
			renderImage();
		}
	}

	//renders image bmp - hardcoded tmp
	void renderImage() {
		SDL_Surface* icon = SDL_LoadBMP("Assets/spear.bmp");
		SDL_Texture* texture = NULL;
		SDL_Rect rect= { 0, 430, 24, 24 };
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

	//attack method
	void attack() {
		while (!quit) {
			SDL_Event event;
			while (SDL_PollEvent(&event)) {
				if (event.type == SDL_QUIT) {
					quit = SDL_TRUE;
				}
				else if (event.type == SDL_KEYDOWN) {
					switch (event.key.keysym.sym) {
						case SDLK_UP:
							damageNPC(p1.col, p1.row - 1, p1.spear);
							if (p1.spear == true) {
								renderSpearUsage(p1.col*grid_cell_size, p1.row*grid_cell_size, 0);
								SDL_Delay(1000);

							}
							quit = SDL_TRUE;
							break;
						case SDLK_RIGHT:
							damageNPC(p1.col + 1, p1.row, p1.spear);
							if (p1.spear == true) {
								renderSpearUsage(p1.col*grid_cell_size, p1.row*grid_cell_size, 1);
								SDL_Delay(1000);
							}
							quit = SDL_TRUE;
							break;
						case SDLK_DOWN:
							damageNPC(p1.col, p1.row + 1, p1.spear);
							if (p1.spear == true) {
								renderSpearUsage(p1.col*grid_cell_size, p1.row*grid_cell_size, 2);
								SDL_Delay(1000);
							}
							quit = SDL_TRUE;
							break;
						case SDLK_LEFT:
							damageNPC(p1.col - 1, p1.row, p1.spear);
							if (p1.spear == true) {
								renderSpearUsage(p1.col*grid_cell_size, p1.row*grid_cell_size, 3);
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

	//damage NPC
	void damageNPC(int x, int y, bool spear) {
		int mult = 1;
		if (spear == true) {
			mult = 2;
		}
		if (m[y][x][2] != 0) {
			//Todo: apply damage to NPC, use mult as multiplier
		}
	}

	//renders spear in attack direction
	void renderSpearUsage(int x, int y, int dir) {
		SDL_Surface* icon = SDL_LoadBMP("Assets/spearUsed.bmp");
		SDL_Texture* texture = NULL;
		SDL_Rect rect;
		switch (dir) {
			case 0:
				rect = { (p1.col - p1.curChunk_x*chunksSize)*grid_cell_size, 
					((p1.row - p1.curChunk_y*chunksSize)*grid_cell_size) - 20, 24, 24 };
				texture = SDL_CreateTextureFromSurface(renderer, icon);
				SDL_FreeSurface(icon);
				SDL_RenderCopy(renderer, texture, NULL, &rect);
				break;
			case 1:
				rect = { (p1.col - p1.curChunk_x*chunksSize)*grid_cell_size + 20,
					((p1.row - p1.curChunk_y*chunksSize)*grid_cell_size), 24, 24 };
				texture = SDL_CreateTextureFromSurface(renderer, icon);
				SDL_FreeSurface(icon);
				SDL_RenderCopyEx(renderer, texture, NULL, &rect, 270, NULL, SDL_FLIP_VERTICAL);
				break;
			case 2:
				rect = { (p1.col - p1.curChunk_x*chunksSize)*grid_cell_size,
					((p1.row - p1.curChunk_y*chunksSize)*grid_cell_size) + 20, 24, 24 };
				texture = SDL_CreateTextureFromSurface(renderer, icon);
				SDL_FreeSurface(icon);
				SDL_RenderCopyEx(renderer, texture, NULL, &rect, 180, NULL, SDL_FLIP_HORIZONTAL);
				break;
			case 3:
				rect = { (p1.col - p1.curChunk_x*chunksSize)*grid_cell_size - 20,
					((p1.row - p1.curChunk_y*chunksSize)*grid_cell_size), 24, 24 };
				texture = SDL_CreateTextureFromSurface(renderer, icon);
				SDL_FreeSurface(icon);
				SDL_RenderCopyEx(renderer, texture, NULL, &rect, 90, NULL, SDL_FLIP_VERTICAL);
				break;
		}
		
		SDL_RenderPresent(renderer);
	}
};

int main() {
	int n = 5000;	//turns 
	world2D test;
	test.run(n);
	return 0;
}