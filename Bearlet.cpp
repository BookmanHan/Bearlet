#include "Import.hpp"
#include "File.hpp"
#include "Config.hpp"
#include "Logging.hpp"

int main(int argc, char** argv)
{
	try
	{
		logout.record() << bearlet_config.configs["Report"]["report_path"];
		logout.record() << "This is a file";
		logout.record() << 1;
		logout.record() << 1.5;
		logout.flush();

		logdb.push_env("Test Framework", 1.0);
		logdb.push_result("Recall", 1.0);
		logdb.record();
		logout.record() << "End.";
		logdb.restore();
	}
	catch(string& str)
	{
		cout << str << endl;
	}

	return 0;
}
