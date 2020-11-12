#pragma once
#include "Model.h"

class YearModel :
	public Model
{
private:
	YearData* getData();

public: // TODO: protected
	Solution* genSolution(XPRBprob* p) override;

	void genProblem() override;
	void genDecVars() override;
	void genObj() override;
	void genCapacityCon();
	void genRepairCon();
	void genMaxMaintCon();
	void genMinMaintCon();
	void genAvailableCon();
};

