#include <iostream>
//#include <conio.h>
#include <random>
#include <SDL.h>
#undef main


using namespace std;
/*const int KEY_ARROW_CHAR1 = 224;
const int KEY_UP = 72;
const int KEY_RIGHT = 77;
const int KEY_DOWN = 80;
const int KEY_LEFT = 75;*/

class player {
public:
	int row, col;
	enum direction { up = 0, right = 1, down = 2, left = 3 };

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
	int m[20][20] = { 0 };
	int spot;
	player p1;

	SDL_Color palette[21] = { {25, 25, 112, 0}, {0, 0, 128, 0}, {0, 0, 205, 0}, {0, 0, 225, 0}, {0, 0, 255, 0}, {45, 100, 245, 0},
								{51, 171, 240, 0}, {82, 219, 255, 0}, {110, 255, 255, 0}, {168, 255, 255}, {227, 220, 192, 0},
								{219, 173, 114, 0}, {202, 143, 66, 0}, {124, 252, 0, 0}, {34, 139, 34, 0}, {0, 100, 0, 0},
								{193, 210, 214, 0}, {174, 187, 199, 0}, {106, 125, 142, 0}, {105, 105, 105, 0}, {240, 240, 240, 0} };
	int grid_cell_size = 20;
	int grid_width = 20, grid_height = 20;
	int window_height = (grid_cell_size * grid_height) + 1;
	int window_width = (grid_cell_size * grid_width) + 1;
	SDL_Color grid_line_color = { 255, 255, 255, 255 }; //White
	SDL_Window *window;
	SDL_Renderer *renderer;
	SDL_bool quit = SDL_FALSE;
	

	world2D() {}
	~world2D() {
		SDL_DestroyRenderer(renderer);
		SDL_DestroyWindow(window);
		SDL_Quit();
	}

