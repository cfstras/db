#pragma once

#include <memory>
#include <unordered_map>

#include "operator.h"
#include "register.h"

class HashJoinOperator : public Operator {
public:
	HashJoinOperator(std::shared_ptr<Operator> leftOperator,
			std::shared_ptr<Operator> rightOperator,
			size_t leftColumn, size_t rightColumn);

	virtual void open();
	virtual bool next();
	virtual void close();
	virtual std::vector<Register*> getOutput();

private:
	DISALLOW_COPY_AND_ASSIGN(HashJoinOperator);

	std::shared_ptr<Operator> leftOperator, rightOperator;
	size_t leftColumn, rightColumn;
	Register* right;
	std::vector<Register*> rightRegs;
	size_t leftLen, rightLen;

	// stores the whole left output
	std::vector<std::vector<Register>> entries;
	// stores indizes into entries
	std::unordered_map<Register*, size_t> hashTable;

	std::vector<Register*> output;
};
