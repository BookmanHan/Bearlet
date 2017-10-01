//#define iGraph_DEBUG
#include "Import.hpp"
#include "Logging.hpp"
#include "Dataset.hpp"
#include "iGraph.hpp"
#include "Operator.hpp"

// To Design Batch Mode.
// +, -, %, *, log, exp, /
// 1.大的张量不归一化直接用图，会产生 NAN 错误。
//
Symbol& ce(iGraph& g, Symbol& y, Symbol& l)
{
	return -(y % log(g.data_const(0.01f) + l) + (g.data_const(1.01f) - y) % log(g.data_const(1.01f) - l));
}

int main(int, char* argv[])
{
	dmMNIST ldMNIST;
	ldMNIST.load();
	int size_feature = ldMNIST.arr_train_data.dims(1);

	iGraph model;
		
	autoref label = model.data_source("label", label_vectorization(ldMNIST.arr_train_label, 10));
	autoref data = model.data_source("data", ldMNIST.arr_train_data);

	autoref W1 = model.variable("Logic Regression Weight", xavier_initial(size_feature, 100));
	autoref W2 = model.variable("Hidden Layer 1", xavier_initial(100, 10));
	autoref y = sigmoid(sigmoid(data * W1) *  W2);

	model.loss("Prediction", (y - label) % (y - label)); 
	model.loss("Regularization W1", model.data_const(atof(argv[2])) % W1 % W1);
	model.loss("Regularization W2", model.data_const(atof(argv[2])) % W2 % W2);

	model.train(atoi(argv[1]));
	
	label.set(label_vectorization(ldMNIST.arr_test_label, 10));
	data.set(ldMNIST.arr_test_data);
	
	model.perform();
	model.report();

	af::array idx_pred;
	af::array val_pred;
	af::max(val_pred, idx_pred, y.value_forward, 1);
	
	af::array acc = sum(idx_pred == ldMNIST.arr_test_label);
	logout.record() << ::print_array(acc/(float)ldMNIST.arr_test_label.dims(0));


	return 0;
}
