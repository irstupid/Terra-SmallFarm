#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <time.h>

#include <stpdmem.h>

#define imax(a, b) (a) > (b) ? (a) : (b)
#define imin(a, b) (a) < (b) ? (a) : (b)

typedef struct {
	int x;
	int y;
} Cell;

typedef enum {
	TILE_EMPTY,
	TILE_CRYSTAL,
	TILE_FARM,
	TILE_DIRT,
	TILE_PATH,
	TILE_CARROT,
	TILE_STRAWBERRY,
	TILE_LETTUCE
} Tile;

typedef struct {
	Tile* tiles;
	void** data;
	int width;
	int height;
} World;

typedef struct {
	int growthStage;
	int growthTime; //time spent in this stage in frames
} CarrotData;

typedef struct {
	int growthStage;
	int growthTime; //time spent in this stage in frames
	int timesHarvested;
} StrawberryData;

typedef struct {
	int growthStage;
	int growthTime; //time spent in this stage in frames
	int quadrent;
	//01
	//23
} LettuceData;

const int CARROT_GROWTH_TIME = 10 * 60;
const int STRAWBERRY_GROWTH_TIME = 30 * 60;
const int LETTUCE_GROWTH_TIME = 2 * 60 * 60;

typedef enum {
	ITEM_CARROT,
	ITEM_STRAWBERRY,
	ITEM_LETTUCE,
	ITEM_CARROT_SEEDS,
	ITEM_STRAWBERRY_SEEDS,
	ITEM_LETTUCE_SEEDS,
	ITEM_DIRT,
	ITEM_FARM,
	ITEM_PATH,
	ITEM_COUNT
} Item;
const char* const ITEM_NAME[] = {
	"carrot",
	"strawberry",
	"lettuce",
	"carrot seeds",
	"strawberry seeds",
	"lettuce seeds",
	"dirt",
	"farm",
	"path"
};
const int ITEM_PRICE[] = {
	4,  //carrot
	6,  //strawberry
	20, //lettuce
	1,  //carrot seeds
	6,  //strawberry seeds
	4,  //lettuce seed
	1,  //dirt
	2,  //farm
	4   //path
};
const int INVENTORY_ITEM_COUNT = 9;
const int SHOP_ITEM_COUNT = 9;
const int INVENTORY_ITEMS[] = {
	ITEM_CARROT_SEEDS,
	ITEM_CARROT,
	ITEM_STRAWBERRY_SEEDS,
	ITEM_STRAWBERRY,
	ITEM_LETTUCE_SEEDS,
	ITEM_LETTUCE,
	ITEM_DIRT,
	ITEM_FARM,
	ITEM_PATH
};
const int SHOP_ITEMS[] = {
	ITEM_CARROT_SEEDS,
	ITEM_CARROT,
	ITEM_STRAWBERRY_SEEDS,
	ITEM_STRAWBERRY,
	ITEM_LETTUCE_SEEDS,
	ITEM_LETTUCE,
	ITEM_DIRT,
	ITEM_FARM,
	ITEM_PATH
};

typedef struct {
	const int* itemSet;
	int select;
	int selectMax;
	bool active;
	bool shop;
} Menu;

typedef struct {
	int money;
	int items[ITEM_COUNT];
} Inventory;

void newWorld(World* world) {
	int w = world->width;
	int h = world->height;
	for(int i = 0; i < w * h; i++) {
		world->tiles[i] = TILE_EMPTY;
		world->data[i] = NULL;
	}
	world->tiles[w/2 + ((h/2) * w)] = TILE_CRYSTAL;
}

World* allocateWorld(int width, int height) {
	World* world = smock(sizeof(World) +
	                     sizeof(Tile) * width * height +
	                     sizeof(void*) * width * height);
	world->width = width;
	world->height = height;
	world->tiles = (Tile*)(world + 1);
	world->data = (void**)(world->tiles + width * height);
	return world;
}

