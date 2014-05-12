#include "input.h"

#include <curses.h>
#include <ctype.h>
#include <wchar.h>
#include <stdbool.h>

size_t getnstr_custom(char* buffer,size_t maxLength){
	size_t length = 0;
	int c,initX,initY;
	getyx(stdscr,initY,initX);
	bool insertMode = false;

	while(true){
		switch((c=getch())){
			case KEY_ENTER:
			case '\n':
			case '\r':
				goto Return;
			
			case KEY_BACKSPACE:
				if(length>0){
					--buffer;
					--length;

					int x,y;
					getyx(stdscr,y,x);
					move(y,x-1);
					delch();
				}
				break;

			case KEY_DC:{//Delete (330)
				int x,y,i;
				getyx(stdscr,y,x);

				char* tmpBuffer = buffer;

				i = x-initX;
				if(i<length){
					do{
						++i;
						*tmpBuffer = *(tmpBuffer+1);
						++tmpBuffer;
					}while(i<length);
					--length;
					delch();
				}

			}	break;

			case KEY_LEFT:{
				int x,y;
				getyx(stdscr,y,x);

				if(x>initX){
					move(y,x-1);
					--buffer;
				}
			}	break;

			case KEY_RIGHT:{
				int x,y;
				getyx(stdscr,y,x);

				if(x<initX+length){
					move(y,x+1);
					++buffer;
				}
			}	break;

			case 262:{//Home
				int x,y;
				getyx(stdscr,y,x);

				move(y,initX);
				buffer = buffer - (x-initX);

			}	break;

			case 360:{//End
				int x,y;
				getyx(stdscr,y,x);

				move(y,initX+length);
				buffer = buffer - (x-initX) + length;

			}	break;

			case 331:{//Insert
				insertMode=!insertMode;

			}	break;

			default:{
				if(length<maxLength){
					int x,y;
					getyx(stdscr,y,x);

					//If in insert mode
					if(insertMode){
						if(length>0){
							char* tmpBuffer = buffer - (x-initX) + length;

							while(tmpBuffer > buffer){
								*tmpBuffer = *(tmpBuffer-1);
								--tmpBuffer;
							}

							addnstr(buffer,length - (x-initX) + 1);
							move(y,x);
						}

						*buffer++ = c;
						++length;
					}
					//If in overtype mode
					else{
						*buffer++ = c;

						//If at last position
						if(length == x-initX)
							++length;
					}

					addch(c);
				}
			}	break;
		}
	}

	Return:
		return length;
}
