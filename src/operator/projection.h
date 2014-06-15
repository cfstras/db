#pragma once

#include <memory>
#include <vector>
#include <cassert>

#include "operator.h"

class ProjectionOperator : public Operator {
public:
	ProjectionOperator(std::shared_ptr<Operator> source,
			std::vector<size_t> columns) :
			source(source), columns(columns)
	{
	}

	virtual void open() {
		source->open();
		auto outs = source->getOutput();
		output.reserve(columns.size());
		for (size_t i : columns) {
			output.push_back(outs[i]);
		}
	}

	virtual bool next() {
		return source->next();
	}

	virtual void close() {
		source->close();
		output.clear();
	}

	virtual std::vector<Register*> getOutput() {
		return output;
	}

private:
	DISALLOW_COPY_AND_ASSIGN(ProjectionOperator);

	std::shared_ptr<Operator> source;
	std::vector<Register*> output;
	std::vector<size_t> columns;
};
