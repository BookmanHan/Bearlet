#include "Import.hpp"
#include "File.hpp"
#include "Config.hpp"
#include "Logging.hpp"
#include "FormatLoader.hpp"
#include "Utils.hpp"

int main(int argc, char** argv)
{
	af::array arr;

	FormatLoaderUnalignedSeperate loader("/home/bookman/Data/a.txt", 20);
	loader.to_array(arr,
			[&](int nline, int nfield, const string & elem)
			{
				return bearlet_cast<float>(elem);
			});
	logout.record() << "End.";

	af_print(arr);

	return 0;
}
