#pragma once

#include <memory>
#include <vector>

#include "operator.h"

class ProjectionOperator : public Operator {
public:
	ProjectionOperator(std::shared_ptr<Operator> source,
			std::vector<size_t> columns);

	virtual void open();
	virtual bool next();
	virtual void close();
	virtual std::vector<Register*> getOutput();

private:
	DISALLOW_COPY_AND_ASSIGN(ProjectionOperator);

	std::shared_ptr<Operator> source;
	std::vector<Register*> output;
	std::vector<size_t> columns;
};
