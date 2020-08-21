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
#define OPTIMAL 280 // The optimal solution, if known

class Deter : public Optimiser
{
protected:
	vector<int> v;
	vector<tuple<int, int>> IP;

	Mode initMode();

	void readData();
	void readTasks(ifstream* datafile, int taskType);
	void readValues(ifstream* datafile;
	void readLambdas(ifstream* datafile); 
	void readPreqs(ifstream* datafile);

	void genDecisionVariables(XPRBprob* prob);
	void genObjective(XPRBprob* prob);
	void genPartialProblem(XPRBprob* prob, Mode* m); 
	void genFullProblem(XPRBprob* prob, Mode* m);

	void printProbOutput(XPRBprob* prob, Mode* m, int id);
	void printModeOutput(Mode* m, bool opt);
};
