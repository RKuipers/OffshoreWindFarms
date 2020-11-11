#pragma once

class Solution
{
protected:
	string modeName;
	int modeId;
	double duration;

public:
	void setMode(string name, int id);
	void setDur(double duration);
	virtual void print();
};

class YearSolution
	: public Solution
{
protected:
	vector<vector<vector<int>>> vessels;
	vector<vector<int>> planned;
	vector<vector<vector<int>>> reactive;

public:
	void print() override;
};

class MonthSolution
	: public Solution
{
protected:
	vector<int> starts;
	vector<vector<vector<int>>> assigns;

public:
	void print() override;
};

