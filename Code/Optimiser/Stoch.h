#pragma once

#include "Optimiser.h"

// Model settings
#define PROBNAME "stoYearLR"
#define MAXPRETIME 28000
#define MAXFULLTIME 28000
#define NSCENARIOS 3
#define NPERIODS 12
#define TPP 4 // Timesteps per Period
#define NTIMES NPERIODS * TPP
#define NPTASKS 2
#define NCTASKS 3
#define NTASKS NPTASKS + NCTASKS
#define NRES 2
#define NASSETS 1
#define DIS 0.9991628
#define BASE 105
#define VARIETY 51

// Mode related settings
//#define LOCKMODE "SetFinFaiDowCuts FinAll TEST0" 
//#define LOCKDIM "SetCuts"		// Current best: SetFinFaiDowCuts, SetOrdResBroCuts
#define LOCKSET 1	// 1 Strong
#define LOCKORD 0	// 0 Strong
#define LOCKFIN	1	// 1 Strong
#define LOCKPRE 0	// 0 Strong
#define LOCKRES 1	// 1 Medium (test more)
#define LOCKACT 0	// 0 Strong
#define LOCKFAI 1	// 1 Strong
#define LOCKCOR	0	// 0 Medium-Strong
#define LOCKDOW	1	// 1 Medium (test more)
#define MODECUTS 6
#define MODEFIN 2
//#define MODETUNE 2
#define MODETEST 2

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
	void genFinishConstraints(XPRBprob* prob, bool cut, bool finAll);
	void genResourceConstraints(XPRBprob* prob, bool cut);
	void genFailureConstraints(XPRBprob* prob, bool cut);
	void genCorrectiveConstraints(XPRBprob* prob, bool cut);
	void genDowntimeConstraints(XPRBprob* prob, bool cut);
	void genPartialProblem(XPRBprob* prob, Mode* m) override;
	void genFullProblem(XPRBprob* prob, Mode* m) override;
};
