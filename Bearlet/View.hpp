#pragma once
#include "Import.hpp"
#include "iGraph.hpp"
#include "Operator.hpp"

class SymEmbedding
	:public Symbol
{
protected:
	bool be_fixed;

public:
	SymEmbedding(bool be_fixed = false)
		:Symbol("Embedding"), be_fixed(be_fixed)
	{
		;
	}

public:
	virtual void forward() override
	{
		value_forward = sym_in[0]->value_forward(sym_in[1]->value_forward.as(s32), af::span);
	}

	//There needs to be optimized.
	//There needs to be optimized.
	//There needs to be optimized.
	//There needs to be optimized.
	//There needs to be optimized.
	//There needs to be optimized.
	virtual void backward() override
	{
		if (be_fixed == false)
		{
			af::array tmp_grad = sym_in[0]->value_backward_grad(sym_in[1]->value_forward.as(s32), af::span);
			af::array tmp_x = sym_in[0]->value_backward_x(sym_in[1]->value_forward.as(s32), af::span); 
			af::array tmp_v = sym_in[0]->value_forward(sym_in[1]->value_forward.as(s32), af::span);
		
			global_sovler->gradient(tmp_grad, tmp_x, tmp_v, value_backward);

			sym_in[0]->value_backward_grad(sym_in[1]->value_forward.as(s32), af::span) = tmp_grad;
			sym_in[0]->value_backward_x(sym_in[1]->value_forward.as(s32), af::span) = tmp_x;
			sym_in[0]->value_forward(sym_in[1]->value_forward.as(s32),af::span) = tmp_v;
		}
	}
};

Symbol& Symbol::operator[](Symbol& a)
{
	return sym_new_node<SymEmbedding>(*this, a);
}

class SymSum
	:public Symbol
{
protected:
	int along_dimenstion;

public:
	SymSum(int along_dimenstion)
		:Symbol("Sum"), along_dimenstion(along_dimenstion)
	{
		;
	}

public:
	virtual void forward()
	{
		value_forward = af::sum(sym_in[0]->value_forward, along_dimenstion);
	}

	virtual void backward()
	{
		switch(along_dimenstion)
		{
			case 0:	
				sym_in[0]->value_backward += af::tile(value_backward, sym_in[0]->value_forward.dims(0));
				break;
			case 1:	
				sym_in[0]->value_backward += af::tile(value_backward, 1, sym_in[0]->value_forward.dims(1));
				break;
			case 2:	
				sym_in[0]->value_backward += af::tile(value_backward, 1, 1, sym_in[0]->value_forward.dims(2));
				break;
			case 3:	
				sym_in[0]->value_backward += af::tile(value_backward, 1, 1, 1, sym_in[0]->value_forward.dims(3));
				break;	
		}
	}
};

Symbol& sum(Symbol& a, int along_dimenstion)
{
	Symbol* node = new SymSum(along_dimenstion);
	
	node->model = a.model;

	a.sym_out.push_back(node);
	node->sym_in.push_back(&a);

	return *node;
}
