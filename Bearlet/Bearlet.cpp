#include "Import.hpp"
#include "File.hpp"
#include "Logging.hpp"
#include "Dataset.hpp"

int main(int, char**)
{
	dmMNIST loader;
	loader.load();

	// bearlet_write("127.0.0.1:6666",
			// [&](FormatFile& file)
			// {
				// file << loader.arr_train_data; 
				// af_print(loader.arr_train_data(af::end, af::span));
			// }, Storage::network_system);

	bearlet_read("127.0.0.1:6666",
			[&](FormatFile& file)
			{
				af::array arr;
				file >> arr; 
				af::array check = (arr == loader.arr_train_data);
				af::array res =  af::sum(af::sum(check, 1), 0);
				logout.record() << "Check = " << print_array(res);
				logout.record() << "Should = " << arr.dims(0) * arr.dims(1);
			}, Storage::network_system);

	return 0;
}
