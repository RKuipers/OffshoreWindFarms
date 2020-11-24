#pragma once

#include <vector>
#include <string>

using namespace std;

class ModeDim
{
protected:
	int current, max;
	vector<string> names;

public:
	ModeDim(vector<string> names);

	virtual int next();
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
	vector<ModeDim> dims;

public:
	int getCurrent(string name);
	string getCurrentName();
	int getCurrentId();
	int next();
};

