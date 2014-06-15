#include "operator/dummy.h"

using namespace std;

DummyOperator::DummyOperator(vector<vector<string>> source) : Operator(),
		source(source), registers(), index(0)
{
	size_t maxSize = 0;
	for (const vector<string>& row : source)
		if (row.size() > maxSize) maxSize = row.size();
	registers.resize(maxSize);
}

void DummyOperator::open() {
	for (size_t i=0; i<registers.size(); i++) {
		registers[i] = new Register("");
	}
}

bool DummyOperator::next() {
	if (index >= source.size()) return false;
	size_t i;
	for (i=0; i<source[index].size(); i++) {
		registers[i]->setString(source[index][i]);
	}
	for (;i<registers.size(); i++) {
		registers[i]->setString("");
	}
	index++;
	return true;
}

void DummyOperator::close() {
	for (size_t i=0; i<registers.size(); i++) {
		delete registers[i];
	}
	index = 0;
	size_t len = registers.size();
	registers.clear();
	registers.resize(len);
}

vector<Register*> DummyOperator::getOutput() {
	return registers;
}
