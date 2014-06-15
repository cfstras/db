#pragma once

#include "operator.h"

#include <iostream>
#include <vector>

class DummyOperator : public Operator {
public:
	DummyOperator(std::vector<std::vector<std::string>> source) : Operator(),
		source(source), registers(), index(0)
	{
		size_t maxSize = 0;
		for (const std::vector<std::string>& row : source)
			if (row.size() > maxSize) maxSize = row.size();
		registers.resize(maxSize);
	}

	virtual void open() {
		for (size_t i=0; i<registers.size(); i++) {
			registers[i] = new Register("");
		}
	}

	virtual bool next() {
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

	virtual void close() {
		for (size_t i=0; i<registers.size(); i++) {
			delete registers[i];
		}
		index = 0;
		size_t len = registers.size();
		registers.clear();
		registers.resize(len);
	}

	virtual std::vector<Register*> getOutput() {
		return registers;
	}

private:
	DISALLOW_COPY_AND_ASSIGN(DummyOperator);

	std::vector<std::vector<std::string>> source;
	std::vector<Register*> registers;
	size_t index;
};
