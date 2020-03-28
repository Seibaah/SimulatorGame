#pragma once
#include <vector>
#include <algorithm>
#include <iostream>
#include <random>

class human
{
public:
	//npc coordinates, default spawn at 0,0
	int row=0, col=0;

	//inventory erlated vars
	int inventory[5], inventorySize = std::size(inventory), inventoryLoad = 0;

	//for crafting - not yet implemented
	//std::vector<int> selectedItems;

	//npc stats
	int hearts = 3, hunger = 3, hungerCountdown = 15;

	//npc vision related variables
	int viewX, viewY, viewRange = 3;

	//var to control npc directive/decision flow
	float decisionCoef = 1.0;

	//coordinates of target
	int foodTargetX = -1, foodTargetY = -1, matTargetX = -1, matTargetY = -1, dangerDeltaX = NULL, dangerDeltaY = NULL;
	
	bool isDead = false;

	human();
	~human();

	void print();
	
	void setX(int x);
	
	void setY(int y);
	
	void updatePos(int dir);
	
	void setView();
	
	void eat();
	
	void dropItem();
	
	void escapeDanger(int(&map)[100][100][3]);
	
	void updateStatsNPC(int(&map)[100][100][3]);
	
	void findFood(int(&map)[100][100][3]);
	
	void findMat(int(&map)[100][100][3]);
	
	void moveNPCs(int(&map)[100][100][3], int targetX, int targetY);
	
	void objectPickupNPC(int(&map)[100][100][3]);
};

