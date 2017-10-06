#pragma once
#include "Import.hpp"
#include "Logging.hpp"
#include "iGraph.hpp"

class SymPlus
	:public Symbol
{
public:
	SymPlus()
		:Symbol("Plus")
	{
		;
	}

public:
	virtual void forward() override
	{
		if (sym_in[0]->be_const == true)
			value_forward = dynamic_cast<SymConst*>(sym_in[0])->const_value + sym_in[1]->value_forward;
		else if (sym_in[1]->be_const == true)
			value_forward = sym_in[0]->value_forward + dynamic_cast<SymConst*>(sym_in[1])->const_value;
		else
			value_forward = sym_in[0]->value_forward + sym_in[1]->value_forward;
	}

	virtual void backward() override
	{
		if (sym_in[0]->be_const == false)
		{
			sym_in[0]->value_backward += value_backward;
		}
		
		if (sym_in[1]->be_const == false)
		{
			sym_in[1]->value_backward += value_backward;
		}
	}
};

Symbol& operator + (Symbol& a, Symbol& b)
{
	return sym_new_node<SymPlus>(a, b);
}

class SymMinus
	:public Symbol
{
public:
	SymMinus()
		:Symbol("Minus")
	{
		;
	}

public:
	virtual void forward() override
	{
		if (sym_in[0]->be_const == true)
			value_forward = dynamic_cast<SymConst*>(sym_in[0])->const_value - sym_in[1]->value_forward;
		else if (sym_in[1]->be_const == true)
			value_forward = sym_in[0]->value_forward - dynamic_cast<SymConst*>(sym_in[1])->const_value;
		else
			value_forward = sym_in[0]->value_forward - sym_in[1]->value_forward;
	}

	virtual void backward() override
	{
		if (sym_in[0]->be_const == false)
		{
			sym_in[0]->value_backward += value_backward;
		}
		
		if (sym_in[1]->be_const == false)
		{
			sym_in[1]->value_backward -= value_backward;
		}
	}
};

class SymNeg
	:public Symbol
{
public:
	SymNeg()
		:Symbol("Neg")
	{
		;
	}

public:
	virtual void forward() override
	{
		value_forward = -sym_in[0]->value_forward;
	}

	virtual void backward() override
	{
		sym_in[0]->value_backward -= value_backward;
	}
};

Symbol& operator - (Symbol& a, Symbol& b)
{
	return sym_new_node<SymMinus>(a, b);
}

Symbol& operator - (Symbol& a)
{
	return sym_new_node<SymNeg>(a);
}


class SymMultiply
	:public Symbol
{
public:
	SymMultiply()
		:Symbol("Multiply")
	{
		;
	}

public:
	virtual void forward() override
	{
		if (sym_in[0]->be_const == true)
			value_forward = dynamic_cast<SymConst*>(sym_in[0])->const_value * sym_in[1]->value_forward;
		else if (sym_in[1]->be_const == true)
			value_forward = sym_in[0]->value_forward * dynamic_cast<SymConst*>(sym_in[1])->const_value;
		else
			value_forward = sym_in[0]->value_forward * sym_in[1]->value_forward;
	}

	virtual void backward() override
	{
		if (sym_in[0]->be_const == false)
		{
			if (sym_in[1]->be_const == true)
				sym_in[0]->value_backward += value_backward * dynamic_cast<SymConst*>(sym_in[1])->const_value;
			else
				sym_in[0]->value_backward += value_backward * sym_in[1]->value_forward;
		}
		
		if (sym_in[1]->be_const == false)
		{
			if (sym_in[0]->be_const == true)
				sym_in[1]->value_backward += value_backward * dynamic_cast<SymConst*>(sym_in[0])->const_value;
			else
				sym_in[1]->value_backward += value_backward * sym_in[0]->value_forward;
		}
	}
};

Symbol& operator % (Symbol& a, Symbol& b)
{
	return sym_new_node<SymMultiply>(a, b);
}

class SymMatrixMultiply
	:public Symbol
{
public:
	SymMatrixMultiply()
		:Symbol("MatrixMultiply")
	{
		;
	}

public:
	virtual void forward() override
	{
		value_forward = af::matmul(sym_in[0]->value_forward, sym_in[1]->value_forward);
	}

	virtual void backward() override
	{
		sym_in[0]->value_backward += af::matmul(value_backward, sym_in[1]->value_forward.T());
		sym_in[1]->value_backward += af::matmul(sym_in[0]->value_forward.T(), value_backward);
	}
};

Symbol& operator * (Symbol& a, Symbol& b)
{
	return sym_new_node<SymMatrixMultiply>(a, b);
}


