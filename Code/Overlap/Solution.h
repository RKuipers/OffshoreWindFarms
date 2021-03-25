#pragma once

#include <vector>
#include <string>
#include <iostream>

using namespace std;

class Solution
{
protected:
	string modeName;
	int modeId;
	double value, duration; 
	// TODO Maybe: Set option somewhere to print modeinfo

public:
	Solution(string name, int id);

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
	vector<vector<vector<int>>> vessels;	// m y
	vector<vector<int>> planned;			// m i
	vector<vector<vector<int>>> repairs;	// sig m i
	vector<vector<vector<int>>> unhandled;	// sig m i

	void printVessels();
	void printPlanned();
	void printRepairs();
	void printUnhandled();

public:
	YearSolution(string name, int id);

	void setVessels(vector<vector<vector<int>>> N);
	void setPlanned(vector<vector<int>> P);
	void setRepairs(vector<vector<vector<int>>> R);
	void setUnhandled(vector<vector<vector<int>>> U);

	vector<vector<vector<int>>> getVessels();
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
	MonthSolution(string name, int id);

	void setFinishes(vector<double> finishes);
	void setStarts(vector<vector<double>> s);
	void setOrders(vector<vector<int>> a);
	void print() override;
};

