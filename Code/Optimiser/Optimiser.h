#pragma once

#include "Mode.cpp"
#include "xprb_cpp.h"
#include "xprs.h"
#include <ctime>		// clock

// Program settings
#define SEED 42 * NTIMES
#define WEATHERTYPE 1
#define VERBOSITY 1		// The one to edit
#define VERBMODE 1
#define VERBSOL 2
#define VERBINIT 3
#define VERBPROG 4
#define VERBWEAT 5
#define NAMES 1
#define INPUTFOLDER "Input files/"
#define OUTPUTFOLDER "Output files/"
#define PROBOUTPUTEXT ".sol"
#define MODEOUTPUTEXT ".csv"
#define DATAEXT ".dat"

using namespace std;
using namespace ::dashoptimization;

class Optimiser
{
private:
	virtual Mode initMode() = 0;

	virtual void readData() = 0;

	virtual void genDecisionVariables(XPRBprob* prob) = 0;
	virtual void genObjective(XPRBprob* prob) = 0;
	void genBasicProblem(XPRBprob* prob, Mode* m);
	virtual void genPartialProblem(XPRBprob* prob, Mode* m) = 0;
	virtual void genFullProblem(XPRBprob* prob, Mode* m) = 0;

	virtual void printProbOutput(XPRBprob* prob, Mode* m, int id) = 0;
	virtual void printModeOutput(Mode* m, bool opt) = 0;
	int printer(string s, int verbosity, bool end, int maxVerb);

	void solveProblem(XPRBprob* prob, bool tune, string name, int maxTime);

public:
	void Run();
};