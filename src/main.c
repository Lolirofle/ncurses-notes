#include <curses.h>
#include <menu.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>

#define ARRAY_SIZE(a) (sizeof(a)/sizeof(a[0]))
#define KEY_CTRL(x)  ((x)&0x1F)

#define EXIT_ALLOCATION_FAILURE 54
#define EXIT_INPUT_ERROR 13

#define INPUTBUFFER_LENGTH 512

struct EntryData{
	unsigned short priority;
	time_t datetime;
	time_t deadline;
};

enum ItemDataType{
	ITEMDATATYPE_BUTTON,
	ITEMDATATYPE_ENTRY,
};

struct ItemData{
	enum ItemDataType type;
	union{
		struct EntryData entry;
	};
};

size_t getnstr_custom(char* buffer,size_t maxLength){
	size_t length = 0;
	int c;
	while((c=getch())){
		switch(c){
			case KEY_ENTER:
			case '\n':
			case '\r':
				*buffer = '\0';
				return length;
			
			default:
				if(length<maxLength && isprint(c)){
					*buffer++ = c;
					++length;
				}
				break;
		}
	}
}

int main(){
	//Initiate buffers
	char* inputBuffer = calloc(INPUTBUFFER_LENGTH,sizeof(char));
	if(!inputBuffer)
		return EXIT_ALLOCATION_FAILURE;

	//Initiate screen
	initscr();
		cbreak();
		noecho();

	//Enable builtin menu control with keys
	keypad(stdscr,TRUE);

	//Initiate items
	unsigned int itemsLen = 1,
	             itemsSize = 10;
	ITEM** items = calloc(itemsSize+1,sizeof(ITEM*));
	items[0] = new_item("[Add...]",NULL);

	//Initiate menu
	MENU* menu = new_menu(items);

	menu_opts_on(menu,O_ONEVALUE);//Enable single selection
	menu_opts_off(menu,O_SHOWDESC);//Disable showing description

	//Set selection marker
	set_menu_mark(menu,"> ");

	//Display the menu
	post_menu(menu);
	refresh();

	//For each key event
	int c;while((c=getch())){
		switch(c){
			//Move the selection down
			case KEY_DOWN:
				menu_driver(menu,REQ_DOWN_ITEM);
				break;

			//Move the selection up
			case KEY_UP:
				menu_driver(menu,REQ_UP_ITEM);
				break;

			//Move the selection to the beginning
			case 262://Home
				menu_driver(menu,REQ_FIRST_ITEM);
				break;

			//Move the selection to the end
			case 360://End
				menu_driver(menu,REQ_LAST_ITEM);
				break;

			//Choose current item
			case 10:{//Enter
				//Get current item
				ITEM* currentItem = current_item(menu);

				//Clear last lines
				move(LINES-1,0);
				clrtoeol();

				//If first item is selected (Add...)
				if(item_index(currentItem)==0 && itemsLen<itemsSize){
					unpost_menu(menu);
						//Get name from input
						mvprintw(LINES-1,0,"Input:");
						size_t inputLength = getnstr_custom(inputBuffer,INPUTBUFFER_LENGTH);
						if(inputLength==0)
							return EXIT_INPUT_ERROR;

						char* name = malloc(inputLength);
						memcpy(name,inputBuffer,inputLength);

						//Clear last lines
						move(LINES-1,0);
						clrtoeol();

						//Initiate new item
						items[itemsLen] = new_item(name,NULL);

						//Allocate and initate item data
						struct EntryData* data = malloc(sizeof(struct EntryData));
						if(!data)
							return EXIT_ALLOCATION_FAILURE;
						set_item_userptr(items[itemsLen],data);

						//Increment length
						++itemsLen;

						//Update the menu's items pointer
						set_menu_items(menu,items);
					post_menu(menu);
				}else{
					//Write selected item info to last lines
					mvprintw(LINES-1,0,"You have chosen item %i with name %s and description %s",
						item_index(currentItem),
						item_name(currentItem),
						item_description(currentItem)
					);
				}

				refresh();
				pos_menu_cursor(menu);

			}	break;

			//Remove current item
			case 330://Delete
			case 263:{//Backspace
				if(itemsLen>0){
					//Get current item
					ITEM* item = current_item(menu);
					int index = item_index(item);

					//Deny removal of first item (Add...)
					if(index==0)
						break;

					//Free item data
					free(item_name(item));
					free(item_description(item));
					free(item_userptr(item));

					//Free item
					free_item(item);

					unpost_menu(menu);
						//Move items in front
						for(int i=index;i<itemsLen;++i)
							items[i]=items[i+1];

						//Decrement length
						--itemsLen;

						//Update the menu's items pointer
						set_menu_items(menu,items);
					post_menu(menu);
				}
			}	break;

			case ' ':
				menu_driver(menu,REQ_TOGGLE_ITEM);
				break;

			//Quit application
			case 113://Q
			case EOF:
			case KEY_CTRL('d'):
				goto End;
		}
	}

	End:
	//Free allocated memory
	unpost_menu(menu);
	for(ITEM** item=items;*item!=NULL;++item)
		free_item(*item);
	free(items);
	free_menu(menu);
	endwin();

	free(inputBuffer);

	return EXIT_SUCCESS;
}
