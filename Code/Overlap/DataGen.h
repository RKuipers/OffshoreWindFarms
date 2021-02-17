#pragma once

#define SEED 42

#include <vector>
#include <algorithm>    // sort, shuffle
#include <random>
#include <iostream>		// cout
#include <string>		// stod
#include <fstream>		// ifstream, ofstream
#include <stdlib.h>     // srand, rand 
#include "InputData.h"
#include "Solution.h"

class DataGen
{
private:
	vector<string> readLine(ifstream* datafile, char sep = '\t');
	bool readEmpty(ifstream* datafile, char sep = '\t');
	int parseArray(vector<string> line, int start, vector<int>* res, int amount);
	int parseArrayDouble(vector<string> line, int start, vector<double>* res, int amount);

	int genRandomFailures(double yearlyFailRate, int nTurbines);
	vector<double> genRandomFailureTimes(int failures, int maxTime);

public:
	YearData* readYear(ifstream* file);
	MonthData* readMonth(ifstream* file);
	MixedData* readMixed(ifstream* file);
	vector<MonthData> genMonths(MixedData* data, YearSolution* sol);
	vector<MonthData> genMonths2(MixedData* data, YearSolution* sol);
};

