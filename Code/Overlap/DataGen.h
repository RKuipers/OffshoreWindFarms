#pragma once

#include <vector>
#include <iostream>
#include <fstream>		// ifstream, ofstream
#include "InputData.h"
#include "Solution.h"

class DataGen
{
public:
	MonthData readMonth(ifstream* file);
	YearData readYear(ifstream* file);
	vector<YearData> genMonths(YearSolution sol);
};

