# Paleolithic

•	2D pixel art, turn based survival game. Coded in C++ and SDL.
•	Implemented procedural terrain generation.
•	Simple inventory and crafting system with UI.
•	NPCs follow a decision tree to gather food and avoid starving.

The final project serves to demo some elements present in simulations, mainly procedural terrain generation, NPCs behavior and simulation loop. 

The terrain generation works with a method described by Prof. Vybihal as “Volcano method with n-sized footprints”. We pick random seed points in the world. The surrounding cells constitute a footprint and have a random chance to see a terrain increase of 1 unit. This method is applied iteratively, and the footprint increases up to a maximum defined value. Each seed has a different max size footprint. The game uses seeds for landmass and for waterbodies.

NPCs are spawned at random locations and have a hunger-meter. Their only directive is to gather food if they see any and consume it when hungry to avoid starving. If no food is in range, then they just wander around.

