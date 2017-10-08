#pragma once
#include "Import.hpp"
#include "Logging.hpp"
#include "Solver.hpp"

#define autoref auto&

class iGraph;

class Symbol
{		
public:
	af::array value_backward_grad;
	af::array value_backward_x;
	af::array value_backward;
	af::array value_forward;

public:
	vector<Symbol*> sym_in;
	vector<Symbol*> sym_out;

public:
	iGraph* model;
	
public:
	const string name;
	bool be_const;

private:
	unsigned long cur_forward;
	unsigned long cur_backward;

public:
	Symbol(const string name)
		:name(name), be_const(false), cur_forward(0), cur_backward(0)
	{
		;
	}

	virtual ~Symbol()
	{
		;
	}

public:
	virtual void forward() = 0;
	virtual void backward() = 0;

public:
	void set(const af::array& value_init)
	{
		value_forward = value_init;
		value_backward = af::constant(0.f, value_init.dims());
		value_backward_x = value_backward;
		value_backward_grad = value_backward;
	}

public:
	void trigger_forward()
	{
		++ cur_forward;
		if (cur_forward >= sym_in.size())
		{
#ifdef BLDEBUG
			logout.record() << name;
#endif
			forward();
			value_backward = af::constant(0.f, value_forward.dims());
#ifdef BLDEBUG
			logout << " = " << value_forward.dims(0) << ", " << value_forward.dims(1) << ", "
				<< value_forward.dims(2) << ", " << value_forward.dims(3);
			logout.flush();
#endif

			for(auto i=sym_out.begin(); i!=sym_out.end(); ++i)
			{
				(*i)->trigger_forward();
			}
			cur_forward = 0;
		}
	}

	void trigger_backward()
	{
		++ cur_backward;
		if (cur_backward >= sym_out.size())
		{
#ifdef BLDEBUG
			logout.record() << name;
			logout << " = " << value_backward.dims(0) << ", " << value_backward.dims(1) << ", "
				<< value_backward.dims(2) << ", " << value_backward.dims(3);
#endif
			backward();
			for(auto i=sym_in.begin(); i!=sym_in.end(); ++i)
			{
				(*i)->trigger_backward();
			}
			cur_backward = 0;
		}
	}

public:
	Symbol& t();
	Symbol& operator[](Symbol& a);
};

class SymDatum
	:public Symbol
{
public:
	SymDatum(const string name)
		:Symbol("Data Input = " + name)
	{
		;
	}

public:
	virtual void forward() override
	{
		;
	}

	virtual void backward() override
	{
		;
	}
};

class SymVariable
	:public Symbol
{
public:
	SymVariable(const string name)
		:Symbol("Variable = " + name)
	{
		;
	}

public:
	virtual void forward() override
	{
		;
	}

	virtual void backward() override
	{
		global_sovler->gradient(value_backward_grad, value_backward_x, value_forward, value_backward);
	}
};

class SymLoss
	:public Symbol
{
public:
	SymLoss(const string name, Symbol& node)
		:Symbol("Loss = " + name)
	{
		node.sym_out.push_back(this);
		this->sym_in.push_back(&node);
	}

public:
	virtual void forward() override
	{
		value_forward = sym_in[0]->value_forward;
		for(auto idim=0; idim<4; ++idim)
		{
			value_forward = af::sum(value_forward, idim);
		}
	}

	virtual void backward() override
	{
		sym_in[0]->value_backward = af::constant(1.f, sym_in[0]->value_forward.dims());
	}
};

class SymConst
	:public Symbol
{
public:
	const float const_value;

public:
	SymConst(float value)
		:Symbol("Const = " + bearlet_cast<float>(value)), const_value(value)
	{
		be_const = true;
	}

public:
	virtual void forward() {;}
	virtual void backward() {;}
};

class SymbolGC
{
protected:
	vector<Symbol*> symbol_pool;

public:
	Symbol* operator()(Symbol* elem)
	{
		symbol_pool.push_back(elem);
		return elem;
	}

	void clear()
	{
		for(auto i=symbol_pool.begin(); i!=symbol_pool.end(); ++i)
		{
			delete [] (*i);
		}
	}
};

