#pragma once

#include "Optimiser.h"
#include <tuple>		// tuple

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
#define BASE 150
#define VARIETY 51
#define OPTIMAL 280 // The optimal solution, if known

class Deter : public Optimiser
{
protected:
	vector<int> v;
	vector<tuple<int, int>> IP;

	Deter();

	Mode initMode();

	void readData() override;
	void readTasks(ifstream* datafile, int taskType, vector<int>* limits) override;
	void readValues(ifstream* datafile) override;
	void readLambdas(ifstream* datafile) override;
	void readPreqs(ifstream* datafile);

	void genDecisionVariables(XPRBprob* prob) override;
	void genObjective(XPRBprob* prob) override;
	void genPartialProblem(XPRBprob* prob, Mode* m) override;
	void genFullProblem(XPRBprob* prob, Mode* m) override;
	void genSetConstraints(XPRBprob* prob, bool cut);
	void genOrderConstraints(XPRBprob* prob, bool cut);
	void genFinishConstraints(XPRBprob* prob, bool cut, bool finAll);
	void genPrecedenceConstraints(XPRBprob* prob, bool cut);
	void genResourceConstraints(XPRBprob* prob, bool cut);
	void genActiveConstraints(XPRBprob* prob, bool cut);
	void genFailureConstraints(XPRBprob* prob, bool cut);
	void genCorrectiveConstraints(XPRBprob* prob, bool cut);
	void genDowntimeConstraints(XPRBprob* prob, bool cut);
};
