#pragma once
#include "Model.h"

class YearModel :
	public Model
{
private:
	YearSolution* solution;

protected:
	vector<vector<vector<XPRBvar>>> N;	// y m sig
	vector <vector<XPRBvar>> P;			// m i
	vector<vector<vector<XPRBvar>>> R;	// m i sig
	vector<vector<vector<XPRBvar>>> U;	// m i sig
	vector<vector<vector<XPRBvar>>> Mp;	// y m sig
	vector<vector<vector<vector<XPRBvar>>>> Mr;	// y m sig ir

	YearData* getData();
	YearSolution* genSolution(XPRBprob* p, double duration) override;

	void genDecVars() override;
	void genObj() override;
	void genCapacityCon();
	void genResourceCon();
	void genRhoCon();
	void genRepairCon();
	void genRegMaintCon();

public:
	YearModel(YearData* data, Mode* mode);

	void genProblem() override;
	YearSolution* solve(int maxTime = 0) override;

	double printMixedValue(vector<MonthSolution*> months);
};

