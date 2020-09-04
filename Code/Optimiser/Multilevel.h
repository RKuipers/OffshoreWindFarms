#pragma once

#include <ctime>		// clock
#include <fstream>		// ifstream, ofstream
#include <string>		// string, to_string
#include <vector>		// vector
#include <iostream>		// cout
#include "xprb_cpp.h"
#include "xprs.h"

// Program settings
#define SEED 42 * NTIMES
#define WEATHERTYPE 1
#define VERBOSITY 4		// The one to edit
#define VERBMODE 1
#define VERBSOL 2
#define VERBINIT 3
#define VERBPROG 4
#define VERBWEAT 5
#define INPUTFOLDER "Input files/"
#define OUTPUTFOLDER "Output files/"
#define PROBOUTPUTEXT ".sol"
#define MODEOUTPUTEXT ".csv"
#define DATAEXT ".dat"

using namespace std;
using namespace ::dashoptimization;

class MultiLevel
{
public:
	MultiLevel();

	void Run();

protected:
	// Program data
	string name;

	// Decision Variables
	vector<XPRBvar> P, R;		// Planned and repaired tasks (Month)
	vector<vector<XPRBvar>> N;	// Vessels needed (Vessel, Month)

	// Parameters
	vector<vector<double>> C;	// Charter costs (Vessel, Month)
	vector<double> eh, em;		// Energy generated in an hour/month (Month)
	vector<int> dpv, drv;		// Duration that a planned/repair task needs a vessel in hours (Vessel)
	int dp;						// Duration of a planned task in hours
	vector<int> f;				// Failures (Month)
	int A;						// Number of assets that need to be serviced with planned tasks
	vector<int> l;				// Amount of hours a vessel is available when chartered for a month (Vessel)
	int V, M;					// Amount of vessels/months

	void getData();

	void genDecisionVariables(XPRBprob* prob);
	void genObjective(XPRBprob* prob);
	void genCapacityConstraints(XPRBprob* prob);
	void genFailuresConstraints(XPRBprob* prob);
	void genPlannedConstraints(XPRBprob* prob);
	void genProblem(XPRBprob* prob);

	void printObj(ofstream* file, XPRBprob*);
	void printResources(ofstream* file);
	void printTasks(ofstream* file);
	void printProbOutput(XPRBprob* prob);
	int printer(string s, int verbosity, bool end = true, int maxVerb = 999);
};