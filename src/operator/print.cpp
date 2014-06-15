#include "operator/print.h"

#include <iostream>

using namespace std;

PrintOperator::PrintOperator(shared_ptr<Operator> source) : Operator(),
source(source)
{
}

void PrintOperator::open() {
	source->open();
}

bool PrintOperator::next() {
	if (!source->next()) return false;
	cout << tupleToString(source->getOutput()) << endl;
	return true;
}

void PrintOperator::close() {
	source->close();
}

vector<Register*> PrintOperator::getOutput() {
	return vector<Register*>();
}
