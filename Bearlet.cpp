#include "Import.hpp"
#include "File.hpp"
#include "Config.hpp"
#include "Logging.hpp"

int main(int argc, char* argv[])
{
	try
	{
	logout.record() << bearlet_config.configs["Report"]["report_path"];
	logout.record() << "This is a file";
	logout.record() << 1;
	logout.record() << 1.5;

	logdb.env_push("Test Framework", 1.0);
	logdb.result_push("Result", 1.0);
	logdb.record();
	}
	catch(string& src)
	{
		cout<<src<<endl;
	}
	return 0;
}
