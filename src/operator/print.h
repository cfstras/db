#pragma once

#include <memory>

#include "operator.h"

class PrintOperator : public Operator {
public:
	PrintOperator(std::shared_ptr<Operator> source);

	virtual void open();
	virtual bool next();
	virtual void close();
	virtual std::vector<Register*> getOutput();

private:
	DISALLOW_COPY_AND_ASSIGN(PrintOperator);

	std::shared_ptr<Operator> source;
};
