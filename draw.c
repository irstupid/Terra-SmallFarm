#include <ncurses.h>
#include <stdio.h>
#include <string.h>

void setupNcurses() {
	initscr();
        cbreak(); //change to raw later maybe
        noecho();
        keypad(stdscr, TRUE);
	timeout(0);
        curs_set(0);
}

void drawTile(int y, int x, Tile tile, void* rawData) {
	switch(tile) {
		case TILE_CRYSTAL:
			mvaddstr(y    , x, "',:,'");
			mvaddstr(y + 1, x, "--@--");
			mvaddstr(y + 2, x, ",':',");
		break;
		case TILE_DIRT:
			mvaddstr(y    , x, " : . ");
			mvaddstr(y + 1, x, ".  , ");
			mvaddstr(y + 2, x, " .  ;");
		break;
		case TILE_FARM:
			mvaddstr(y    , x, "~~~~~");
			mvaddstr(y + 1, x, "~~~~~");
			mvaddstr(y + 2, x, "~~~~~");
		break;
		case TILE_PATH:
			mvaddstr(y    , x, "[][ ]");
			mvaddstr(y + 1, x, "[ ][]");
			mvaddstr(y + 2, x, "][ ][");
		break;
		case TILE_CARROT:
			CarrotData carrotData = *((CarrotData*)rawData);
			switch(carrotData.growthStage) {
				case 0:
					mvaddstr(y    , x, "     ");
					mvaddstr(y + 1, x, "     ");
					mvaddstr(y + 2, x, " ;,, ");
				break;
				case 1:
					mvaddstr(y    , x, "     ");
					mvaddstr(y + 1, x, " ,;, ");
					mvaddstr(y + 2, x, " \\ / ");
				break;
				case 3: default:
					mvaddstr(y    , x, ".#$#;");
					mvaddstr(y + 1, x, "\\\"  /");
					mvaddstr(y + 2, x, " \\_/ ");
				break;
			}
		break;
		case TILE_STRAWBERRY:
			StrawberryData strawberryData = *((StrawberryData*)rawData);
			switch(strawberryData.growthStage) {
				case 0:
					mvaddstr(y    , x, "     ");
					mvaddstr(y + 1, x, "     ");
					mvaddstr(y + 2, x, " ..: ");
				break;
				case 1:
					mvaddstr(y    , x, "     ");
					mvaddstr(y + 1, x, " c\") ");
					mvaddstr(y + 2, x, "( c )");
				break;
				case 2:
					mvaddstr(y    , x, " ,C. ");
					mvaddstr(y + 1, x, "(   )");
					mvaddstr(y + 2, x, "\\   /");
				break;
				case 3: default:
					mvaddstr(y    , x, " ,C. ");
					mvaddstr(y + 1, x, "(% %)");
					mvaddstr(y + 2, x, "\\ % /");
				break;
			}
		break;
		case TILE_LETTUCE:
			LettuceData lettuceData = *((LettuceData*)rawData);
			switch(lettuceData.growthStage) {
				case 0:
					mvaddstr(y    , x, " ,-, ");
					mvaddstr(y + 1, x, "( 0 )");
					mvaddstr(y + 2, x, " '-' ");
				break;
				case 1:
					switch(lettuceData.quadrent) {
						case 0:
							mvaddstr(y    , x, "  ,~,");
							mvaddstr(y + 1, x, " /\\  ");
							mvaddstr(y + 2, x, "/\"\"-/");
						break;
						case 1:
							mvaddstr(y    , x, "/\"\\  ");
							mvaddstr(y + 1, x, "|) ) ");
							mvaddstr(y + 2, x, "\\_/~.");
						break;
						case 2:
							mvaddstr(y    , x, "\\_;-\\");
							mvaddstr(y + 1, x, " (/ |");
							mvaddstr(y + 2, x, " (_,~");
						break;
						case 3:
							mvaddstr(y    , x, "/~_~'");
							mvaddstr(y + 1, x, "\\/ *,");
							mvaddstr(y + 2, x, ",\\_- ");
						break;
					}
				break;
				case 2: default:
					switch(lettuceData.quadrent) {
						case 0:
							mvaddstr(y    , x, "   ~\"");
							mvaddstr(y + 1, x, "/\"( _");
							mvaddstr(y + 2, x, "\\_ /.");
						break;
						case 1:
							mvaddstr(y    , x, "\"\"\\  ");
							mvaddstr(y + 1, x, "_/X. ");
							mvaddstr(y + 2, x, ".\\  |");
						break;
						case 2:
							mvaddstr(y    , x, " ,X\\'");
							mvaddstr(y + 1, x, "(   \"");
							mvaddstr(y + 2, x, " \\___");
						break;
						case 3:
							mvaddstr(y    , x, "'/ .)");
							mvaddstr(y + 1, x, "\"L-+ ");
							mvaddstr(y + 2, x, "/-_/ ");
						break;
					}
				break;
			}
		break;
		case TILE_EMPTY: default:
			mvaddstr(y    , x, "     ");
			mvaddstr(y + 1, x, "     ");
			mvaddstr(y + 2, x, "     ");
		break;
	}
}

