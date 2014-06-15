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
	inputRegister = source->getOutput()[row];
}

bool SelectionOperator::next() {
	while (source->next()) {
		if (*inputRegister == value) {
			return true;
		}
	}
	return false;
}

void SelectionOperator::close() {
	source->close();
}

vector<Register*> SelectionOperator::getOutput() {
	return source->getOutput();
}
