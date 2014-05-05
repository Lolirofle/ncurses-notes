//Notes API

#include <stdbool.h>

struct Notes{
	struct NoteEntry* entries;
};

struct NoteEntry{
	char* title;
	char* comment;
	void* data;
	unsigned long datetime;
	unsigned long deadline;
};

bool Notes_addEntry(struct Notes* notes,const struct NoteEntry* entry);
bool Notes_insertEntry(struct Notes* notes,size_t index,const struct NoteEntry* entry);
bool Notes_removeEntry(struct Notes* notes,const struct NoteEntry* entry);
bool Notes_removeEntryByIndex(struct Notes* notes,unsigned int index);
bool Notes_moveEntry(struct Notes* notes,size_t from,size_t to);
bool Notes_swapEntry(struct Notes* notes,size_t a,size_t b);

bool Notes_loadFile(const char* filename,struct Notes* out);
bool Notes_saveFile(const char* filename,const struct Notes* notes);
bool Notes_unload(struct Notes* notes);
