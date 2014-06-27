//Notes API

struct NotesImplementation{	
	void*(*init)();
	void(*free)(void*);

	bool(*getEntry)(const void* notesData,const struct NoteEntry* entry);
	bool(*addEntry)(void* notesData,const struct NoteEntry* entry);
	bool(*insertEntry)(void* notesData,size_t index,const struct NoteEntry* entry);
	bool(*removeEntry)(void* notesData,const struct NoteEntry* entry);
	bool(*removeEntryByIndex)(void* notesData,unsigned int index);
	bool(*moveEntry)(void* notesData,size_t from,size_t to);
	bool(*swapEntry)(void* notesData,size_t a,size_t b);

	bool(*forAllEntries)(void* notesData,bool(*func)(struct NoteEntry*,void* userData),void* userData);

	bool load(void*,const struct NotesImplementation* impl,void* out_notesData);
	bool save(void*,const struct NotesImplementation* impl,const void* notesData);
};
