#ifndef __LOLIROFLE_NOTES_MENU_H_INCLUDED__
#define __LOLIROFLE_NOTES_MENU_H_INCLUDED__

struct MenuData{
	MENU* menu;

	ITEM** items;
	unsigned int itemsLen,
	             itemsSize;
};

struct EntryData{
	char* title;
	char* description;
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
		struct{
			int(*perform)(struct MenuData* menuData);
		}button;
	};
};

#endif
