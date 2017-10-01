#include "Import.hpp"
#include "Logging.hpp"
#include "Dataset.hpp"
#include "iGraph.hpp"
#include "Operator.hpp"

// To Design Batch Mode.
int main(int, char* argv[])
{
	dmMNIST ldMNIST;
	ldMNIST.load();
	int size_feature = ldMNIST.arr_train_data.dims(1);

	iGraph model;
		
	autoref label = model.data_source("label", label_vectorization(ldMNIST.arr_train_label, 10));
	autoref data = model.data_source("data", ldMNIST.arr_train_data);

	autoref W = model.variable("Logic Regression Weight", xavier_initial(size_feature, 10));
	autoref y = sigmoid(data * W);

	model.loss("Prediction", model.data_const(-1.f) 
			% (y % log(label + model.data_const(0.01))+ (model.data_const(1.01f) - y) % log(model.data_const(1.01f) - label)));
	model.loss("Regularization", model.data_const(atof(argv[2])) % W % W);

	model.train(atoi(argv[1]));
	
	label.set(label_vectorization(ldMNIST.arr_test_label, 10));
	data.set(ldMNIST.arr_test_data);
	
	model.perform();
	model.report();

	af::array idx_pred;
	af::array val_pred;
	af::max(val_pred, idx_pred, y.value_forward, 1);
	
	af::array acc = sum(idx_pred == ldMNIST.arr_test_label);
	af_print(acc/(float)ldMNIST.arr_test_label.dims(0));


	return 0;
}
