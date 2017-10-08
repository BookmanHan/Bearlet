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

Symbol& unit(iGraph& model, Symbol& triple_user, Symbol& triple_item, Symbol& triple_rate)
{
	autoref var_user = model.variable("Var Users", xavier_initial(30000, 5));
	autoref var_item = model.variable("Var Items", xavier_initial(10000, 5));
	autoref var_rate_u = model.variable("Var Rate", xavier_initial(10, 5));
	autoref var_rate_i = model.variable("Var Rate", xavier_initial(10, 5));

	autoref embed_user = sigmoid(var_user[triple_user] % var_rate_u[triple_rate]);
	autoref embed_item = sigmoid(var_item[triple_item] % var_rate_i[triple_rate]);

	model.loss("Sum R", model(10.f) % abs(embed_user));
	model.loss("Sum R", model(10.f) % abs(embed_item));
	
	autoref guass = embed_user - embed_item;
	autoref pred = sum(embed_user % embed_item, 1) % exp(-sum(abs(guass), 1));

	model.loss("likelihood", pred);

	return pred;
}

af::array test(
		iGraph& model,
		Symbol& triple_user,
		Symbol& triple_item,
		Symbol& triple_rate,
		Symbol& y,
		af::array& arr_rating, 
		int batch_size, 
		int batch_num_train)
{
	af::array mse = af::constant(0.f, 2);
	int n_base = batch_size * batch_num_train;
	int batch_num_test = arr_rating.dims(0) * 0.2 / batch_size;
	for(auto ibatch = 0; ibatch < batch_num_test; ++ibatch)
	{
		af::array batch_seq = af::seq(n_base + ibatch * batch_size, n_base + (ibatch + 1) * batch_size, 1);
		triple_user.set(arr_rating(batch_seq, 0));
		triple_item.set(arr_rating(batch_seq, 1));
		triple_rate.set(arr_rating(batch_seq, 2));

		model.perform();

		mse += af::join(0,
				af::sum(af::abs(y.value_forward - triple_rate.value_forward)),
				af::sum((y.value_forward - triple_rate.value_forward) 
						* (y.value_forward - triple_rate.value_forward)));

	}

	mse(0) /= batch_num_test * batch_size;
	mse(1) = af::sqrt(mse(1)/(1.f * batch_num_test * batch_size));

	return mse;
}

int main(int, char* argv[])
{
	af::info();

	dmMovielens1M loader;
	loader.load();
	
	int batch_size = 1024 * atoi(argv[2]);
	int batch_num_train = loader.arr_rating.dims(0) * 0.8 / batch_size;iGraph model;

	autoref triple_user = model.data_source("Users", loader.arr_rating(af::seq(0, 1000, 2) ,0));
	autoref triple_item = model.data_source("Items", loader.arr_rating(af::seq(0, 1000, 2) ,1));
	autoref triple_rate = model.data_source("Rates", loader.arr_rating(af::seq(0, 1000, 2), 2));
	autoref rate_1 = model.data_source("R.1", af::constant(1.f, batch_size + 1));
	autoref rate_5 = model.data_source("R.2", af::constant(5.f, batch_size + 1));

	//autoref bias_user = model(1.f) + sigmoid(model.variable("Bias", xavier_initial(30000, 1))[triple_user]);
	//autoref bias_item = model(1.f) + sigmoid(model.variable("Bias", xavier_initial(10000, 1))[triple_item]);

	Symbol* unit_1 = &unit(model, triple_user, triple_item, rate_1);
	for(auto i=0; i<9; ++i)
		unit_1 = &(*unit_1 + unit(model, triple_user, triple_item, rate_1));
	
	Symbol* unit_5 = &unit(model, triple_user, triple_item, rate_5);
	for(auto i=0; i<9; ++i)
		unit_5 = &(*unit_5 + unit(model, triple_user, triple_item, rate_5));

	autoref frac = 
		model(1.f) % exp(*unit_1) 
		+ model(5.f) % exp(*unit_5);
			
	autoref part = 
		exp(*unit_1) 
		+ exp(*unit_5);
	
	autoref y = frac / part;

	model.loss("Loss", (y - triple_rate) % (y - triple_rate));
	
	model.train(atoi(argv[1]) * batch_num_train,
			[&](iGraph, int epos_id)
			{
				int batch_id = epos_id % batch_num_train;
				
				if (batch_id == 1)
				{
					auto metric  = ::test(model, triple_user, triple_item, triple_rate, y,	
							loader.arr_rating, batch_size, batch_num_train);
				
					logout.record();
					logout << "MSE = " << metric(0, 0).scalar<float>();
					logout << ", RMSE = " << metric(1, 0).scalar<float>();
				}
	
				af::array batch_seq = af::seq(batch_id * batch_size, (batch_id + 1) * batch_size, 1);
				triple_user.set(loader.arr_rating(batch_seq, 0));
				triple_item.set(loader.arr_rating(batch_seq, 1));
				triple_rate.set(loader.arr_rating(batch_seq, 2));
			});

	::test(model, triple_user, triple_item, triple_rate, y,	
							loader.arr_rating, batch_size, batch_num_train);
	return 0;
}