SymbolGC sym_generator;

template<typename Tnode>
Symbol& sym_new_node(Symbol& a, Symbol& b)
{
	Symbol* node = sym_generator(new Tnode);

	if (a.model != b.model)
		throw string("Two Symbol could not match.");
	else
		node->model = a.model;

	a.sym_out.push_back(node);
	b.sym_out.push_back(node);
	node->sym_in.push_back(&a);
	node->sym_in.push_back(&b);

	return *node;
}

template<typename Tnode>
Symbol& sym_new_node(Symbol& a)
{
	Symbol* node = sym_generator(new Tnode);

	node->model = a.model;

	a.sym_out.push_back(node);
	node->sym_in.push_back(&a);

	return *node;
}

class SymPrint
	:public Symbol
{
public:
	SymPrint()
		:Symbol("Print")
	{
		;
	}

public:
	virtual void forward()
	{
		logout.record() << print_array(sym_in[0]->value_forward);
		value_forward = sym_in[0]->value_forward;
	}

	virtual void backward()
	{
		logout.record() << print_array(value_backward);
		sym_in[0]->value_backward = value_backward;
	}
};

Symbol& print(Symbol& a)
{
	return sym_new_node<SymPrint>(a);
}

inline
af::array normal_initial(int dim1, int dim2 = 1, int dim3 = 1, int dim4 = 1)
{
	return af::randn(dim1, dim2, dim3, dim4);
}

inline 
af::array xavier_initial(int dim1, int dim2 = 1, int dim3 = 1, int dim4 = 1)
{
	return af::randn(dim1, dim2, dim3, dim4) / sqrt(dim1);
}


class iGraph
{
protected:
	vector<Symbol*>		DataSources;
	vector<Symbol*>		Variables;
	vector<Symbol*>		Losses;

public:
	Symbol& data_source(const string name, const af::array& arr)
	{
		Symbol* node = sym_generator(new SymDatum(name));
		node->model = this;
		node->set(arr);
		DataSources.push_back(node);

		return *node;
	}

	Symbol& variable(const string name, const af::array& arr)
	{
		Symbol* node = sym_generator(new SymVariable(name));
		node->model = this;
		node->set(arr);
		Variables.push_back(node);

		return *node;
	}

	Symbol& loss(const string name, Symbol& target)
	{
		Symbol* node = sym_generator(new SymLoss(name, target));
		node->model = this;
		Losses.push_back(node);

		return *node;
	}

	Symbol& operator()(float elem)
	{
		Symbol* node = sym_generator(new SymConst(elem));
		node->model = this;
		DataSources.push_back(node);

		return *node;
	}
public:
	void perform()
	{
		for(auto i=DataSources.begin(); i!=DataSources.end(); ++i)
		{
			(*i)->trigger_forward();
		}

		for(auto i=Variables.begin(); i!=Variables.end(); ++i)
		{
			(*i)->trigger_forward();
		}
	}

	void learn()
	{
		for(auto i=Losses.begin(); i!=Losses.end(); ++i)
		{
			(*i)->trigger_backward();
		}
	}

	void test(function<void(Symbol&)> fn_proc)
	{
		for(auto i=Losses.begin(); i!= Losses.end(); ++i)
		{
			fn_proc(**i);
		}
	}

	void report()
	{
		logout.record() << "[iGraph] Objectives:";
		for(auto i=Losses.begin(); i!= Losses.end(); ++i)
		{
			float res = *((*i)->value_forward.host<float>());
			logout.record() <<"[iGraph] Loss of " << (*i)->name << " = " << res;
		}
	}

	void train(int epos, function<void(iGraph&, int)> fn_mid_train = [](iGraph&, int){})
	{
		int round = epos;
		int portion = epos / 100;
		int percent = 0;
		logout.record() << "[iGraph] Training -> ";
		while(epos --> 0)
		{
			fn_mid_train(*this,  round - epos);
			perform();
			if (round - epos >= percent * portion)
			{
				logout.record() << "[iGraph Train] " << percent << "% have been done.";
				logout.flush();
				++ percent;
			}
			learn();
		}
	}
};


