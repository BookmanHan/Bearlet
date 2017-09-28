#include "Import.hpp"
#include "File.hpp"
#include "Logging.hpp"
#include "Dataset.hpp"

int main(int argc, char** argv)
{
	dmMNIST ld_MNIST;
	ld_MNIST.save();

	dmWeiboPair ld_wp;
	ld_wp.save();

	dmAbalone ld_aba;
	ld_aba.save();

	dmCIFAR10 ld_c10;
	ld_c10.save();

	dmCIFAR100 ld_c100;
	ld_c100.save();

	dmSimpleQuestions ld_sq;
	ld_sq.save();

	dmMovielens100K ld_ma;
	ld_ma.save();

	dmMovielens1M ld_mb;
	ld_mb.save();

	dmMovielens10M ld_mc;
	ld_mc.save();

	dmMovielens20M ld_md;
	ld_md.save();

	return 0;
}
