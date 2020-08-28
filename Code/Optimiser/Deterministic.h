#pragma once

#include "Optimiser.h"

// Model settings
#define PROBNAME "lifeSimple"
#define MAXPRETIME 10
#define MAXFULLTIME 10
#define NPERIODS 7
#define TPP 4 // Timesteps per Period
#define NTIMES NPERIODS * TPP
#define NITASKS 2
#define NMPTASKS 1
#define NMCTASKS 3
#define NDTASKS 2
#define NMTASKS NMPTASKS + NMCTASKS
#define NTASKS NITASKS + NMTASKS + NDTASKS
#define NIP 2
#define NRES 2
#define NASSETS 2
#define DIS 1.0
#define BASE 105
#define VARIETY 51
#define OPTIMAL 280 // The optimal solution, if known

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
#define MODECUTS 9
#define MODEFIN 2
//#define MODETUNE 2
#define MODETEST 2

class Deter : public Optimiser
{
public:
	Deter();

protected:
	vector<int> v;					// Values (Time)
	vector<tuple<int, int>> IP;		// Precedences (Task tuples)

	Mode initMode();

	void readTasks(ifstream* datafile, int taskType, vector<int>* limits) override;
	void readValues(ifstream* datafile) override;
	void readLambdas(ifstream* datafile) override;
	void readPreqs(ifstream* datafile);
	void readData() override;

	void genDecisionVariables(XPRBprob* prob) override;
	void genObjective(XPRBprob* prob) override;
	void genSetConstraints(XPRBprob* prob, bool cut);
	void genOrderConstraints(XPRBprob* prob, bool cut);
	void genFinishConstraints(XPRBprob* prob, bool cut, bool finAll);
	void genPrecedenceConstraints(XPRBprob* prob, bool cut);
	void genResourceConstraints(XPRBprob* prob, bool cut);
	void genActiveConstraints(XPRBprob* prob, bool cut);
	void genFailureConstraints(XPRBprob* prob, bool cut);
	void genCorrectiveConstraints(XPRBprob* prob, bool cut);
	void genDowntimeConstraints(XPRBprob* prob, bool cut);
	void genPartialProblem(XPRBprob* prob, Mode* m) override;
	void genFullProblem(XPRBprob* prob, Mode* m) override;
};