//void saveState(World* world, Inventory* inventory, char* name) {
//	FILE* file = fopen(name, "wb");
//	uint16_t items[ITEM_COUNT];
//	for(int i = 0; i < ITEM_COUNT; i++) {
//		items[i] = inventory->items[i];
//	}
//	fwrite(items, sizeof(uint16_t), ITEM_COUNT, file);
//	uint16_t money = inventory->money;
//	fwrite(&money, sizeof(uint16_t), 1, file);
//	uint16_t size[2] = {world->width, world->height};
//	fwrite(size, sizeof(uint16_t), 2, file);
//	uint16_t tiles[world->width * world->height];
//	for(int i = 0; i < world->width * world->height; i++) {
//		tiles[i] = world->tiles[i];
//	}
//	fwrite(tiles, sizeof(uint16_t), world->width * world->height, file);
//	for(int i = 0; i < world->width * world->height; i++) {
//		if(world->data[i] != NULL) {
//			uint16_t index = i;
//			fwrite(&index, sizeof(uint16_t), 1, file);
//			int dataSize = 0;
//			uint16_t data[8]; //arbitrary buffer size
//			switch(world->tile[i]) {
//				case TILE_CARROT:
//					dataSize = 2;
//					data[0] = *((CarrotData*)world->data[i])->growthStage;
//					data[1] = *((CarrotData*)world->data[i])->growthTime;
//				break;
//				case TILE_STRAWBERRY:
//					dataSize = 3;
//					data[0] = *((CarrotData*)world->data[i])->growthStage;
//					data[1] = *((CarrotData*)world->data[i])->growthTime;
//					data[3] = *((CarrotData*)world->data[i])->timesHarvested;
//				break;
//				case TILE_LETTUCE:
//					dataSize = 3;
//					data[0] = *((CarrotData*)world->data[i])->growthStage;
//					data[1] = *((CarrotData*)world->data[i])->growthTime;
//					data[2] = *((CarrotData*)world->data[i])->quadrent;
//				break;
//			}
//			fwrite(data, sizeof(uint16_t), dataSize, file);
//		}
//	}
//	fclose(file);
//}

#include "draw.c"

bool placeCarrot(World* world, int i) {
	Tile* tile = &world->tiles[i];
	if(*tile != TILE_FARM) {
		return false;
	}
	*tile = TILE_CARROT;
	world->data[i] = smock(sizeof(CarrotData));
	*((CarrotData*)world->data[i]) = (CarrotData){};
	return true;
}

bool placeStrawberry(World* world, int i) {
	Tile* tile = &world->tiles[i];
	if(*tile != TILE_FARM) {
		return false;
	}
	*tile = TILE_STRAWBERRY;
	world->data[i] = smock(sizeof(StrawberryData));
	*((StrawberryData*)world->data[i]) = (StrawberryData){};
	return true;
}

bool placeLettuce(World* world, int i, int growthStage, int quadrent) {
	Tile* tile = &world->tiles[i];
	if(*tile != TILE_FARM) {
		return false;
	}
	*tile = TILE_LETTUCE;
	world->data[i] = smock(sizeof(LettuceData));
	*((LettuceData*)world->data[i]) = (LettuceData){.growthStage = growthStage, .quadrent = quadrent};
	return true;
}

