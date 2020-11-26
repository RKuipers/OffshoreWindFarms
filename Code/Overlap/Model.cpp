#include "Model.h"

double Model::solveBasics(clock_t start)
{
	// TODO: Expand

	p.setMsgLevel(1);
	if (start == 0)
		start = clock();
	p.mipOptimise();
	return ((double)clock() - start) / (double)CLOCKS_PER_SEC;
}

Model::Model(InputData* data, Mode* mode, string name) : p((name + to_string(mode->getCurrentId())).c_str()), data(data), mode(mode) { }
