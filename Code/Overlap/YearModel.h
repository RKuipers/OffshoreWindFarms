#pragma once
#include "Model.h"

class YearModel :
	public Model
{
private:
	YearSolution* solution;

	YearData* getData();

protected:
	vector<vector<vector<XPRBvar>>> N;	// y m sig
	vector <vector<XPRBvar>> P;			// m i
	vector<vector<vector<XPRBvar>>> R;	// m i sig

	YearSolution* genSolution(XPRBprob* p, double duration) override;

	void genProblem() override;
	void genDecVars() override;
	void genObj() override;
	void genCapacityCon();
	void genRepairCon();
	void genMaxMaintCon();
	void genMinMaintCon();
	void genAvailableCon();

public:
	YearModel(YearData* data, Mode* mode);

	YearSolution* solve() override;
};