bool processInput(int c, World* world, Cell* player, Inventory* inventory, Menu* menu) {
	if (c == ERR) {
		return false;
	} else if (c == 'x' || c == 'X'){
		return true;
	}
	
	if(!menu->active) {
		switch(c) {
			case KEY_UP: case 'w': case 'W':
				player->y--;
				player->y = imax(player->y, 0);
			break;
			case KEY_DOWN: case 's': case 'S':
				player->y++;
				player->y = imin(player->y, world->height - 1);
			break;
			case KEY_LEFT: case 'a': case 'A':
				player->x--;
				player->x = imax(player->x, 0);
			break;
			case KEY_RIGHT: case 'd': case 'D':
				player->x++;
				player->x = imin(player->x, world->width- 1);
			break;
			case 'q': case 'Q':
				menu->active = true;
				if(world->tiles[player->x + player->y * world->width] == TILE_CRYSTAL) {
					menu->shop = true;
				} else {
					menu->shop = false;
				}
			break;	
			case 'e': case 'E':
				Item select = menu->itemSet[menu->select];
				Tile* tile = &(world->tiles[player->x + player->y * world->width]);
				void* data = world->data[player->x + player->y * world->width];

				if(*tile == TILE_CRYSTAL) {
					break;
				}
				if(*tile == TILE_CARROT) {
					if(((CarrotData*)data)->growthStage == 2) {
						*tile = TILE_FARM;
						inventory->items[ITEM_CARROT] += 1;
					}
					break;
				}
				if(*tile == TILE_STRAWBERRY) {
					if(((StrawberryData*)data)->growthStage == 3) {
						((StrawberryData*)data)->growthStage = 2;
						((StrawberryData*)data)->timesHarvested++;
						if(((StrawberryData*)data)->timesHarvested >= 3) {
							*tile = TILE_FARM;
						}
						inventory->items[ITEM_STRAWBERRY] += 1;
					}
					break;
				}
				if(*tile == TILE_LETTUCE) {
					if(((LettuceData*)data)->growthStage == 2) {
						switch(((LettuceData*)data)->quadrent) {
							case 0:
								world->tiles[player->x     + (player->y    ) * world->width] = TILE_FARM;
								world->tiles[player->x + 1 + (player->y    ) * world->width] = TILE_FARM;
								world->tiles[player->x     + (player->y + 1) * world->width] = TILE_FARM;
								world->tiles[player->x + 1 + (player->y + 1) * world->width] = TILE_FARM;
							break;
							case 1:
								world->tiles[player->x     + (player->y    ) * world->width] = TILE_FARM;
								world->tiles[player->x - 1 + (player->y    ) * world->width] = TILE_FARM;
								world->tiles[player->x     + (player->y + 1) * world->width] = TILE_FARM;
								world->tiles[player->x - 1 + (player->y + 1) * world->width] = TILE_FARM;
							break;
							case 2:
								world->tiles[player->x     + (player->y    ) * world->width] = TILE_FARM;
								world->tiles[player->x + 1 + (player->y    ) * world->width] = TILE_FARM;
								world->tiles[player->x     + (player->y - 1) * world->width] = TILE_FARM;
								world->tiles[player->x + 1 + (player->y - 1) * world->width] = TILE_FARM;
							break;
							case 3:
								world->tiles[player->x     + (player->y    ) * world->width] = TILE_FARM;
								world->tiles[player->x - 1 + (player->y    ) * world->width] = TILE_FARM;
								world->tiles[player->x     + (player->y - 1) * world->width] = TILE_FARM;
								world->tiles[player->x - 1 + (player->y - 1) * world->width] = TILE_FARM;
							break;
						}
						inventory->items[ITEM_LETTUCE] += 1;
					}
					break;
				}

				if(select == ITEM_STRAWBERRY || select == ITEM_CARROT || select == ITEM_LETTUCE) {
					*tile = TILE_EMPTY;
					break;
				}
				if(inventory->items[select] > 0) {
					if(select == ITEM_DIRT) { world->tiles[player->x + player->y * world->width] = TILE_DIRT; }
					if(select == ITEM_FARM) { world->tiles[player->x + player->y * world->width] = TILE_FARM; }
					if(select == ITEM_PATH) { world->tiles[player->x + player->y * world->width] = TILE_PATH; }
					if(select == ITEM_CARROT_SEEDS) {
						if(!placeCarrot(world, player->x + player->y * world->width)) {
							break;
						}
					}
					if(select == ITEM_STRAWBERRY_SEEDS) {
						if(!placeStrawberry(world, player->x + player->y * world->width)) {
							break;
						}
					}
					if(select == ITEM_LETTUCE_SEEDS) {
						if(!placeLettuce(world, player->x + player->y * world->width, 0, 0)) {
							break;
						}
					}
					inventory->items[select]--;
				}
			break;
		}
	} else {
		switch(c) {
			case KEY_UP: case 'w': case 'W':
				menu->select--;
				menu->select = (menu->select < 0 ? menu->selectMax : menu->select);
			break;
			case KEY_DOWN: case 's': case 'S':
				menu->select++;
				menu->select = (menu->select > menu->selectMax ? 0 : menu->select);
			break;
			case 'q': case 'Q':
				menu->active = false;
			break;
			case 'e': case 'E':
				if(menu->shop) {
					if(SHOP_ITEMS[menu->select] == ITEM_CARROT || SHOP_ITEMS[menu->select] == ITEM_STRAWBERRY|| SHOP_ITEMS[menu->select] == ITEM_LETTUCE) {
						if(inventory->items[SHOP_ITEMS[menu->select]] > 0) {
							inventory->money += ITEM_PRICE[SHOP_ITEMS[menu->select]];
							inventory->items[SHOP_ITEMS[menu->select]]--;
						}
					} else if(inventory->money >= ITEM_PRICE[SHOP_ITEMS[menu->select]]) {
						inventory->money -= ITEM_PRICE[SHOP_ITEMS[menu->select]];
						inventory->items[SHOP_ITEMS[menu->select]]++;
					}
				}
			break;
		}
	}

	return false;
}

void cleanup(World* world) {
	free(world);
	cleanUpDraw();
}

