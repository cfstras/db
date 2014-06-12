#include "marshal.h"

using namespace std;

Marshal::Marshal(Schema schema) : schema(schema) {

}

vector<Register> marshal(const Record& source) {
	return vector<Register>();
}

void Marshal::marshal(const Record& source, vector<Register> registers) {

}

const Record unmarshal(vector<Register>& registers) {
	return Record(0, "");
}
