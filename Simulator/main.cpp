#include <iostream>
#include <random>
#include <SDL.h>
#undef main


using namespace std;

class player {
public:
	int row, col;	//player coordinates
	int curChunk_x, curChunk_y;		//top left camera chunk
	int camBound_xMin, camBound_xMax, camBound_yMin, camBound_yMax;	//camera bounds
	enum direction { up = 0, right = 1, down = 2, left = 3 };		//orientation -> not implemented

	player() {}
	~player() {}

	void walk(int dir) {
		switch (dir) {
			case 0:
				this->row--;
				break;
			case 1:
				this->col++;
				break;
			case 2:
				this->row++;
				break;
			case 3:
				this->col--;
				break;
		}
	}
};

class world2D {
public:
	int m[100][100] = { 0 }, mSize = size(m);		//Full map and map side size
	int chunks[10][10], chunksSize = size(chunks);	//Chunks map and chunks side size
	int camSize = size(chunks)*2;		//Camera view size
	player p1;

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
	 
	void run(int turns) {
		this->loadWorld();
		this->renderCameraPerspective();
		this->simLoop(turns);
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
					m[i * chunksSize + (int)disSeedInChunk(gen)][j * chunksSize + (int)disSeedInChunk(gen)] = 1;
					//save seed coord
					groundCoord[groundCount][0] = i * chunksSize + (int)disSeedInChunk(gen); //y
					groundCoord[groundCount][1] = j * chunksSize + (int)disSeedInChunk(gen); //x
					groundCount++;
				}
				//else it gets a water seed
				else {
					chunks[i][j] = -1;
					//place ground seed inside chunk
					m[i * chunksSize + (int)disSeedInChunk(gen)][j * chunksSize + (int)disSeedInChunk(gen)] = -1;
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
								m[y2][x2] += var;
							} else	m[y2][x2] += 1;
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
								m[y2][x2] -= var;
							}
							else	m[y2][x2] -= 1;
						}
					}
				}
			}
		}

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
		if (SDL_CreateWindowAndRenderer(window_width, window_height, 0, &window, &renderer) < 0) {
			SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
				"Create window and renderer: %s", SDL_GetError());
			return EXIT_FAILURE;
		}
		return 0;
	}

	//Print world to console. For debug purposes.
	void renderCameraPerspective() {
		
		//Rendering individual squares. Color changes based on value.
		int x = 0, y = 0;
		for (int i = p1.curChunk_y*chunksSize; i < p1.curChunk_y*chunksSize+camSize; i++) {
			x = 0;
			for (int j = p1.curChunk_x*chunksSize; j < p1.curChunk_x*chunksSize+camSize; j++) {
				SDL_Rect tile = { (x * grid_cell_size), (y * grid_cell_size), grid_cell_size, grid_cell_size };
				int colorPos = tileColor(m[i][j]);
				SDL_SetRenderDrawColor(renderer, palette[colorPos].r, palette[colorPos].g, palette[colorPos].b, 0);
				SDL_RenderFillRect(renderer, &tile);
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
		for (int x = 0; x < 1 + grid_width * grid_cell_size; x += grid_cell_size) {
			SDL_RenderDrawLine(renderer, x, 0, x, window_height);
		}
		//draw horizontal lines in backbuffer
		for (int y = 0; y < 1 + grid_height * grid_cell_size; y += grid_cell_size) {
			SDL_RenderDrawLine(renderer, 0, y, window_width, y);
		}

		SDL_RenderClear; //clear screen
		SDL_RenderPresent(renderer); //Render backbuffer
	}

	//returns position of correct palette color for a given tile 
	int tileColor(int x) {
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
		SDL_Rect playerRender = { ((this->p1.col-this->p1.curChunk_x*chunksSize)*grid_cell_size), 
			((this->p1.row-this->p1.curChunk_y*chunksSize)*grid_cell_size), 
			grid_cell_size / 2, grid_cell_size / 2 };
		SDL_SetRenderDrawColor(renderer, 228, 0, 224, 0);
		SDL_RenderFillRect(renderer, &playerRender);
	}

	//Process player input and schedules world updates
	void simLoop(int a) {
		while (!quit) {
			SDL_Event event;
			while (SDL_PollEvent(&event)) {
				if (event.type == SDL_QUIT) {
					quit = SDL_TRUE;
				}
				else if (event.type == SDL_KEYDOWN) {
					switch (event.key.keysym.sym) {
						case SDLK_UP:
							p1.walk(0);
							updateCamera();
							renderCameraPerspective();
							a--;
							break;
						case SDLK_RIGHT:
							p1.walk(1);
							updateCamera();
							renderCameraPerspective();
							a--;
							break;
						case SDLK_DOWN:
							p1.walk(2);
							updateCamera();
							renderCameraPerspective();
							a--;
							break;
						case SDLK_LEFT:
							p1.walk(3);
							updateCamera();
							renderCameraPerspective();
							a--;
							break;
						case SDLK_m:
							displayWorldMap();
						}
				}
				else continue;
			}
			if (a == 0) {
				break;
			}		
		}
		quit = SDL_TRUE;
	}

	//Recenter camera when needed
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
					int colorPos = tileColor(m[i][j]);
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
};

int main() {
	int n = 5000;	//turns 
	world2D test;
	test.run(n);
	return 0;
}