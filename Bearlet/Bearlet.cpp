#include "Import.hpp"
#include "File.hpp"
#include "Logging.hpp"
#include "Dataset.hpp"
#include "Interactive.hpp"

int main(int, char**)
{
	af::Window w;
	do
	{
		w.show();
	}while(!w.close());
	return 0;
}
