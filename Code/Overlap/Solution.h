#pragma once

#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include "InputData.h"
#include "MathHelp.h"

using namespace std;

class Solution
{
protected:
	string name;
	double value, duration; 

public:
	Solution(string name);

	void setResult(double value, double duration);
	double getObj();

	void printObj();
	void printDur();
	virtual void print() =0;
};

class YearSolution
	: public Solution
{
protected:
	vector<vector<int>> vessels;			// m y
	vector<vector<int>> planned;			// m ip
	vector<vector<vector<int>>> repairs;	// sig m ir
	vector<vector<vector<int>>> unhandled;	// sig m ir

	vector<vector<double>> avail;					// sig m
	vector<vector<double>> timeUnavailP;			// sig m
	vector<vector<vector<double>>> timeUnavailR;	// sig m ir
	vector<vector<vector<double>>> timeUnavailU;	// sig m ir
	vector<double> vCosts;							// m
	vector<double> tCosts;							// m

	YearData* data;

	void printVessels();
	void printPlanned();
	void printFailures();
	void printRepairs();
	void printUnhandled();

	void calcSecondaries();
	void printAvailability();
	void printScenarios();
	void printDinwoodie();
	void writeCSV();

public:
	YearSolution(string name, YearData* data);

	void setVessels(vector<vector<int>> N);
	void setPlanned(vector<vector<int>> P);
	void setRepairs(vector<vector<vector<int>>> R);
	void setUnhandled(vector<vector<vector<int>>> U);

	vector<vector<int>> getVessels();
	vector<vector<int>> getPlanned();
	vector<vector<vector<int>>> getRepairs();
	vector<vector<vector<int>>> getUnhandled();

	void print() override;
};

class MonthSolution
	: public Solution
{
protected:
	vector<vector<double>> starts;			// i a
	vector<double> finishes;				// i
	vector<vector<int>> orders;				// v j

	void printFinishes();
	void printStarts();
	void printOrders();

public:
	MonthSolution(string name);

	void setFinishes(vector<double> finishes);
	void setStarts(vector<vector<double>> s);
	void setOrders(vector<vector<int>> a);
	void print() override;
};

