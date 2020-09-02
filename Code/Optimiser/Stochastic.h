#pragma once

#include "Optimiser.h"

// Model settings
#define PROBNAME "stoYearMR"
#define MAXPRETIME 800
#define MAXFULLTIME 800
#define NSCENARIOS 3
#define NPERIODS 12
#define TPP 15 // Timesteps per Period
#define NTIMES NPERIODS * TPP
#define NPTASKS 2
#define NCTASKS 3
#define NTASKS NPTASKS + NCTASKS
#define NRES 2
#define NASSETS 2
#define DIS 0.9991628
#define BASE 105
#define VARIETY 51

// Mode related settings
//#define LOCKMODE "FinFaiCuts" 
//#define LOCKDIM "SetCuts"		// Current best: FinResFaiDowCuts
//#define LOCKSET	0	// 1	Weak
//#define LOCKFIN	1	// 1?	Irrelevant
//#define LOCKRES	0	// 1	Med
//#define LOCKFAI	1	// 1?
//#define LOCKCOR	0	// 1	Med
//#define LOCKDOW	0	// 1	Strong 
#define MODECUTS 6
//#define MODETUNE 2
//#define MODETEST 2

class Stoch : public Optimiser
{
public:
	Stoch();

protected:
	vector<vector<int>> v;		// Values (Time, Scenario)

	Mode initMode();

	void readTasks(ifstream* datafile, int taskType, vector<int>* limits) override;
	void readValues(ifstream* datafile) override;
	void readLambdas(ifstream* datafile) override;
	void readData() override;

	void genDecisionVariables(XPRBprob* prob) override;
	void genObjective(XPRBprob* prob) override;
	void genSetConstraints(XPRBprob* prob, bool cut);
	void genFinishConstraints(XPRBprob* prob, bool cut);
	void genResourceConstraints(XPRBprob* prob, bool cut);
	void genFailureConstraints(XPRBprob* prob, bool cut);
	void genCorrectiveConstraints(XPRBprob* prob, bool cut);
	void genDowntimeConstraints(XPRBprob* prob, bool cut);
	void genPartialProblem(XPRBprob* prob, Mode* m) override;
	void genFullProblem(XPRBprob* prob, Mode* m) override;
};
