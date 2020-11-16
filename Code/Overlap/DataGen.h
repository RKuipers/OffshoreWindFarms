#pragma once

#include <vector>
#include <iostream>
#include <fstream>		// ifstream, ofstream
#include "InputData.h"
#include "Solution.h"

class DataGen
{
private:
	vector<string> readLine(ifstream* datafile, char sep = '\t');

public:
	YearData readYear(ifstream* file);
	MonthData readMonth(ifstream* file);
	vector<MonthData> genMonths(YearSolution sol);
};

