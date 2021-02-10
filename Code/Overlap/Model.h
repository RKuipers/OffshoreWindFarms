#pragma once

#include <vector>
#include <ctime>
#include <algorithm>    // std::max

#include "Mode.h"
#include "InputData.h"
#include "Solution.h"
#include "Weather.h"
#include "xprb_cpp.h"
#include "xprs.h"

using namespace ::dashoptimization;

class Model
{
protected:
	XPRBprob p;
	InputData* data;
	Weather weather;
	Mode* mode;
	Solution* solution;

	virtual Solution* genSolution(XPRBprob* p, double duration) =0;
	double solveBasics(int maxTime = 0, clock_t start = 0);

	virtual void genDecVars() = 0;
	virtual void genObj() = 0;
	
public:
	Model(InputData* data, Mode* mode, string name);

	virtual void genProblem() = 0;
	virtual Solution* solve(int maxTime = 0) =0;
};

