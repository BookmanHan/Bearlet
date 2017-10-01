//#define iGraph_DEBUG
#include "Import.hpp"
#include "Logging.hpp"
#include "Dataset.hpp"
#include "iGraph.hpp"
#include "Operator.hpp"

// To Design Batch Mode.
// +, -, %, *, log, exp, /
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
	autoref W = model.variable("Hidden Layer", xavier_initial(n_input, n_output));
	return sigm(abs(x) * W);
}

int main(int, char* argv[])
{
	dmCIFAR10 loader;
	loader.load();

	int size_feature = loader.arr_train_data.dims(1);

	iGraph model;
		
	autoref label = model.data_source("label", label_vectorization(loader.arr_train_label, 10));
	autoref data = model.data_source("data", loader.arr_train_data/255.f);

	autoref hid_1 = sigmoid_layer(data, size_feature, 300);
	autoref t = sigmoid_layer(hid_1, 300, 500);
	autoref y = sigmoid_layer(t, 500, 10);

	model.loss("Prediction", (y - label) % (y - label)); 
	model.loss("Diversity", abs(t));
	model.loss("Diversity", abs(hid_1));

	model.train(atoi(argv[1]));
		
	label.set(label_vectorization(loader.arr_test_label, 10));
	data.set(loader.arr_test_data/255.f);
	
	model.perform();
	model.report();

	af::array idx_pred;
	af::array val_pred;
	af::max(val_pred, idx_pred, y.value_forward, 1);
	
	af::array acc = sum(idx_pred == loader.arr_test_label);
	logout.record() << ::print_array(acc/(float)loader.arr_test_label.dims(0));


	return 0;
}
