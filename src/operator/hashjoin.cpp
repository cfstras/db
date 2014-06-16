#include "operator/hashjoin.h"

using namespace std;

HashJoinOperator::HashJoinOperator(shared_ptr<Operator> leftOperator,
		shared_ptr<Operator> rightOperator,
		size_t leftColumn, size_t rightColumn) :
		leftOperator(leftOperator), rightOperator(rightOperator),
		leftColumn(leftColumn), rightColumn(rightColumn)
{
}

void HashJoinOperator::open() {
	leftOperator->open();
	const auto &leftRegs = leftOperator->getOutput();
	Register* left = leftRegs[leftColumn];
	leftLen = leftRegs.size();

	// read left input
	while (leftOperator->next()) {
		if (hashTable.find(*left) != hashTable.end()) {
			continue; // key is already in there
		}
		vector<Register> regs;
		regs.reserve(leftLen);
		for (size_t i=0; i<leftLen; i++) {
			regs.emplace_back(Register(*leftRegs[i]));
		}
		size_t ind = entries.size();
		entries.push_back(regs);
		hashTable.emplace(make_pair(entries[ind][leftColumn], ind));
	}
	leftOperator->close();

	rightOperator->open();

	rightRegs = rightOperator->getOutput();
	rightLen = rightRegs.size();
	right = rightRegs[rightColumn];

	output.resize(leftLen + rightLen - 1);
	for (size_t i=0; i<output.size(); i++) {
		output[i] = new Register(0);
	}
}

bool HashJoinOperator::next() {
	auto it = hashTable.end();
	do {
		if (!rightOperator->next()) return false;
		it = hashTable.find(*right);
	} while (it == hashTable.end());

	// found one.
	// copy the values into our output registers.
	// this means an additional copy, but since getOutput() is not marked const,
	// we have to assume that the caller does not call it regularly, so just
	// setting new pointers would do nothing.
	vector<Register>& leftRegs = entries[it->second];

	for (size_t i=0; i<leftLen; i++) {
		output[i]->set(leftRegs[i]);
	}
	size_t tempLeftLen = leftLen;
	for (size_t i=0; i<rightLen; i++) {
		if (i == rightColumn) {
			tempLeftLen--; // skip this one
			continue;
		}
		output[tempLeftLen+i]->set(*rightRegs[i]);
	}
	return true;
}

vector<Register*> HashJoinOperator::getOutput() {
	return output;
}

void HashJoinOperator::close() {
	entries.clear();
	hashTable.clear();
}
