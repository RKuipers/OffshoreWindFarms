#include "Mode.h"

//----------------------------------------------MODEDIM--------------------------------------------

ModeDim::ModeDim(string dimName, int max) : dimName(dimName), current(0), max(max)
{
	names = vector<string>(max, dimName);
	for (int i = 0; i < max; ++i)
		names[i] = names[i] + to_string(i);
}

ModeDim::ModeDim(vector<string> names, string dimName) : dimName(dimName), names(names), current(0), max(names.size()) {}

int ModeDim::next()
{
	current++;
	if (current >= max)
	{
		current = 0;
		return -1;
	}
	return current;
}

int ModeDim::checkCurrent(string name)
{
	if (dimName.compare(name) == 0)
		return current;

	for (int i = 0; i < max; ++i)
		if (names[i].compare(name) == 0)
			if (current == i)
				return 1;
			else
				return 0;

	return -1;
}

string ModeDim::getCurrent()
{
	return names[current];
}

int ModeDim::getCurrentId()
{
	return current;
}

//--------------------------------------------MODEDIMCOMB------------------------------------------

int ModeDimComb::next()
{
	return 0; // TODO
}

//-----------------------------------------------MODE----------------------------------------------

int Mode::getCurrent(string name)
{
	for (int i = 0; dims.size(); ++i)
	{
		int r = dims[i].checkCurrent(name);
		if (r != -1)
			return r;
	}
	return -1;
}

string Mode::getCurrentName()
{
	vector<string> list = getCurrentList();
	string res = "";
	for (int i = 0; list.size(); ++i)
		res = res + list[i];
	return res;
}

vector<string> Mode::getCurrentList()
{
	vector<string> res = vector<string>();
	for (int i = 0; dims.size(); ++i)
		res.push_back(dims[i].getCurrent());
	return res;
}

int Mode::getCurrentId()
{
	return current;
}

int Mode::next()
{
	current++;
	int i = 0;
	while (i < dims.size() && dims[i].next() == -1)
		i++;
	if (i >= dims.size())
		return -1;
	return i;
}

void Mode::addDim(string dimName, int max)
{
	dims.push_back(ModeDim(dimName, max));
}

void Mode::addDim(vector<string> names, string dimName)
{
	dims.push_back(ModeDim(names, dimName));
}
