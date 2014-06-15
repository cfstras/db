#pragma once

#include <memory>

#include "operator.h"
#include "register.h"

class SelectionOperator : public Operator {
public:
	SelectionOperator(std::shared_ptr<Operator> source, size_t haystack_row,
			const Register &needle);

	virtual void open();
	virtual bool next();
	virtual void close();
	virtual std::vector<Register*> getOutput();

private:
	DISALLOW_COPY_AND_ASSIGN(SelectionOperator);

	std::shared_ptr<Operator> source;
	size_t row;
	Register value;
};
