
enum EntryAction{
	ENTRYACTIONS_INSERT,
	ENTRYACTIONS_ADD,
	ENTRYACTIONS_DELETE,
};

union EntryActionData{
	struct{}insert;
	struct{}add;
	struct{}delete;
}
