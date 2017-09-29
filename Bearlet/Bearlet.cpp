#include "Import.hpp"
#include "File.hpp"
#include "Logging.hpp"
#include "Dataset.hpp"
#include "Interactive.hpp"

int main(int, char**)
{
	// RemoteCommander cmd("127.0.0.1:6666", ios::out);
	// cmd.send("Run", 
			// [](FormatFile& file)
			// {
				// int a = 10;
				// string b = "Run Successly.";
				// file << a << b;
			// });

	RemoteCommander cmd("127.0.0.1:6666", ios::in);
	cmd.registe("Run",
			[](FormatFile& file)
			{
				int a;
				string b;
				file >> a >> b;
				logout.record() << a;
				logout.record() << b;
			});
	cmd.recieve();

	return 0;
}
