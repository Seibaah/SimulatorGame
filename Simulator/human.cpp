#include "human.h"


human::human()
{
}

human::~human()
{
}

void human::print() {
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
	this->col = x;
	this->viewX = col - 3;
}

void human::setY(int y)
{
	this->row = y;
	this->viewY = row - 3;
}

void human::move(int dir)
{
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
	this->viewX = col - 3;
	this->viewY = row - 3;
}

void human::eat()
{
	for (int i = 0; i < this->inventorySize; i++) {
		if (this->inventory[i] == 1 || this->inventory[i] == 2) {
			this->inventory[i] = 0;
			this->inventoryLoad--;
			this->hunger+=1;
			this->decisionCoef += 0.3;
			this->hungerCountdown = 14;
			break;
		}
	}
}

void human::dropItem()
{
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


