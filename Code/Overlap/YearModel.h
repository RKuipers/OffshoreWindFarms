#pragma once
#include "Model.h"

class YearModel :
	public Model
{
protected:
	void genProblem() override;
	void genDecVars() override;
	void genObj() override;
	void genCapacityCon();
	void genRepairCon();
	void genMaxMaintCon();
	void genMinMaintCon();
	void genAvailableCon();
};

