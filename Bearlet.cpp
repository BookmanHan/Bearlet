#include "Import.hpp"
#include "File.hpp"
#include "Config.hpp"
#include "Logging.hpp"
#include "FormatLoader.hpp"
#include "Utils.hpp"

int main(int argc, char** argv)
{
	FormatLoaderCSV loader("/home/bookman/Data/Classicial/abalone/abalone.data", ",");
	
	af::array arr;
	loader.to_array(arr, 
		[](int nline, int nfield, const string str)
		{
			if (nfield == 0)
			{
				if (str == "M") return (float)0.;
				else if (str == "F") return (float)1.;
				else return (float)2.;
			}
			
			return bearlet_cast<float>(str);
		}, false);

	bearlet_write("a.bldata",
			[&](FormatFile& file)
			{
				file << arr;
			});

	af::array ard;

	bearlet_read("a.bldata",
			[&](FormatFile& file)
			{
				file >> ard;
			});

	af_print(ard);

	return 0;
}
