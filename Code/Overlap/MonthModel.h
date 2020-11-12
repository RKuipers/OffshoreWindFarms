#pragma once

#include "Model.h"

class MonthModel :
	public Model
{
private:
	MonthData* getData();

public: // TODO: protected
	vector<XPRBvar> s;					// i
	vector<vector<vector<XPRBvar>>> a;	// v i j

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

//public: TODO
	MonthModel(MonthData* data);
};