	//Algorithm to generate terrain TODO: Refine
	int loadWorld() {
		//fill 2d array
		int chunks[10][10];
		int coord[100][2], count = 0;
		int waterCoord[100][2], waterCount = 0;
		random_device rd;           //Will be used to obtain a seed for the random number engine
		mt19937 gen(rd());          //Standard mersenne_twister_engine seeded with rd()
		uniform_real_distribution<> disSeedOnMap(0.2f, 1.0);         //Distribution for seed probability
		uniform_real_distribution<> disSeedInChunk(0.0, 2.0);        //Distribution for seed within chunk
		uniform_real_distribution<> disWaterSeed(0.0, 2.0);        //Distribution for water seed
		double baseCoef = (double)((int)(disSeedOnMap(gen) * 100)) / 100;
		double waterCoef = (double)((int)(disWaterSeed(gen) * 100)) / 100;

		cout << baseCoef << endl;

		//marks which chunks will hold a seed for the world gen to use later
		for (int i = 0; i < 10; i++) {
			for (int j = 0; j < 10; j++) {
				double seedProb = (double)((int)(disSeedOnMap(gen) * 100)) / 100;
				if (seedProb <= baseCoef) {
					chunks[i][j] = 1;
					m[i * 2 + (int)disSeedInChunk(gen)][j * 2 + (int)disSeedInChunk(gen)] = 1;
					//save seed coord
					coord[count][0] = i * 2 + (int)disSeedInChunk(gen); //y
					coord[count][1] = j * 2 + (int)disSeedInChunk(gen); //x
					count++;
				}
				else {
					chunks[i][j] = -1;
					m[i * 2 + (int)disSeedInChunk(gen)][j * 2 + (int)disSeedInChunk(gen)] = -1;
					//save seed coord
					waterCoord[waterCount][0] = i * 2 + (int)disSeedInChunk(gen); //y
					waterCoord[waterCount][1] = j * 2 + (int)disSeedInChunk(gen); //x
					waterCount++;
				}
				
				/*else {
					double waterProb = (double)((int)(disWaterSeed(gen) * 100)) / 100;
					if (waterProb <= waterCoef) {
						chunks[i][j] = -1;
						m[i * 2 + (int)disSeedInChunk(gen)][j * 2 + (int)disSeedInChunk(gen)] = -1;
						//save seed coord
						waterCoord[waterCount][0] = i * 2 + (int)disSeedInChunk(gen); //y
						waterCoord[waterCount][1] = j * 2 + (int)disSeedInChunk(gen); //x
						waterCount++;
					}
					else chunks[i][j] = 0;
				}*/
			}
		}
		//printing seed map
		/*for (int i = 0; i < 20; i++) {
			for (int j = 0; j < 20; j++) {
				if (m[i][j] >= 0) {
					cout << "| " << m[i][j] << '|';
				}
				else cout << '|' << m[i][j] << '|';
			} cout << endl;
		}
		cout << "-------------------------------------------" << endl;
		*/
		//height variation
		uniform_real_distribution<> disHeight(0, 2.0);
		//time value
		uniform_real_distribution<> disRange(3.0, 7.0);
		for (int n = 0; n < count; n++) {
			int y = coord[n][0], x = coord[n][1];
			double time = (int)disRange(gen);
			for (int t = 0; t < time; t++) {
				for (int y2 = y - t; y2 <= y; y2++) {
					for (int x2 = x - t; x2 <= x; x2++) {
						if ((y2 >= 0 && y2 < 20) && (x2 >= 0 && x2 < 20)) {
							//int var = (int)disHeight(gen);
							if (t == time - 1) {
								int var = (int)disHeight(gen);
							} else	m[y2][x2] += 1;
						}
						//else ignore out of bounds coords
					}
				}
			}
		}
		//deep variation
		uniform_real_distribution<> disDeep(-1.99, 0.0);
		//time2 value
		uniform_real_distribution<> disRange2(3.0, 7.0);
		for (int n = 0; n < waterCount; n++) {
			int y = waterCoord[n][0], x = waterCoord[n][1];
			double time2 = (int)disRange2(gen);
			for (int t = 0; t < time2; t++) {
				for (int y2 = y - t; y2 <= y; y2++) {
					for (int x2 = x - t; x2 <= x; x2++) {
						if ((y2 >= 0 && y2 < 20) && (x2 >= 0 && x2 < 20)) {
							//int var = (int)disDeep(gen);
							if (t == time2 - 1) {
								int var = (int)disDeep(gen);
							}
							else	m[y2][x2] -= 1;
						}
						//else ignore out of bounds coords
					}
				}
			}
		}
		
		//Backup hardcoded fill
		/* for (int i = 0; i < 100; i++) {
			for (int j = 0; j < 100; j++) {
				if ((i < 1) || (i > 8) || (j < 1) || (j > 8)) {
					m[i][j] = 1;
				}
				else m[i][j] = 0;
			}
		}*/

		//place p1 at the center and save overwritten tile
		p1.col = 10; p1.row = 10; p1.down;
		//spot = m[p1.row][p1.col];
		//m[p1.row][p1.col] = 99;

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
	void displayWorld() {
		//diplay text world to console
		for (int i = 0; i < 20; i++) {
			for (int j = 0; j < 20; j++) {
				if (m[i][j] >= 0) {
					cout << "| " << m[i][j] << '|';
				}
				else cout << '|' << m[i][j] << '|';
			} cout << endl;
		}
		cout << "-------------------------------------------" << endl;

		//Rendering individual squares. Color changes based on value.
		for (int i = 0; i < 20; i++) {
			for (int j = 0; j < 20; j++) {
				SDL_Rect tile = { (j * grid_width), (i * grid_width), grid_cell_size, grid_cell_size };
				switch (m[i][j]) {
					case -9:{
						SDL_SetRenderDrawColor(renderer, palette[1].r, palette[1].g, palette[1].b, 0);
						SDL_RenderFillRect(renderer, &tile);
						break;
					}
					case -8: {
						SDL_SetRenderDrawColor(renderer, palette[2].r, palette[2].g, palette[2].b, 0);
						SDL_RenderFillRect(renderer, &tile);
						break;
					}
					case -7: {
						SDL_SetRenderDrawColor(renderer, palette[3].r, palette[3].g, palette[3].b, 0);
						SDL_RenderFillRect(renderer, &tile);
						break;
					}
					case -6: {
						SDL_SetRenderDrawColor(renderer, palette[4].r, palette[4].g, palette[4].b, 0);
						SDL_RenderFillRect(renderer, &tile);
						break;
					}
					case -5: {
						SDL_SetRenderDrawColor(renderer, palette[5].r, palette[6].g, palette[6].b, 0);
						SDL_RenderFillRect(renderer, &tile);
						break;
					}
					case -4: {
						SDL_SetRenderDrawColor(renderer, palette[7].r, palette[7].g, palette[7].b, 0);
						SDL_RenderFillRect(renderer, &tile);
						break;
					}
					case -3: {
						SDL_SetRenderDrawColor(renderer, palette[8].r, palette[8].g, palette[8].b, 0);
						SDL_RenderFillRect(renderer, &tile);
						break;
					}
					case -2: {
						SDL_SetRenderDrawColor(renderer, palette[9].r, palette[9].g, palette[9].b, 0);
						SDL_RenderFillRect(renderer, &tile);
						break;
					}
					case -1: {
						SDL_SetRenderDrawColor(renderer, palette[10].r, palette[10].g, palette[10].b, 0);
						SDL_RenderFillRect(renderer, &tile);
						break;
					}
					case 0: {
						SDL_SetRenderDrawColor(renderer, palette[11].r, palette[11].g, palette[11].b, 0);
						SDL_RenderFillRect(renderer, &tile);
						break;
					}
					case 1: {
						SDL_SetRenderDrawColor(renderer, palette[12].r, palette[12].g, palette[12].b, 0);
						SDL_RenderFillRect(renderer, &tile);
						break;
					}
					case 2: {
						SDL_SetRenderDrawColor(renderer, palette[13].r, palette[13].g, palette[13].b, 0);
						SDL_RenderFillRect(renderer, &tile);
						break;
					}
					case 3: {
						SDL_SetRenderDrawColor(renderer, palette[14].r, palette[14].g, palette[14].b, 0);
						SDL_RenderFillRect(renderer, &tile);
						break;
					}
					case 4: {
						SDL_SetRenderDrawColor(renderer, palette[15].r, palette[15].g, palette[15].b, 0);
						SDL_RenderFillRect(renderer, &tile);
						break;
					}
					case 5: {
						SDL_SetRenderDrawColor(renderer, palette[16].r, palette[16].g, palette[16].b, 0);
						SDL_RenderFillRect(renderer, &tile);
						break;
					}
					case 6: {
						SDL_SetRenderDrawColor(renderer, palette[17].r, palette[17].g, palette[17].b, 0);
						SDL_RenderFillRect(renderer, &tile);
						break;
					}
					case 7: {
						SDL_SetRenderDrawColor(renderer, palette[18].r, palette[18].g, palette[18].b, 0);
						SDL_RenderFillRect(renderer, &tile);
						break;
					}
					case 8: {
						SDL_SetRenderDrawColor(renderer, palette[19].r, palette[19].g, palette[19].b, 0);
						SDL_RenderFillRect(renderer, &tile);
						break;
					}
					case 9: {
						SDL_SetRenderDrawColor(renderer, palette[20].r, palette[20].g, palette[20].b, 0);
						SDL_RenderFillRect(renderer, &tile);
						break;
					}
					/*case 99: {
						SDL_SetRenderDrawColor(renderer, 228, 0, 224, 0);
						SDL_RenderFillRect(renderer, &tile);
						break;
					}*/
					default: {
						if (m[i][j]<=-10){
							SDL_SetRenderDrawColor(renderer, palette[0].r, palette[0].g, palette[0].b, 0);
							SDL_RenderFillRect(renderer, &tile);
						} else {
							SDL_SetRenderDrawColor(renderer, palette[20].r, palette[20].g, palette[20].b, 0);
							SDL_RenderFillRect(renderer, &tile);
						}
						break;
					}
				}
			}
		}

		renderPlayer();

		SDL_SetRenderDrawColor(renderer, grid_line_color.r, grid_line_color.g,
			grid_line_color.b, grid_line_color.a);

		for (int x = 0; x < 1 + grid_width * grid_cell_size;
			x += grid_cell_size) {
			SDL_RenderDrawLine(renderer, x, 0, x, window_height);
		}
		for (int y = 0; y < 1 + grid_height * grid_cell_size;
			y += grid_cell_size) {
			SDL_RenderDrawLine(renderer, 0, y, window_width, y);
		}
		SDL_RenderClear; //clear screen
		SDL_RenderPresent(renderer); //Render backbuffer
		
	}

	//Renders player
	void renderPlayer() {
		SDL_Rect playerRender = { (this->p1.col*grid_cell_size), (this->p1.row*grid_cell_size), grid_cell_size / 2, grid_cell_size / 2 };
		SDL_SetRenderDrawColor(renderer, 228, 0, 224, 0);
		SDL_RenderFillRect(renderer, &playerRender);
	}

	//Process player input and schedules world updates
	void simLoop() {
		int a = 5;
		char input;
		while (!quit) {
			SDL_Event event;
			while (SDL_PollEvent(&event)) {
				if (event.type == SDL_QUIT) {
					quit = SDL_TRUE;
				}
				else if (event.type == SDL_KEYDOWN) {
					//this->updateWorld();
					switch (event.key.keysym.sym) {
						case SDLK_UP:
							this->p1.walk(0);
							this->renderPlayer();
							this->displayWorld();
							a--;
							break;
						case SDLK_RIGHT:
							this->p1.walk(1);
							this->renderPlayer();
							this->displayWorld();
							a--;
							break;
						case SDLK_DOWN:
							this->p1.walk(2);
							this->renderPlayer();
							this->displayWorld();
							a--;
							break;
						case SDLK_LEFT:
							this->p1.walk(3);
							this->renderPlayer();
							this->displayWorld();
							a--;
							break;
						}

				}
				else continue;
			}
			if (a == 0) {
				break;
			}		
		}
			
		quit = SDL_TRUE;
			
		//deprecated implementation
		/*do {
			unsigned char ch1 = _getch();
			if (ch1 == KEY_ARROW_CHAR1) {
				this->updateWorld();
				switch (input = _getch()) {
				case KEY_UP:
					this->p1.walk(0);
					break;
				case KEY_RIGHT:
					this->p1.walk(1);
					break;
				case KEY_DOWN:
					this->p1.walk(2);
					break;
				case KEY_LEFT:
					this->p1.walk(3);
					break;
				}
				this->placePlayer();
			}
			this->displayWorld();
			a--;
		} while (a > 0);*/
	}

	//Erases player at position row, col. Restores the location with original tile. Happens when an arrow key has been pressed
	//deprecated
	void updateWorld() {
		this->m[this->p1.row][this->p1.col] = this->spot;
	}

	//Saves new overwritten tile then writes player onto it
	//deprecated
	void placePlayer() {
		//this->spot = m[this->p1.row][this->p1.col];
		//this->m[this->p1.row][this->p1.col] = 99;
	}
};

int main() {
	char input;
	world2D test;
	test.loadWorld();
	test.displayWorld();
	test.simLoop();
	return 0;
}