class SymDivide
	:public Symbol
{
public:
	SymDivide()
		:Symbol("Divide")
	{
		;
	}

public:
	virtual void forward() override
	{
		if (sym_in[0]->be_const == true)
			value_forward = dynamic_cast<SymConst*>(sym_in[0])->const_value / sym_in[1]->value_forward;
		else if (sym_in[1]->be_const == true)
			value_forward = sym_in[0]->value_forward / dynamic_cast<SymConst*>(sym_in[1])->const_value;
		else
			value_forward = sym_in[0]->value_forward / sym_in[1]->value_forward;
	}

	virtual void backward() override
	{
		if (sym_in[0]->be_const == false)
		{
			if (sym_in[1]->be_const == true)
				sym_in[0]->value_backward += value_backward / dynamic_cast<SymConst*>(sym_in[1])->const_value;
			else
				sym_in[0]->value_backward += value_backward / sym_in[1]->value_forward;
		}
		
		if (sym_in[1]->be_const == false)
		{
			sym_in[1]->value_backward += - value_backward / sym_in[1]->value_forward * value_forward;
		}
	}
};

Symbol& operator / (Symbol& a, Symbol& b)
{
	return sym_new_node<SymDivide>(a, b);
}

class SymPower
	:public Symbol
{
public:
	SymPower()
		:Symbol("Power")
	{
		;
	}

public:
	virtual void forward() override
	{
		if (sym_in[0]->be_const == true)
			value_forward = af::pow(
					af::constant(dynamic_cast<SymConst*>(sym_in[0])->const_value, sym_in[1]->value_forward.dims()),
					sym_in[1]->value_forward);
		else if (sym_in[1]->be_const == true)
			value_forward = af::pow(
					sym_in[1]->value_forward,
					af::constant(dynamic_cast<SymConst*>(sym_in[0])->const_value, sym_in[1]->value_forward.dims()));	
		else
			value_forward = af::pow(sym_in[0]->value_forward, sym_in[1]->value_forward);
	}

	virtual void backward() override
	{
		if (sym_in[0]->be_const == false)
		{
			sym_in[0]->value_backward += sym_in[0]->value_backward = value_backward * value_forward * af::log(sym_in[0]->value_forward);
		}
		
		if (sym_in[1]->be_const == false)
		{
			if (sym_in[0]->be_const == true)
				sym_in[1]->value_backward += value_backward * value_forward * sym_in[1]->value_forward / dynamic_cast<SymConst*>(sym_in[0])->const_value;
			else
				sym_in[1]->value_backward += value_backward * value_forward * sym_in[1]->value_forward / sym_in[0]->value_forward;
		}	
	}
};

Symbol& operator ^ (Symbol& a, Symbol& b)
{
	return sym_new_node<SymPower>(a, b);
}

class SymExp
	:public Symbol
{
public:
	SymExp()
		:Symbol("Exp")
	{
		;
	}

public:
	virtual void forward() override
	{
		value_forward = af::exp(sym_in[0]->value_forward);
	}

	virtual void backward() override
	{
		sym_in[0]->value_backward += value_backward * value_forward;
	}
};

Symbol& exp(Symbol& a)
{
	return sym_new_node<SymExp>(a);
}

class SymLog
	:public Symbol
{
public:
	SymLog()
		:Symbol("Log")
	{
		;
	}

public:
	virtual void forward() override
	{
		value_forward = af::log(sym_in[0]->value_forward);
	}

	virtual void backward() override
	{
		sym_in[0]->value_backward += value_backward / sym_in[0]->value_forward;
	}
};

Symbol& log(Symbol& a)
{
	return sym_new_node<SymLog>(a);
}

class SymABS
	:public Symbol
{
public:
	SymABS()
		:Symbol("ABS")
	{
		;
	}

public:
	virtual void forward() override
	{
		value_forward = af::abs(sym_in[0]->value_forward);
	}

	virtual void backward() override
	{
		sym_in[0]->value_backward += value_backward % (-2 * af::sign(sym_in[0]->value_forward) + 1);
	}
};

Symbol& abs(Symbol& a)
{
	return sym_new_node<SymABS>(a);
}

class SymTransport
	:public Symbol
{
public:
	SymTransport()
		:Symbol("Transport")
	{
		;
	}

public:
	virtual void forward() override
	{
		value_forward = sym_in[0]->value_forward.T();
	}

	virtual void backward() override
	{
		sym_in[0]->value_backward += value_backward.T();
	}
};

Symbol& Symbol::t()
{
	return sym_new_node<SymTransport>(*this);
}

class SymSigmoid
	:public Symbol
{
public:
	SymSigmoid()
		:Symbol("Sigmoid")
	{
		;
	}

public:
	virtual void forward() override
	{
		value_forward = 1.f/(1.f + af::exp(-sym_in[0]->value_forward));
	}

	virtual void backward() override
	{
		sym_in[0]->value_backward += value_backward * (1.f - value_forward) * value_forward;
	}
};

Symbol& sigmoid(Symbol& a)
{
	return sym_new_node<SymSigmoid>(a);
}
