#include "marshal.h"

using namespace std;

Marshal::Marshal(Schema::Relation relation) : relation(relation) {
	//TODO
}

vector<Register*> Marshal::marshal(const Record& source) {
	vector<Register*> v(relation.attributes.size());
	marshal(source, v);
	return v;
}

void Marshal::marshal(const Record& source, vector<Register*> registers) {
	//TODO
}

Record Marshal::unmarshal(vector<Register*>& registers) {
	//TODO
	return Record(0, "");
}
