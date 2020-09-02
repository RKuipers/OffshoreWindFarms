#pragma once

#include "Mode.cpp"
#include "Weathergen.h"
#include "xprb_cpp.h"
#include "xprs.h"
#include <ctime>		// clock
#include <fstream>		// ifstream, ofstream
#include <string>		// string, to_string
#include <vector>		// vector
#include <iostream>		// cout
#include <tuple>		// tuple

// Program settings
#define SEED 42 * NTIMES
#define WEATHERTYPE 1
#define VERBOSITY 2		// The one to edit
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

class Optimiser
{
protected:
	// Model settings
	int nPeriods, optimal, maxPTime, maxFTime;
	string baseName;
	vector<tuple<int, string>> sols;

	// Model parameters
	vector<vector<int>> C;			// Costs (Resource, Period)
	vector<int> d;					// Duration (Task)
	vector<vector<int>> sa;			// Start Value (Task, Time)
	vector<vector<int>> rho;		// Required resources (Resource, Task)
	vector<vector<int>> m;			// Max resources (Resource, Period)
	vector<vector<int>> lambda;		// Time until failure (Asset, Task)

	// Model variables
	vector<vector<XPRBvar>> N;			// Needed resources (Resource, Period)
	vector<vector<XPRBvar>> o;			// Online turbines (Asset, Time)
	vector<vector<vector<XPRBvar>>> s;	// Started tasks (Asset, Task, Time)

	// Helper object
	WeatherGenerator wg;

	Optimiser(int nPeriods, int nRes, int nTasks, int nTimes, int nAssets, string name, const WeatherGenerator& weaGen);

	virtual Mode initMode() = 0;

	virtual void readTasks(ifstream* datafile, int taskType, vector<int>* limits) = 0;
	void readResources(ifstream* datafile);
	virtual void readValues(ifstream* datafile) = 0;
	virtual void readLambdas(ifstream* datafile) = 0;
	virtual void readData() = 0;
	void splitString(string s, vector<string>* res, char sep = ' ');
	int parsePeriodical(char type, vector<string> line, int start, vector<int>* res, int amount);
	vector<vector<string>> parseSection(ifstream* datafile, string name, bool canCopy = true, int expectedAmount = -1);

	virtual void genDecisionVariables(XPRBprob* prob) = 0;
	virtual void genObjective(XPRBprob* prob) = 0;
	void genBasicProblem(XPRBprob* prob, Mode* m);
	virtual void genPartialProblem(XPRBprob* prob, Mode* m) = 0;
	virtual void genFullProblem(XPRBprob* prob, Mode* m) = 0;
	void genCon(XPRBprob* prob, const XPRBrelation& ac, string base, int nInd, int* indices, bool cut);

	void printWeather(vector<int> waveheights);
	void printObj(ofstream* file, XPRBprob* prob);
	void printTurbines(ofstream* file);
	void printResources(ofstream* file);
	void printTasks(ofstream* file);
	void printProbOutput(XPRBprob* prob, Mode* m, int id);
	void printModeOutput(Mode* m);
	int printer(string s, int verbosity, bool end = true, int maxVerb = 999);
	string boolVec2Str(vector<bool> vec);

	void solveProblem(XPRBprob* prob, bool tune, string name, int maxTime);

public:
	void Run();
};