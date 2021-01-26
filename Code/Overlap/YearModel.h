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
	vector<vector<vector<XPRBvar>>> FU;	// m i sig
	vector<vector<vector<XPRBvar>>> R;	// m i sig

	YearData* getData();
	YearSolution* genSolution(XPRBprob* p, double duration) override;

	void genDecVars() override;
	void genObj() override;
	void genCapacityCon();
	void genResourceCon();
	void genRepairCon();
	void genRegMaintCon();

public:
	YearModel(YearData* data, Mode* mode);

	void genProblem() override;
	YearSolution* solve() override;

	double printMixedValue(vector<MonthSolution*> months);
};

