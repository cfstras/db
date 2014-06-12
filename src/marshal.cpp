#include "marshal.h"

using namespace std;

Marshal::Marshal(shared_ptr<Schema> schema) : schema(schema) {

}

vector<Register*> Marshal::marshal(const Record& source) {
	return vector<Register*>();
}

void Marshal::marshal(const Record& source, vector<Register*> registers) {

}

Record Marshal::unmarshal(vector<Register*>& registers) {
	return Record(0, "");
}
