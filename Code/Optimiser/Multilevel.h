#pragma once

#include <ctime>		// clock
#include <fstream>		// ifstream, ofstream
#include <string>		// string, to_string
#include <vector>		// vector
#include <iostream>		// cout
#include <numeric>      // iota
#include <algorithm>    // random_shuffle
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

	void Run(bool top);

protected:
	// Program data
	string name;

	// Decision Variables
	vector<XPRBvar> P;					// Planned tasks (Month)
	vector<vector<XPRBvar>> R;			// Repair tasks (Month, Scenario)
	vector<vector<vector<XPRBvar>>> N;	// Vessels needed (Vessel, Month, Scenario)
	vector<XPRBvar> gam;				// The buffer capacity (Scenario)
	vector<vector<vector<XPRBvar>>> ep;	// Binary variable wether any vessels are chartered that month (Vessel, Month, Scenario)
	vector<XPRBvar> s;					// Starting times (Task)
	vector<vector<vector<XPRBvar>>> a;	// Task assignments (Vessel, Task, Order)

	// Parameters of top level
	vector<vector<double>> C;	// Charter costs (Vessel, Month)
	vector<double> eh, em;		// Energy generated in an hour/month (Month)
	double u;					// Value of a buffer unit
	vector<int> dpy, dry;		// Duration that a planned/repair task needs a vessel in hours (Vessel)
	int dp;						// Duration of a planned task in hours
	vector<vector<int>> f;		// Failures (Month, Scenario)
	int A;						// Number of assets that need to be serviced with planned tasks
	vector<int> l;				// Amount of hours a vessel is available when chartered for a month (Vessel)
	int Y, M, S;				// Amount of vessel-types/months/scenarios
	int LARGE;					// A large number

	// Paramters of bottom level
	vector<double> c;				// The cost of a task being uncompleted for a single timestep (Task)
	vector<vector<double>> sd, d;	// The start delay and duration of work (Vessel Type, Task)
	vector<vector<int>> rho;		// The required number of vessels of a type that are used by a task (Vessel Type, Task)
	vector<int> Vy;					// The amount of vessels per type (Vessel Type)
	int V, I, J, T;					// Amount of vessels/tasks/tasks-per-vessel/times

	// Auxillerary metrics
	vector<double> minDur;			// Maximum duration based on summing maximum sd and d over all vessel types (Task)
	vector<int> Vyo;				// The offset between indices in V and Vy (Vessel Type)

	void genScenario(int id, double expected = 25.0);
	void getData();

	void genTopDecisionVariables(XPRBprob* prob);
	void genTopObjective(XPRBprob* prob);
	void genCapacityConstraints(XPRBprob* prob);
	void genCharterConstraints(XPRBprob* prob);
	void genFailuresConstraints(XPRBprob* prob);
	void genPlannedConstraints(XPRBprob* prob);
	void genTopProblem(XPRBprob* prob);

	void genLowDecisionVariables(XPRBprob* prob);
	void genLowObjective(XPRBprob* prob);
	void genSetConstraints(XPRBprob* prob);
	void genOrdConstraints(XPRBprob* prob);
	void genResourceConstraints(XPRBprob* prob);
	void genDurationConstraints(XPRBprob* prob);
	void genLowProblem(XPRBprob* prob);

	void printObj(ofstream* file, XPRBprob*);
	void printFailures(ofstream* file);
	void printResources(ofstream* file);
	void printTasks(ofstream* file);
	void printGamma(ofstream* file);
	void printTopProbOutput(XPRBprob* prob); 
	void printStarts(ofstream* file);
	void printTaskOrders(ofstream* file);
	void printLowProbOutput(XPRBprob* prob);
	int printer(string s, int verbosity, bool end = true, int maxVerb = 999);
};