#include <stdio.h>
#include <unistd.h>

void save(World* world, Inventory* inventory, char* name) {
	FILE* file = fopen(name, "wb");
	if(file == NULL) {
		perror("failed to open file");
		exit(1);
	}
	uint16_t money = inventory->money;
	fwrite(&money, sizeof(uint16_t), 1, file);
	uint16_t items[ITEM_COUNT];
	for(int i = 0; i < ITEM_COUNT; i++) {
		items[i] = inventory->items[i];
	}
	fwrite(items, sizeof(uint16_t), ITEM_COUNT, file);
	uint16_t size[2] = {world->width, world->height};
	fwrite(size, sizeof(uint16_t), 2, file);
	uint16_t tiles[world->width * world->height];
	for(int i = 0; i < world->width * world->height; i++) {
		tiles[i] = world->tiles[i];
	}
	fwrite(tiles, sizeof(uint16_t), world->width * world->height, file);
	for(int i = 0; i < world->width * world->height; i++) {
		if(world->data[i] != NULL) {
			uint16_t index = i;
			fwrite(&index, sizeof(uint16_t), 1, file);
			int dataSize = 0;
			uint16_t data[8]; //arbitrary buffer size
			switch(world->tiles[i]) {
				case TILE_CARROT:
					dataSize = 2;
					data[0] = ((CarrotData*)world->data[i])->growthStage;
					data[1] = ((CarrotData*)world->data[i])->growthTime;
				break;
				case TILE_STRAWBERRY:
					dataSize = 3;
					data[0] = ((StrawberryData*)world->data[i])->growthStage;
					data[1] = ((StrawberryData*)world->data[i])->growthTime;
					data[2] = ((StrawberryData*)world->data[i])->timesHarvested;
				break;
				case TILE_LETTUCE:
					dataSize = 3;
					data[0] = ((LettuceData*)world->data[i])->growthStage;
					data[1] = ((LettuceData*)world->data[i])->growthTime;
					data[2] = ((LettuceData*)world->data[i])->quadrent;
				break;
			}
			fwrite(data, sizeof(uint16_t), dataSize, file);
		}
	}
	fclose(file);
}

World* load(Inventory* inventory, char* name) {
	if(!(access(name, F_OK) == 0)) {
		World* world = allocateWorld(18, 10);
		newWorld(world);
		*inventory = (Inventory){0, .money = 64};
		save(world, inventory, name);
		return world;
	}
	FILE* file = fopen(name, "rb");
	if(file == NULL) {
		perror("failed to open file");
		exit(1);
	}
	uint16_t moneyBuffer;
	fread(&moneyBuffer, sizeof(uint16_t), 1, file);
	inventory->money = moneyBuffer;
	uint16_t itemBuffer[ITEM_COUNT];
	fread(itemBuffer, sizeof(uint16_t), ITEM_COUNT, file);
	for(int i = 0; i < ITEM_COUNT; i++) {
		inventory->items[i] = itemBuffer[i];
	}
	
	uint16_t sizeBuffer[2];
	fread(sizeBuffer, sizeof(uint16_t), 2, file);
	World* world = allocateWorld(sizeBuffer[0], sizeBuffer[1]);
	uint16_t tileBuffer[world->width * world->height];
	fread(tileBuffer, sizeof(uint16_t), world->width * world->height, file);
	for(int i = 0; i < world->width * world->height; i++) {
		world->tiles[i] = tileBuffer[i];
	}
	uint16_t indexBuffer;
	uint16_t dataBuffer[8]; //arbitrary buffer size matching saveState
	while(fread(&indexBuffer, sizeof(uint16_t), 1, file) != 0) {
		switch(world->tiles[indexBuffer]) {
			case TILE_CARROT:
				world->data[indexBuffer] = smock(sizeof(CarrotData));
				fread(dataBuffer, sizeof(uint16_t), 2, file);
				*((CarrotData*)world->data[indexBuffer]) = (CarrotData){dataBuffer[0], dataBuffer[1]};
			break;
			case TILE_STRAWBERRY:
				world->data[indexBuffer] = smock(sizeof(StrawberryData));
				fread(dataBuffer, sizeof(uint16_t), 3, file);
				*((StrawberryData*)world->data[indexBuffer]) = (StrawberryData){dataBuffer[0], dataBuffer[1], dataBuffer[2]};
			break;
			case TILE_LETTUCE:
				world->data[indexBuffer] = smock(sizeof(LettuceData));
				fread(dataBuffer, sizeof(uint16_t), 3, file);
				*((LettuceData*)world->data[indexBuffer]) = (LettuceData){dataBuffer[0], dataBuffer[1], dataBuffer[2]};
			break;
		}
	}
	fclose(file);
	return world;
}
