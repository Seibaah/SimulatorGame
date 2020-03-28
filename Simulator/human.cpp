#include "human.h"


human::human()
{
}


human::~human()
{
}


void human::print() {
	//used for debugging
	std::cout << "-------------------------------------" << std::endl;
	std::cout << this->row <<" "<< this->col << std::endl;
	std::cout << "hunger: " << this->hunger << " hearts: " << this->hearts << std::endl;
	std::cout << "dec coef: " << decisionCoef << std::endl;
	std::cout << "inventory" << std::endl;

	for (int i = 0; i < this->inventorySize; i++) {
		std::cout << inventory[i] << " " ;
	}
	std::cout << std::endl;

}


void human::setX(int x)
{
	//sets npc x and vision x coordinates
	this->col = x;
	this->viewX = col - 3;
}


void human::setY(int y)
{
	//sets npc y and vision y coordinates
	this->row = y;
	this->viewY = row - 3;
}


void human::updatePos(int dir)
{
	//moves npc coordinates, prevents out of bounds placement
	switch (dir) {
	case 0:
		if (row != 0)	this->row--; break;
	case 1:
		if (col != 99)	this->col++; break;
	case 2:
		if (row != 99)	this->row++; break;
	case 3:
		if (col != 0)	this->col--; break;
	}
}


void human::setView() {
	//updates vision coordinates
	this->viewX = col - 3;
	this->viewY = row - 3;
}


void human::eat()
{
	for (int i = 0; i < this->inventorySize; i++) {

		//eat a fish or apple is possible
		if (this->inventory[i] == 1 || this->inventory[i] == 2) {
			//updates inventory vars accordingly
			this->inventory[i] = 0;
			this->inventoryLoad--;

			//update npc stats
			this->hunger+=1;
			this->decisionCoef += 0.3;
			this->hungerCountdown = 14;

			break;
		}
	}
}


void human::dropItem()
{
	//NPC will free half of its inventory worth of material
	int quant = inventorySize / 2;
	for (int i = 0; i < this->inventorySize; i++) {
		if (this->inventory[i] == 3 || this->inventory[i] == 4) {
			this->inventory[i] = 0;
			this->inventoryLoad--;
			quant--;
			if (quant == 0) {
				break;
			}
		}
	}
}

//implementation incomplete - inactive
void human::escapeDanger(int(&map)[100][100][3]) {
	for (int i = this->viewY; i < this->viewY + this->viewRange * 2; i++) {
		for (int j = this->viewX; j < this->viewX + this->viewRange * 2; j++) {
			if ((i >= 0 && i <= 99) && (j >= 0 && j <= 99)) {
				if (map[i][j][2] == 2) {	//jaguar
					this->dangerDeltaY = this->row - i;
					this->dangerDeltaX = this->col - j;
				}
			}
		}
	}
}


void human::updateStatsNPC(int(&map)[100][100][3]) {
	
	//if NPC is alive
	if (this->isDead == false) {

		//if hunger is at 0 then loose a heart
		if (this->hunger == 0) {
			this->hearts--;
		}
		
		if (this->hungerCountdown == 0) {
			this->hunger--;
			this->decisionCoef -= 0.3;
			this->hungerCountdown = 14;
		}
		else this->hungerCountdown--;
	}
	//kills player if hearts reach 0
	if (this->hearts == 0 && this->isDead == false) {
		map[this->row][this->col][2] = 0;
		this->isDead = true;
	}
}


void human::findFood(int(&map)[100][100][3]) {

	//scans vision range of map to find food
	for (int i = this->viewY; i < this->viewY + this->viewRange * 2; i++) {
		for (int j = this->viewX; j < this->viewX + this->viewRange * 2; j++) {
			if ((i >= 0 && i <= 99) && (j >= 0 && j <= 99)) {
				if (map[i][j][1] == 1 || map[i][j][1] == 2) {
					this->foodTargetY = i;
					this->foodTargetX = j;
				}
			}
		}
	}
}


void human::findMat(int(&map)[100][100][3]) {

	//scans vision range of map to find materials
	for (int i = this->viewY; i < this->viewY + this->viewRange * 2; i++) {
		for (int j = this->viewX; j < this->viewX + this->viewRange * 2; j++) {
			if ((i >= 0 && i <= 99) && (j >= 0 && j <= 99)) {
				if ((map[i][j][1] == 3) || (map[i][j][1] == 4)) {
					this->matTargetY = i;
					this->matTargetX = j;
				}
			}
		}
	}
}


void human::moveNPCs(int(&map)[100][100][3], int targetX, int targetY) {

	//if no target is found NPC moves in a random direction
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_real_distribution<> disDir(0, 4);

	map[this->row][this->col][2] = 0;
	if (targetX == -1 || targetY == -1) {
		int dir = (disDir(gen));
		this->updatePos(dir);
	}

	else {
		if (this->row < targetY) {
			this->updatePos(2);
		}
		else if (this->row > targetY) {
			this->updatePos(0);
		}
		else if (this->col < targetX) {
			this->updatePos(1);
		}
		else if (this->col > targetX) {
			this->updatePos(3);
		}
	}

	//if target is reached pick up the item
	if ((this->row == targetY) && (this->col == targetX)) {

		//if inventory full, free space
		if (this->inventoryLoad == this->inventorySize) {
			this->dropItem();
		}

		//reset targets
		this->foodTargetX = -1;
		this->foodTargetY = -1;
		this->matTargetX = -1;
		this->matTargetY = -1;

		//pickup obj
		this->objectPickupNPC(map);
	}

	//place player on board
	map[this->row][this->col][2] = 1;

	//update vision
	this->setView();
}


void human::objectPickupNPC(int(&map)[100][100][3]) {

	//pickup obj if there is an item on the ground and if there is space
	if (this->inventoryLoad < this->inventorySize && map[this->row][this->col][1] != 0) {
		for (int i = 0; i < this->inventorySize; i++) {

			if (this->inventory[i] == 0) {

				this->inventory[i] = map[this->row][this->col][1];
				map[this->row][this->col][1] = 0;
				this->inventoryLoad++;

				break;
			}
		}
	}
}


