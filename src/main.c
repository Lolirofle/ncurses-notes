#include <curses.h>
#include <menu.h>
#include <stdlib.h>

#define ARRAY_SIZE(a) (sizeof(a)/sizeof(a[0]))
#define KEY_CTRL(x)  ((x)&0x1F)

int main(){
	//Initiate screen
	initscr();
		cbreak();
		noecho();

	//Enable builtin menu control with keys
	keypad(stdscr,TRUE);

	//Initiate items
	unsigned int itemsLen = 5,
	             itemsSize = 10;
	ITEM** items = calloc(itemsSize+1,sizeof(ITEM*));
	items[0] = new_item("[Add...]","Add a new item to the list");
	items[1] = new_item("Choice 1",NULL);//set_item_userptr(items[0],)
	items[2] = new_item("Choice 2",NULL);
	items[3] = new_item("Choice 3",NULL);
	items[4] = new_item("Choice 4",NULL);

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

				//Write to last lines
				mvprintw(LINES-1,0,"You have chosen item %i with name %s and description %s",
					item_index(currentItem),
					item_name(currentItem),
					item_description(currentItem)
				);

				refresh();
				pos_menu_cursor(menu);

				//If first item is selected (Add...)
				if(item_index(currentItem)==0 && itemsLen<itemsSize){
					unpost_menu(menu);
						//Initialize new item
						items[itemsLen] = new_item("Added item",NULL);

						//Increment length
						++itemsLen;

						//Update the menu's items pointer
						set_menu_items(menu,items);
					post_menu(menu);
				}

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
}
