#pragma once
#include <vector>
#include <algorithm>
#include <iostream>

class human
{
public:
	human();
	~human();
	int row, col;
	//enum direction { up = 0, right = 1, down = 2, left = 3 };		//orientation
	int inventory[5], inventorySize = std::size(inventory), inventoryLoad = 0;
	std::vector<int> selectedItems;
	float hearts = 3.0, hunger = 3.0;
	int hungerCountdown = 15;
	bool spear = false;
	int viewX, viewY, viewRange = 3;
	float decisionCoef = 1.0;
	int foodTargetX = -1, foodTargetY = -1, matTargetX = -1, matTargetY = -1, dangerDeltaX = NULL, dangerDeltaY = NULL;
	bool isDead = false;

	void print();
	void setX(int x);
	void setY(int y);
	void move(int dir);
	void setView();
	void eat();
	void dropItem();
};

