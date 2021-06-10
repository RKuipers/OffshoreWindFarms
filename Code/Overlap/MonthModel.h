#pragma once

#include "Model.h"

class MonthModel :
	public Model
{
private:
	MonthSolution* solution;
	int id;

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
	MonthModel(MonthData* data, Mode* mode, string name = "Month", int id = 0);

	void getRequirements(vector<double>* eps, vector<int>* rho, int globalY);

	void genProblem() override;
	void genPartialProblem(int it);
	MonthSolution* solve(int maxTime = 0, bool verbose = true) override;
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
	FeedbackModel(MonthData* data, Mode* mode, int id = 0);

	void genProblem() override;
	vector<double> getEps(int globalY);
};