int anchorx = 0;
int anchory = 0;

void findAnchor(World* world) {
        anchorx = COLS/2 - (world->width * 5)/2;
        anchory = LINES/2 - (world->height * 3)/2;
        if(anchorx - 1 < 0 || anchory - 1 < 0) {
                fprintf(stderr, "screen is to small");
                exit(1);
        }
}

void drawWorld(World* world) {
	for(int x = 0; x < world->width; x++) {
	for(int y = 0; y < world->height; y++) {
		drawTile(anchory + y * 3, anchorx + x * 5, world->tiles[x + y * world->width], world->data[x + y * world->width]);
	}
	}
}

void drawPlayer(Cell* player, bool owo) {
	int x = player->x * 5 + anchorx;
	int y = player->y * 3 + anchory;

	mvaddstr(y    , x + 1, "\\V/");
	if(!owo) {
		mvaddstr(y + 1, x    , "(o-o)");
	} else {
		mvaddstr(y + 1, x    , "(owo)");
	}
	mvaddstr(y + 2, x + 1, "|");
	mvaddstr(y + 2, x + 3, "|");

}

void drawMenu(World* world, Menu* menu, Inventory* inventory) {
	if(menu->active) {
		if(!menu->shop) {
			for(int i = 0; i < INVENTORY_ITEM_COUNT; i++) {
				mvaddstr(anchory + i, anchorx, "                    ");
				mvaddstr(anchory + i, anchorx + 1, ITEM_NAME[INVENTORY_ITEMS[i]]);
				mvprintw(anchory + i, anchorx + 20, "%d", inventory->items[INVENTORY_ITEMS[i]]);
			}
			mvaddch(anchory + menu->select, anchorx, '>');
		} else {
			mvaddstr(anchory, anchorx, "     ## SHOP ##    ");
			for(int i = 0; i < SHOP_ITEM_COUNT; i++) {
				mvaddstr(anchory + 1 + i, anchorx, "                       ");
				mvaddstr(anchory + 1 + i, anchorx + 1, ITEM_NAME[SHOP_ITEMS[i]]);
				mvprintw(anchory + 1 + i, anchorx + 20, "%d", ITEM_PRICE[INVENTORY_ITEMS[i]]);
				mvprintw(anchory + 1 + i, anchorx + 23, "%d", inventory->items[INVENTORY_ITEMS[i]]);
			}
			mvaddch(anchory + 1 + menu->select, anchorx, '>');
		}
	} else {
		mvaddstr(anchory, anchorx, "                    ");
		if(!menu->shop) {
			mvaddstr(anchory, anchorx, ITEM_NAME[INVENTORY_ITEMS[menu->select]]);
		} else {
			mvaddstr(anchory, anchorx, ITEM_NAME[SHOP_ITEMS[menu->select]]);
		}
	}

	char moneyBuffer[8]; //the stylish segfault suicide should kill the program befor this buffer overflows
	sprintf(moneyBuffer, "%d", inventory->money);
	mvaddstr(anchory, anchorx + world->width * 5 - strlen(moneyBuffer), moneyBuffer);
}

void finishDraw() {
	refresh();
}

void cleanUpDraw() {
	endwin();
}
