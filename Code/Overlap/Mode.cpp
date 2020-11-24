#include "Mode.h"

//----------------------------------------------MODEDIM--------------------------------------------

ModeDim::ModeDim(vector<string> names) : names(names), max(names.size()) {}

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
	return 0; // TODO
}

string Mode::getCurrentName()
{
	return "test"; // TODO
}

int Mode::getCurrentId()
{
	return 0; // TODO
}

int Mode::next()
{
	return 0; // TODO
}
