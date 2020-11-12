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
	double duration;

public:
	Solution(string name, int id);

	void setDur(double duration);
	virtual void print() =0;
};

class YearSolution
	: public Solution
{
protected:
	vector<vector<vector<int>>> vessels;	// sig m y
	vector<vector<int>> planned;			// m i
	vector<vector<vector<int>>> reactive;	// sig m i

	void printVessels();
	void printPlanned();
	void printReactive();

public:
	YearSolution(string name, int id);

	void setVessels(vector<vector<vector<int>>> N);
	void setPlanned(vector<vector<int>> P);
	void setReactive(vector<vector<vector<int>>> R);

	void print() override;
};

class MonthSolution
	: public Solution
{
protected:
	vector<double> starts;					// i
	vector<vector<int>> orders;				// v 

	void printStarts();
	void printOrders();

public:
	MonthSolution(string name, int id);

	void setStarts(vector<double> s);
	void setOrders(vector<vector<vector<int>>> a);
	void print() override;
};

