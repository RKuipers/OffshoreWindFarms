#include "Model.h"

double Model::solveBasics(int maxTime, bool verbose, clock_t start)
{
	// TODO: Expand

	if (!verbose)
		p.setMsgLevel(0);
	string name = p.getName();

	XPRBloadmat(p.getCRef());
	XPRSprob opt_prob = XPRBgetXPRSprob(p.getCRef());
	if (maxTime != 0)
		XPRSsetintcontrol(opt_prob, XPRS_MAXTIME, -maxTime);
	XPRSsetdblcontrol(opt_prob, XPRS_MIPRELSTOP, 0.01);
	//XPRStune(opt_prob, "g");

	p.exportProb(XPRB_LP, ("Output Files/" + name).c_str());
	if (start == 0)
		start = clock();
	p.mipOptimise();
	return ((double)clock() - start) / (double)CLOCKS_PER_SEC;
}

Model::Model(InputData* data, Mode* mode, string name) : p((name).c_str()), data(data), mode(mode) { }
