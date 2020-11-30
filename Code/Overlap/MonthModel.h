#pragma once

#include "Model.h"

class MonthModel :
	public Model
{
private:
	MonthSolution* solution;

	MonthData* getData();

public: // TODO: protected
	vector<XPRBvar> s;					// i
	vector<vector<vector<XPRBvar>>> a;	// v i j

	MonthSolution* genSolution(XPRBprob* p, double duration) override;

	void genProblem() override;
	void genDecVars() override;
	void genObj() override;
	void genLimitCon();
	void genOrderCon();
	void genResourceCon();
	void genDurationCon();
	void genFinishCon();
	void genFixedCon();

//public: TODO
	MonthModel(MonthData* data, Mode* mode);

	MonthSolution* solve() override;
};

