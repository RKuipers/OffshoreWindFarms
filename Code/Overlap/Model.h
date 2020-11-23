#pragma once

#include <vector>
#include <ctime>

#include "Mode.h"
#include "InputData.h"
#include "Solution.h"
#include "Weather.h"
#include "xprb_cpp.h"
#include "xprs.h"

using namespace ::dashoptimization;

class Model
{
public: // TODO: protected
	XPRBprob p;
	InputData* data;
	Weather weather;
	Mode* mode;
	Solution* solution;

	virtual Solution* genSolution(XPRBprob* p, double duration) =0;
	
// public: TODO
	Model(InputData* data);

	virtual Solution* solve();
	virtual void genProblem();
	virtual void genDecVars();
	virtual void genObj();
};