void update(World* world, Cell* player, Inventory* inventory, Menu* menu) {
	if(menu->shop) {
		menu->selectMax = SHOP_ITEM_COUNT - 1;
		menu->itemSet = SHOP_ITEMS;
	} else {
		menu->selectMax = INVENTORY_ITEM_COUNT - 1;
		menu->itemSet = INVENTORY_ITEMS;
	}

	for(int i = 0; i < world->width * world->height; i++) {
		switch(world->tiles[i]) {
			case TILE_CARROT:
				CarrotData* carrotData = (CarrotData*)world->data[i];
				if(carrotData->growthStage >= 2) {
					break;
				}
				carrotData->growthTime++;
				if(carrotData->growthTime > CARROT_GROWTH_TIME) {
					carrotData->growthStage++;
					carrotData->growthTime = 0;
				}
			break;
			case TILE_STRAWBERRY:
				StrawberryData* strawberryData = (StrawberryData*)world->data[i];
				if(strawberryData->growthStage >= 3) {
					break;
				}
				strawberryData->growthTime++;
				if(strawberryData->growthTime > STRAWBERRY_GROWTH_TIME) {
					strawberryData->growthStage++;
					strawberryData->growthTime = 0;
				}
			break;
			case TILE_LETTUCE:
				LettuceData* lettuceData = (LettuceData*)world->data[i];
				if(lettuceData->growthStage >= 2) {
					break;
				}
				lettuceData->growthTime++;
				if(lettuceData->growthTime > LETTUCE_GROWTH_TIME) {
					if(lettuceData->growthStage == 0) {
						if       (world->tiles[i - 1               ] == TILE_FARM &&
						          world->tiles[i     - world->width] == TILE_FARM &&
						          world->tiles[i - 1 - world->width] == TILE_FARM) {
							placeLettuce(world, i - 1               , 1, 2);
							placeLettuce(world, i     - world->width, 1, 1);
							placeLettuce(world, i - 1 - world->width, 1, 0);
							lettuceData->quadrent = 3;
							lettuceData->growthStage = 1;
							lettuceData->growthTime = 0;
						} else if(world->tiles[i + 1               ] == TILE_FARM &&
						          world->tiles[i     - world->width] == TILE_FARM &&
						          world->tiles[i + 1 - world->width] == TILE_FARM) {
							placeLettuce(world, i + 1               , 1, 3);
							placeLettuce(world, i     - world->width, 1, 0);
							placeLettuce(world, i + 1 - world->width, 1, 1);
							lettuceData->quadrent = 2;
							lettuceData->growthStage = 1;
							lettuceData->growthTime = 0;
						} else if(world->tiles[i - 1               ] == TILE_FARM &&
						          world->tiles[i     + world->width] == TILE_FARM &&
						          world->tiles[i - 1 + world->width] == TILE_FARM) {
							placeLettuce(world, i - 1               , 1, 0);
							placeLettuce(world, i     + world->width, 1, 3);
							placeLettuce(world, i - 1 + world->width, 1, 2);
							lettuceData->quadrent = 1;
							lettuceData->growthStage = 1;
							lettuceData->growthTime = 0;
						} else if(world->tiles[i + 1               ] == TILE_FARM &&
						          world->tiles[i     + world->width] == TILE_FARM &&
						          world->tiles[i + 1 + world->width] == TILE_FARM) {
							placeLettuce(world, i + 1               , 1, 1);
							placeLettuce(world, i     + world->width, 1, 2);
							placeLettuce(world, i + 1 + world->width, 1, 3);
							lettuceData->quadrent = 0;
							lettuceData->growthStage = 1;
							lettuceData->growthTime = 0;
						} else {
							lettuceData->growthTime--;
						}
					} else {
						lettuceData->growthStage++;
					}
				}
			break;
			default:
				if(world->data[i] != NULL) {
					free(world->data[i]);
					world->data[i] = NULL;
				}
			break;
		}
	}

	if(inventory->money > UINT16_MAX) {
		cleanup(world);
		int* nullPointer = NULL;
		*nullPointer = 0; //stylish segfault suicide
	}
}

int main(int argc, char** argv) {
//	if(argc != 2) {
//		printf("invalid arguments!\n");
//		return 1;
//	}

	setupNcurses();

	struct timespec frametime = {0, 16666667};

	World* world = allocateWorld(18, 10);
	newWorld(world);

	Cell player = {0, 0};
	Inventory inventory = {.money = 100};

	Menu menu = {.active = false};

	findAnchor(world);

	while(1) {
		if(processInput(getch(), world, &player, &inventory, &menu)) {
			break;
		}
		update(world, &player, &inventory, &menu);

		drawWorld(world);
		drawPlayer(&player, inventory.items[ITEM_STRAWBERRY] > 128);
		drawMenu(world, &menu, &inventory);
		finishDraw();

		nanosleep(&frametime, NULL);
	}
	
	//saveState(world, &inventory, argv[1]);
	cleanup(world);
	return 0;
}
