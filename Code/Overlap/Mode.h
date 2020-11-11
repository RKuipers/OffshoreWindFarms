#pragma once

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

class ModeDim
{
protected:
	int current, max;
	vector<string> names;

public:
	virtual int next();
};

class ModeDimComb
	: public ModeDim
{
public:
	int next() override;
};

