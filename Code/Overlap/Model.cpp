#include "Model.h"

double Model::solveBasics(clock_t start)
{
	// TODO: Expand

	if (data->getMonth() == nullptr || data->getMonth()->I <= 3)
		p.setMsgLevel(0);
	string name = p.getName();
	p.exportProb(XPRB_LP, ("Output Files/" + name).c_str());
	if (start == 0)
		start = clock();
	p.mipOptimise();
	return ((double)clock() - start) / (double)CLOCKS_PER_SEC;
}

Model::Model(InputData* data, Mode* mode, string name) : p((name + to_string(mode->getCurrentId())).c_str()), data(data), mode(mode) { }
