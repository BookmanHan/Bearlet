#include "Import.hpp"
#include "File.hpp"
#include "Config.hpp"
#include "Logging.hpp"
#include "FormatLoader.hpp"
#include "Utils.hpp"

int main(int argc, char** argv)
{
	af::array arr;

	FormatLoaderAlignedSeperate loader("/home/bookman/Experiment/Bearlet/Dataset/mnist/mnist_train.csv", ",");
	loader.to_array(arr,
			[&](int nline, int nfield, const string & elem)
			{
				return bearlet_cast<float>(elem);
			});
	logout.record() << "End.";

	af_print(arr(-1,af::span));

	return 0;
}
