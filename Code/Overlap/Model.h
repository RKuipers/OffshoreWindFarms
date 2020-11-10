#pragma once
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
	
public:
	Solution solve();
	virtual void genProblem();
	virtual void genDecVars();
	virtual void genObj();
};

