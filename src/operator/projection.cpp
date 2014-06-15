#include "operator/projection.h"

using namespace std;

ProjectionOperator::ProjectionOperator(shared_ptr<Operator> source,
		vector<size_t> columns) :
		source(source), columns(columns)
{
}

void ProjectionOperator::open() {
	source->open();
	auto outs = source->getOutput();
	output.reserve(columns.size());
	for (size_t i : columns) {
		output.push_back(outs[i]);
	}
}

bool ProjectionOperator::next() {
	return source->next();
}

void ProjectionOperator::close() {
	source->close();
	output.clear();
}

vector<Register*> ProjectionOperator::getOutput() {
	return output;
}
