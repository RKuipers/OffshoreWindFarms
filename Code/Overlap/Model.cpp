#include "Model.h"

double Model::solveBasics(clock_t start)
{
	// TODO: Expand

	p.setMsgLevel(0);
	if (start == 0)
		start = clock();
	p.mipOptimise();
	return ((double)clock() - start) / (double)CLOCKS_PER_SEC;
}

Model::Model(InputData* data) : data(data) { }
