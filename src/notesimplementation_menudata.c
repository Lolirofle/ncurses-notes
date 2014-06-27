#include "notesimplementation.h"

static void* menuData_init(){
	return NULL;
}

static void menuData_free(void*){

}

static bool menuData_getEntry(const void* notesData,const struct NoteEntry* entry){
	return false;
}

static bool menuData_addEntry(void* notesData,const struct NoteEntry* entry){
	return false;
}

static bool menuData_insertEntry(void* notesData,size_t index,const struct NoteEntry* entry){
	return false;
}

static bool menuData_removeEntry(void* notesData,const struct NoteEntry* entry){
	return false;
}

static bool menuData_removeEntryByIndex(void* notesData,unsigned int index){
	return false;
}

static bool menuData_moveEntry(void* notesData,size_t from,size_t to){
	return false;
}

static bool menuData_swapEntry(void* notesData,size_t a,size_t b){
	return false;
}

bool menuData_forAllEntries(void* notesData,bool(*func)(struct NoteEntry*,void* userData),void* userData){
	return false
}

const struct NotesImplementation notesImplementation_menuData = {
	.init               = menuData_init,
	.free               = menuData_free,
	.getEntry           = menuData_getEntry,
	.addEntry           = menuData_addEntry,
	.insertEntry        = menuData_insertEntry,
	.removeEntry        = menuData_removeEntry,
	.removeEntryByIndex = menuData_removeEntryByIndex,
	.moveEntry          = menuData_moveEntry,
	.swapEntry          = menuData_swapEntry,

	.forAllEntries = menuData_forAllEntries
};
