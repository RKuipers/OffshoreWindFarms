#pragma once

#include <vector>
#include <iostream>		// cout
#include <string>		// stod
#include <fstream>		// ifstream, ofstream
#include "InputData.h"
#include "Solution.h"

class DataGen
{
private:
	vector<string> readLine(ifstream* datafile, char sep = '\t');
	bool readEmpty(ifstream* datafile, char sep = '\t');
	int parseArray(vector<string> line, int start, vector<int>* res, int amount);

public:
	YearData readYear(ifstream* file);
	MonthData readMonth(ifstream* file);
	vector<MonthData> genMonths(YearSolution sol);
};

