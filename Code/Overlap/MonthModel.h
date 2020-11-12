#pragma once
#include "Model.h"

class MonthModel :
	public Model
{
protected:
	Solution* genSolution(XPRBprob* p) override;

	void genProblem() override;
	void genDecVars() override;
	void genObj() override;
	void genLimitCon();
	void genOrderCon();
	void genResourceCon();
	void genDurationCon();
	void genFinishCon();
	void genFixedACon();
	void genFixedSCon();
};

