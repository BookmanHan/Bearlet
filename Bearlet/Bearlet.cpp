#include "Import.hpp"
#include "Logging.hpp"
#include "Dataset.hpp"
#include "iGraph.hpp"
#include "Operator.hpp"
#include "View.hpp"

// To Design Batch Mode.
// +, -, %, *, log, exp, /, t(), abs 
// 1.大的张量不归一化直接用图，会产生 NAN 错误。但直接使用 sigmoid 不会，必须解决这个问题。
//
// TO DO:
// 1.Taylor 函数层。
// 2.设计测试用例，测试全部 iGraph 内容。

Symbol& sigm(Symbol& x)
{
	iGraph& model = *x.model;
	return model(1.f) / (model(1.f) + exp(-x));
}

Symbol& sigmoid_layer(Symbol& x, int n_input, int n_output)
{
	iGraph& model = *x.model;
	autoref W = model.variable("Hidden Layer", xavier_initial(n_output, n_input));
	return sigmoid(x * W.t());
}

Symbol& tanh_layer(Symbol& x, int n_input, int n_output)
{
	iGraph& model = *x.model;
	autoref W = model.variable("Hidden Layer", xavier_initial(n_input, n_output));
	autoref t = x * W;
	return (exp(t) - exp(-t))/(exp(t) + exp(-t));
}

Symbol& unit(iGraph& model, Symbol& triple_user, Symbol& triple_item)
{
	vector<Symbol*> pred;
	for(auto i=0; i<5; ++i)
	{
		autoref var_user = model.variable("Var Users", xavier_initial(1000, 5));
		autoref var_item = model.variable("Var Items", xavier_initial(2000, 5));

		model.loss("R", model(10.f) % var_user % var_user);
		model.loss("R", model(10.f) % var_item % var_item);

		autoref embed_user = sigmoid(var_user[triple_user]);
		autoref embed_item = sigmoid(var_item[triple_item]);
		
		pred.push_back(&(log(sum(embed_user % embed_item, 1))
			- sum((embed_user - embed_item) % (embed_user - embed_item), 1)));
	}

	autoref bias_user = model(1.f) + sigmoid(model.variable("Bias", xavier_initial(1000, 1))[triple_user]);
	autoref bias_item = model(1.f) + sigmoid(model.variable("Bias", xavier_initial(1000, 1))[triple_item]);

	autoref frac = 
		model(1.f) % exp(*pred[0] % bias_user % bias_item) 
		+ model(2.f) % exp(*pred[1] % bias_user % bias_item) 
		+ model(3.f) % exp(*pred[2] % bias_user % bias_item) 
		+ model(4.f) % exp(*pred[3] % bias_user % bias_item) 
		+ model(5.f) % exp(*pred[4] % bias_user % bias_item);
			
	autoref part = 
		exp(*pred[0] % bias_user % bias_item) 
		+ exp(*pred[1] % bias_user % bias_item) 
		+ exp(*pred[2] % bias_user % bias_item) 
		+ exp(*pred[3] % bias_user % bias_item) 
		+ exp(*pred[4] % bias_user % bias_item);
	
	autoref y = frac / part;

	return y;
}

int main(int, char* [])
{
	dmMovielens100K loader;
	loader.load();

	iGraph model;
	autoref triple_user = model.data_source("Users", loader.arr_rating(af::seq(0, 1000, 2) ,0));
	autoref triple_item = model.data_source("Items", loader.arr_rating(af::seq(0, 1000, 2) ,1));
	autoref triple_rate = model.data_source("Rates", loader.arr_rating(af::seq(0, 1000, 2), 2));

	autoref y_total = unit(model, triple_user, triple_item)
		+ unit(model, triple_user, triple_item)
		+ unit(model, triple_user, triple_item);
	
	autoref y = y_total / model(3.f);

	model.loss("Loss", (y - triple_rate) % (y - triple_rate));
	model.train(500);

	triple_user.set(loader.arr_rating(af::seq(1, 1000, 2), 0));
	triple_item.set(loader.arr_rating(af::seq(1, 1000, 2), 1));
	triple_rate.set(loader.arr_rating(af::seq(1, 1000, 2), 2));

	model.perform();

	logout.direct() << ::print_array(
		af::sum(af::abs(y.value_forward - triple_rate.value_forward))/500);


	return 0;
}
