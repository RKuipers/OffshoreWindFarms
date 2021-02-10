#include "Model.h"

double Model::solveBasics(int maxTime, clock_t start)
{
	// TODO: Expand

	if (data->getMonth() == nullptr || data->getMonth()->J <= 15)
		p.setMsgLevel(0);
	string name = p.getName();

	if (maxTime != 0)
	{
		XPRBloadmat(p.getCRef());
		XPRSprob opt_prob = XPRBgetXPRSprob(p.getCRef());
		XPRSsetintcontrol(opt_prob, XPRS_MAXTIME, -maxTime);
	}

	p.exportProb(XPRB_LP, ("Output Files/" + name).c_str());
	if (start == 0)
		start = clock();
	p.mipOptimise();
	return ((double)clock() - start) / (double)CLOCKS_PER_SEC;
}

Model::Model(InputData* data, Mode* mode, string name) : p((name + to_string(mode->getCurrentId())).c_str()), data(data), mode(mode) { }
