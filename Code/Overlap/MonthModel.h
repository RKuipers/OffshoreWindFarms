#pragma once

#include "Model.h"

class MonthModel :
	public Model
{
private:
	MonthSolution* solution;

protected:
	vector<XPRBvar> s;					// i
	vector<vector<vector<XPRBvar>>> a;	// v i i'
	vector<vector<XPRBvar>> aF, aL;		// v i

	MonthData* getData();
	MonthSolution* genSolution(XPRBprob* p, double duration) override;

	void genProblem() override;
	void genDecVars() override;
	void genObj() override;
	void genLimitCon();
	void genOrderCon();
	void genResourceCon();
	void genDurationCon();
	virtual void genFinishCon();
	void genFixedCon();

public: 
	MonthModel(MonthData* data, Mode* mode, string name = "Month");

	void getRequirements(vector<double>* eps, vector<int>* rho);

	MonthSolution* solve() override;
};

class FeedbackModel :
	public MonthModel
{
protected:
	vector<XPRBvar> Ty;
	XPRBvar T;

	void genProblem() override;
	void genDecVars() override;
	void genObj() override;
	void genFinishCon() override;
	void genMaxFinCon();

public:
	FeedbackModel(MonthData* data, Mode* mode);

	vector<double> getEps();
};

