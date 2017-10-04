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

int main(int, char* [])
{
	af::info();
	af::setDevice(0);

	dmMovielens100K loader;
	loader.load();

	iGraph model;
	autoref triple_user = model.data_source("Users", loader.arr_rating(af::seq(0, 10000, 2) ,0));
	autoref triple_item = model.data_source("Items", loader.arr_rating(af::seq(0, 10000, 2) ,1));
	autoref triple_rate = model.data_source("Rates", loader.arr_rating(af::seq(0, 10000, 2), 2));

	autoref rep_user = model.variable("Rep Users", xavier_initial(10000, 10));
	autoref rep_item = model.variable("Rep Items", xavier_initial(20000, 10));

	autoref embed_user = rep_user[triple_user];
	autoref embed_item = rep_item[triple_item];

	autoref pred = sum(embed_user %	embed_item, 1);
	
	model.loss("Loss", (pred - triple_rate) % (pred - triple_rate));
	model.train(100);

	triple_user.set(loader.arr_rating(af::seq(1, 10000, 2), 0));
	triple_item.set(loader.arr_rating(af::seq(1, 10000, 2), 1));
	triple_rate.set(loader.arr_rating(af::seq(1, 10000, 2), 2));

	model.perform();

	logout.direct() << ::print_array(
		af::sum(af::abs(pred.value_forward - triple_rate.value_forward)/10000));


	return 0;
}
