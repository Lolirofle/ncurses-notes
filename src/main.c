#include <curses.h>
#include <menu.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include <form.h>
#include <locale.h>

#include "exit.h"
#include "menu.h"

#define ARRAY_SIZE(a) (sizeof(a)/sizeof(a[0]))
#define KEY_CTRL(x)  ((x)&0x1F)

#define INPUTBUFFER_LENGTH 512
char* inputBuffer;

static ITEM* Item_createEntry(unsigned int id,char* title,char* description){
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
		.id          = id,
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
		//Initate form fields
		FIELD* formFields[] = {
			new_field(1,COLS-2,2,1,0,0),
			new_field(10,COLS-2,5,1,0,0),
			NULL
		};
		
		//Field options
		for(unsigned char i=sizeof(formFields)/sizeof(formFields[0]);i-->0;){
			set_field_back(formFields[i],A_UNDERLINE);//Enable underlined formFields
			field_opts_off(formFields[i],O_AUTOSKIP); //Disable auto skipping to next formFields when filled up
			field_opts_off(formFields[i],O_STATIC);
			set_max_field(formFields[i],512);
		}

		//Initiate form
		FORM* form = new_form(formFields);
		post_form(form);
		
		//Refresh display
		refresh();

		//Print fields text
		mvprintw(1,1,"Title:");
		mvprintw(4,1,"Description:");

		//Setup form input mode
		enum{
			MODE_OVERLAY,
			MODE_INSERT,
		}mode = MODE_INSERT;
		form_driver(form,REQ_INS_MODE);

		enum{
			ESCAPE_FALSE,
			ESCAPE_TRUE,
			ESCAPE_CSI,
		}escaped = ESCAPE_FALSE;
		#define ESCAPEBUFFER_SIZE 16
		char escapeBuffer[ESCAPEBUFFER_SIZE];
		unsigned char escapeBufferLength = 0;

		//Fetch input
		int ch;
		while((ch = getch())){
			//http://en.wikipedia.org/wiki/ANSI_escape_code
			if(escaped == ESCAPE_TRUE){
				switch(ch){
					case '['://TODO: These will match every '['
						escaped = ESCAPE_CSI;
						break;

					case 0  ... 63 :
					case 96 ... 255:
						escapeBuffer[escapeBufferLength++] = ch;

						if(escapeBufferLength>=ESCAPEBUFFER_SIZE)
							goto EscapeNormal_break;
						break;

					default:
					EscapeNormal_break:
						escaped = ESCAPE_FALSE;
						escapeBufferLength = 0;
						break;
				}
			}else if(escaped ==ESCAPE_CSI){
				switch(ch){
					case 'D'://Word left
						form_driver(form,REQ_PREV_WORD);
						goto EscapeCsi_break;

					case 'C'://Word right
						form_driver(form,REQ_NEXT_WORD);
						goto EscapeCsi_break;

					case 0  ... 63 :
					case 127 ... 255:
						escapeBuffer[escapeBufferLength++] = ch;
						
						if(escapeBufferLength>=ESCAPEBUFFER_SIZE)
							goto EscapeCsi_break;
						break;

					default:
					EscapeCsi_break:
						escaped = ESCAPE_FALSE;
						escapeBufferLength = 0;
						break;
				}
			}else{
				switch(ch){
					case 9://Tab
						form_driver(form,REQ_NEXT_FIELD);
						break;

					case 353://Shift + Tab
						form_driver(form,REQ_PREV_FIELD);
						break;

					case KEY_DOWN:
						form_driver(form,REQ_NEXT_LINE);
						break;

					case KEY_UP:
						form_driver(form,REQ_PREV_LINE);
						break;

					case KEY_LEFT:
						form_driver(form,REQ_PREV_CHAR);
						break;

					case KEY_RIGHT:
						form_driver(form,REQ_NEXT_CHAR);
						break;

					case 262://Home
						form_driver(form,REQ_BEG_LINE);
						break;

					case 360://End
						form_driver(form,REQ_END_LINE);
						break;

					case KEY_BACKSPACE:
						form_driver(form,REQ_DEL_PREV);
						break;

					case 8://CTRL + Backspace (Why is this assigned to the ASCII BS/backspace on my current setup?)
						form_driver(form,REQ_DEL_WORD);
						break;

					case KEY_DC://Delete
						form_driver(form,REQ_DEL_CHAR);
						break;

					case 27://Escape
						escaped = ESCAPE_TRUE;
						break;

					case 155://CSI Escape
						escaped = ESCAPE_CSI;
						break;

					case KEY_ENTER:
					case 10:
					case 13:
						if(form_driver(form,REQ_VALIDATION)==E_OK)
							goto Form_complete;
						break;

					case 331://Insert
						switch(mode){
							case MODE_OVERLAY:
								form_driver(form,REQ_INS_MODE);
								mode = MODE_INSERT;
								break;

							case MODE_INSERT:
								form_driver(form,REQ_OVL_MODE);
								mode = MODE_OVERLAY;
								break;
						}
						break;

					default:
						form_driver(form,ch);
						break;
				}
			}
		}

		Form_complete:{
			char* title       = field_buffer(formFields[0],0),
			    * description = field_buffer(formFields[1],0);
			size_t titleLen       = strlen(title)+1,
			       descriptionLen = strlen(description)+1;

			//Silently ignore empty input
			//if(titleLen == 0)
			//	return EXIT_SUCCESS;

			//Copy over buffer to newly allocated block
			char* titleStr,
			    * descriptionStr;

			if(titleLen == 1)
				titleStr = NULL;
			else{
				if(!(titleStr = malloc(titleLen)))
					return EXIT_ALLOCATION_FAILURE;
				memcpy(titleStr,title,titleLen);
			}

			if(descriptionLen == 1)
				descriptionStr = NULL;
			else{
				if(!(descriptionStr = malloc(descriptionLen)))
					return EXIT_ALLOCATION_FAILURE;
				memcpy(descriptionStr,description,descriptionLen);	
			}

			//Free form and its fields
			unpost_form(form);
			free_form(form);
			for(unsigned char i=sizeof(formFields)/sizeof(formFields[0]);i-->0;)
				free_field(formFields[i]);

			//Update menu with a new entry
			unpost_menu(menuData->menu);
				//Initiate new item
				if(!(menuData->items[menuData->itemsLen] = Item_createEntry(menuData->itemsNextId++,titleStr,descriptionStr)))
					return EXIT_ITEMENTRYCREATION_FAILURE;

				//Increment length
				++menuData->itemsLen;

				//Update the menu's items pointer
				set_menu_items(menuData->menu,menuData->items);
			post_menu(menuData->menu);
			
			//Refresh display
			refresh();
		}
	}

	return EXIT_SUCCESS;
}

int main(){
	int exitCode = EXIT_SUCCESS;
	struct MenuData menuData;

	//Unicode input with forms doesn't work without this?
	setlocale(LC_ALL,"");

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
	menuData.itemsNextId = 1;
	menuData.itemsLen    = 1;
	menuData.itemsSize   = 10;
	menuData.items       = calloc(menuData.itemsSize+1,sizeof(ITEM*));
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
