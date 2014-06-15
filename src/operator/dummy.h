#pragma once

#include "operator.h"

#include <iostream>
#include <vector>

class DummyOperator : public Operator {
public:
	DummyOperator(std::vector<std::vector<std::string>> source);

	virtual void open();
	virtual bool next();
	virtual void close();
	virtual std::vector<Register*> getOutput();

private:
	DISALLOW_COPY_AND_ASSIGN(DummyOperator);

	std::vector<std::vector<std::string>> source;
	std::vector<Register*> registers;
	size_t index;
};
