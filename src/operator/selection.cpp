#include "selection.h"

using namespace std;

SelectionOperator::SelectionOperator(shared_ptr<Operator> source,
		size_t haystack_row, const Register &needle) :
		source(source), row(haystack_row), value(0)
{
	value.set(needle);
}

void SelectionOperator::open() {
	source->open();
}

bool SelectionOperator::next() {
	row = row+1;
	return false;
}

void SelectionOperator::close() {
	source->close();
}

vector<Register*> SelectionOperator::getOutput() {
	return source->getOutput();
}
