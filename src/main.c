#include <curses.h>
#include <menu.h>
#include <string.h>
#include <time.h>

#include "exit.h"
#include "menu.h"
#include "input.h"

#define ARRAY_SIZE(a) (sizeof(a)/sizeof(a[0]))
#define KEY_CTRL(x)  ((x)&0x1F)

#define INPUTBUFFER_LENGTH 512
char* inputBuffer;

static ITEM* Item_createEntry(char* title,char* description){
	ITEM* item = new_item(title,description);
	if(!item)
		return NULL;
	
	//Allocate and initate item data
	struct ItemData* data = malloc(sizeof(struct ItemData));
	if(!data)
		return NULL;

	set_item_userptr(item,data);

	data->type = ITEMDATATYPE_ENTRY;
	data->entry = (struct EntryData){
		.title       = title,
		.description = description,
		.priority    = 0,
		.datetime    = time(NULL),
		.deadline    = 0,
	};

	return item;
}

static ITEM* Item_createButton(char* title,int(*perform)(struct MenuData* menuData)){
	ITEM* item = new_item(title,NULL);

	//Allocate and initate item data
	struct ItemData* data = malloc(sizeof(struct ItemData));
	if(!data)
		return NULL;

	set_item_userptr(item,data);

	data->type = ITEMDATATYPE_BUTTON;
	data->button.perform = perform;

	return item;
}

static void Item_free(ITEM* item){
	struct ItemData* data = item_userptr(item);

	//Free item data
	switch(data->type){
		case ITEMDATATYPE_ENTRY:
			free(data->entry.title);
			free(data->entry.description);
			break;
		
		default:
			break;
	}

	free(data);

	//Free item
	free_item(item);
}

static int button_performAdd(struct MenuData* menuData){
	if(menuData->itemsLen<menuData->itemsSize){
		//Clear last lines
		move(LINES-1,0);
		clrtoeol();

		//Get title from input
		mvprintw(LINES-1,0,"Input: ");
		size_t inputLength = getnstr_custom(inputBuffer,INPUTBUFFER_LENGTH);

		//Clear last lines
		move(LINES-1,0);
		clrtoeol();

		//Silently ignore empty input
		if(inputLength==0)
			return EXIT_SUCCESS;

		inputBuffer[inputLength++] = '\0';
		char* title = malloc(inputLength);
		if(!title)
			return EXIT_ALLOCATION_FAILURE;
		memcpy(title,inputBuffer,inputLength);

		unpost_menu(menuData->menu);
			//Initiate new item
			if(!(menuData->items[menuData->itemsLen] = Item_createEntry(title,NULL)))
				return EXIT_ITEMENTRYCREATION_FAILURE;

			//Increment length
			++menuData->itemsLen;

			//Update the menu's items pointer
			set_menu_items(menuData->menu,menuData->items);
		post_menu(menuData->menu);
	}

	return EXIT_SUCCESS;
}

int main(){
	int exitCode = EXIT_SUCCESS;
	struct MenuData menuData;

	//Initiate buffers
	if(!(inputBuffer = calloc(INPUTBUFFER_LENGTH,sizeof(char)))){
		exitCode = EXIT_ALLOCATION_FAILURE;
		goto End;
	}

	//Initiate screen
	SCREEN* screen = newterm(getenv("TERM"),stdout,stdin);
	cbreak();
	noecho();
	keypad(stdscr,TRUE);//Enable builtin menu control with keys

	//Initiate initial items
	menuData.itemsLen  = 1;
	menuData.itemsSize = 10;
	menuData.items     = calloc(menuData.itemsSize+1,sizeof(ITEM*));
	if(!(menuData.items[0] = Item_createButton("[Add...]",&button_performAdd))){
		exitCode = EXIT_ALLOCATION_FAILURE;
		goto End;
	}

	//Initiate menu
	menuData.menu = new_menu(menuData.items);

	menu_opts_on (menuData.menu,O_ONEVALUE);//Enable single selection
	menu_opts_off(menuData.menu,O_SHOWDESC);//Disable showing description

	//Set selection marker
	set_menu_mark(menuData.menu,"> ");

	//Display the menu
	post_menu(menuData.menu);
	refresh();

	//For each key event
	int c;while((c=getch())){
		switch(c){
			//Move the selection down
			case KEY_DOWN:
				menu_driver(menuData.menu,REQ_DOWN_ITEM);
				break;

			//Move the selection up
			case KEY_UP:
				menu_driver(menuData.menu,REQ_UP_ITEM);
				break;

			//Move the selection to the beginning
			case 262://Home
				menu_driver(menuData.menu,REQ_FIRST_ITEM);
				break;

			//Move the selection to the end
			case 360://End
				menu_driver(menuData.menu,REQ_LAST_ITEM);
				break;

			//Choose current item
			case 10:{//Enter
				//Get current item
				ITEM* currentItem = current_item(menuData.menu);
				struct ItemData* itemData = item_userptr(currentItem);

				//If button is selected, perform the button action
				switch(itemData->type){
					case ITEMDATATYPE_BUTTON:{
						int returnCode = button_performAdd(&menuData);
						if(returnCode!=EXIT_SUCCESS){
							exitCode = returnCode;
							goto End;
						}
					}	break;

					case ITEMDATATYPE_ENTRY:
						//Clear last lines
						move(LINES-1,0);
						clrtoeol();

						//Write selected item info to last lines
						mvprintw(LINES-1,0,"You have chosen item %i with name %s and description %s",
							item_index(currentItem),
							item_name(currentItem),
							item_description(currentItem)
						);
						break;

					default:
						exitCode = EXIT_UNKNOWN_ITEMDATATYPE;
						goto End;
				}

				refresh();
				pos_menu_cursor(menuData.menu);

			}	break;

			//Remove current item
			case 330://Delete
			case 263:{//Backspace
				if(menuData.itemsLen>0){
					//Get current item
					ITEM* item = current_item(menuData.menu);
					struct ItemData* data = item_userptr(item);

					//Deny removal of non-entry items
					if(data->type != ITEMDATATYPE_ENTRY)
						break;

					int index = item_index(item);

					Item_free(item);

					unpost_menu(menuData.menu);
						//Move items in front
						for(int i=index;i<menuData.itemsLen;++i)
							menuData.items[i]=menuData.items[i+1];

						//Decrement length
						--menuData.itemsLen;

						//Update the menu's items pointer
						set_menu_items(menuData.menu,menuData.items);
					post_menu(menuData.menu);
				}
			}	break;

			case ' ':
				menu_driver(menuData.menu,REQ_TOGGLE_ITEM);
				break;

			//Quit application
			case 'q':
			case EOF:
			case KEY_CTRL('d'):
				goto End;
		}
	}

	End:
	//Free allocated memory
	unpost_menu(menuData.menu);
	for(ITEM** item=menuData.items;*item!=NULL;++item)
		free_item(*item);
	free(menuData.items);
	free_menu(menuData.menu);
	endwin();
	delscreen(screen);

	free(inputBuffer);

	return exitCode;
}
