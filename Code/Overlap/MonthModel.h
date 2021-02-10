#pragma once

#include "Model.h"

class MonthModel :
	public Model
{
private:
	MonthSolution* solution;

protected:
	vector<vector<XPRBvar>> s;			// v j
	vector<vector<vector<XPRBvar>>> a;	// v i j
	vector<XPRBvar> f;					// i

	MonthData* getData();
	MonthSolution* genSolution(XPRBprob* p, double duration) override;

	void genDecVars() override;
	void genObj() override;
	void genOrderCon();
	void genLimitCon();
	void genResourceCon();
	void genDurationCon();
	void genReleaseCon();
	virtual void genDeadlineCon();
	void genFixedCons();
	void genFinishCon();
	void genPrecedenceCon();

public: 
	MonthModel(MonthData* data, Mode* mode, string name = "Month");

	void getRequirements(vector<double>* eps, vector<int>* rho);

	void genProblem() override;
	MonthSolution* solve(int maxTime = 0) override;
};

class FeedbackModel :
	public MonthModel
{
protected:
	vector<XPRBvar> Ty;
	XPRBvar T;

	void genDecVars() override;
	void genObj() override;
	void genDeadlineCon() override;
	void genMaxFinCon();

public:
	FeedbackModel(MonthData* data, Mode* mode);

	void genProblem() override;
	vector<double> getEps();
};

