#include <iostream>
#include "world.h"

#undef main

using namespace std;

int main() {
	int n = 5000;	//turns the simulation will run
	world naboo;
	naboo.run(n);
	return 0;
}