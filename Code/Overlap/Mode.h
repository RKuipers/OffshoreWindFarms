#pragma once

#include <vector>
#include <string>

using namespace std;

class ModeDim
{
protected:
	int current, max;
	string dimName;
	vector<string> names;

public:
	ModeDim(string dimName, int max);
	ModeDim(vector<string> names, string dimName = "");

	virtual int next();
	virtual int checkCurrent(string name);
	virtual string getCurrent();
	virtual int getCurrentId();
};

class ModeDimComb
	: public ModeDim
{
public:
	int next() override;
};

class Mode
{
private:
	int current;
	vector<ModeDim> dims;

public:
	int getCurrent(string name);
	string getCurrentName();
	vector<string> getCurrentList();
	int getCurrentId();
	int next();

	void addDim(string dimName, int max);
	void addDim(vector<string> names, string dimName = "");
};

