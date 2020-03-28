There are 2 ways to run the project:

1)Simple way: Use the executable in the release folder, no extra configuration needed for runtime.

2)Harder way: Run from Visual Studio 2017 + SDL 2.0
	a) Follow these instructions to install and set up SDL 2.0 with VS 2017.
		https://www.youtube.com/watch?v=QQzAHcojEKg
		VERY IMPORTANT: the user in the video uses x64 libraries, use the x86 as this is a win32 app.

------------------------------------------------------------------------------------------------------------------------
Tutorial:
	-Use arrow keys to move around
	-Collect items by walking on them, pickup is automatic, you have a 5 item inventory capacity.
	-To perform inventory management press 's' and use the numbers 1-5 to select or deselect items.
		-Once selected you can press 'c' to try and craft something with them. Currently there is only 1 recipe: a spear for 2 wood and 1 rock.
		 Other item selections are ignored, orders doesn't matter. Select mode is exited.
		-By pressing 'd' you can drop the selected items and free up inventory space. Thrown items can't be recovered. Select mode is exited.
		-By pressing 's' select mode is exited.
	-By pressing 'a' you can attack. If you have a spear crafted an animation will play out. The implementation is currently only aesthetic as it
	 stands incomplete.
	-By pressing 'e' you will consume an apple or fish from your inventory and replenish your hunger bar. If none availabe nothing happens. No 	 difference between eating apple or fish. You can only eat when hungry.
	-By pressing 'm' you can see the whole world map and your position on it denoted by a fuchsia square. Press 'm' again to go back to normal camera.
	 No other action is allowed while in world map view..

Stats:
	-Every 15 turns you loose 1 hunger from the hunger bar. Eating resets the hunger timer. If your bar runs out you loose 1 heart per turn.
	 A full hunger bar (i.e. 3 hunger) will regenerate your health bar if the latter is not full (i.e. 3 hearts).
	-You die when your hearts reach 0.

World generation:
	-Done using n-sized footprint algorithm to generate land or oceans. Chunks (10x10 zones of the map) will either host a randomly placed land or
	 ocean seed which will affect it's cell's value accordingly. Land to ocean proportion is a random coef between .4 and .66.
	-Player and NPCs are spawned in a random location that is either land or shallow water.
	-Items are spawned in random location in the world, but only os specific types of cells (e.g fish can only spawn on shallow water, wood only in
	 deep forest, etc...).
	-New items are spawned every 20 turns.

NPCs:
	-Random number between 8 and 16 per run.
	-Have a simple decision process. They have a vision range which they scan for food and materials. If they are hungry or not, judged by a decision
	 coef, they will go for food or materials. If none if found within vision the NPC moves in a random direction.
	-They can also die from hunger.
	-No memory-like ability.
	-Incomplete and inactive fight or flee implementation.

Extra notes:
	-I did the sprites. :D

Known bugs:
	-Rare render error will display black or white tiles on debug lauch. No fix besides re-launching the simulation. 
	-Rare misc memory errors.
	-NPC "stuck" on an object.
	-NPC goes for last valid target.
	-Trying to move out of bounds counts as an action, thus you can die from trying to break out of the world.
	-Player and NPCs movement not restricted to viable terrain (i.e. can go swimming in deep ocean)
	-Probably a couple of other things I am forgetting at the moment.

Ideas to implement:
	-Graph based path searching for NPCs.
	-Better terrain transition smoothing (no concrete idea yet).
	-Finish combat implementation.
	-Add naturally hostile NPCs like jaguars.
	-Add savestates and possibility to load and save to a file.
	-Add data tarcking for simulation objects.
	-Develop crafting system even further.
















	