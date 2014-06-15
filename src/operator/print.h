#pragma once

#include <iostream>
#include <memory>

#include "operator.h"

class PrintOperator : public Operator {
public:
	PrintOperator(std::shared_ptr<Operator> source) : Operator(),
			source(source)
	{
	}

	virtual void open() {
		source->open();
	}

	virtual bool next() {
		if (!source->next()) return false;
		std::cout << tupleToString(source->getOutput()) << std::endl;
		return true;
	}

	virtual void close() {
		source->close();
	}

	virtual std::vector<Register*> getOutput() {
		return std::vector<Register*>();
	}

private:
	DISALLOW_COPY_AND_ASSIGN(PrintOperator);

	std::shared_ptr<Operator> source;
};
