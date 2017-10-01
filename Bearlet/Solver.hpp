#pragma once
#include "Import.hpp"

class DecentAdaDelta
{
protected:
	float moment;
	float regularization;

public:
	DecentAdaDelta(float moment, float regularization)
		:moment(moment), regularization(regularization)
	{
		;
	}

	void set_parameter(float moment, float regularization)
	{
		this->moment = moment;
		this->regularization = regularization;
	}
public:
	void gradient(float& derv_grad, float& derv_x, float& elem, float& grad) const
	{
		derv_grad = moment * derv_grad + (1 - moment) * grad * grad;
		float derv_elem = sqrt(derv_x + regularization) / sqrt(derv_grad + regularization) * grad;
		derv_x = moment * derv_x + (1 - moment) * derv_elem * derv_elem;

		elem -= derv_elem;
	}

	void gradient(
			af::array& derv_grad,
			af::array& derv_x,
			af::array& elem,
			af::array& grad) const
	{
		derv_grad = moment * derv_grad + (1 - moment) * grad * grad; 
		derv_grad.eval();

		af::array derv_elem = af::sqrt(derv_x + regularization) / af::sqrt(derv_grad + regularization) * grad;
		derv_elem.eval();

		derv_x = moment * derv_x + (1 - moment) * derv_elem * derv_elem;
		derv_x.eval();

		elem -= derv_elem;
		elem.eval();
	}
};

DecentAdaDelta* global_sovler  = new DecentAdaDelta(0.6, 1e-6);

