#include "DataGen.h"

vector<string> DataGen::readLine(ifstream* datafile, char sep)
{
	string line = "#";
	vector<string> res = vector<string>();

	while (line[0] == '#')
		getline(*datafile, line);

	if (line.empty())
		return res;

	size_t pos = 0;

	while (line.find(sep, pos) != string::npos)
	{
		size_t newPos = line.find(sep, pos);
		res.push_back(line.substr(pos, newPos - pos));
		pos = newPos + 1;
	}

	res.push_back(line.substr(pos));
}

YearData DataGen::readYear(ifstream* file)
{
	
}

MonthData DataGen::readMonth(ifstream* file)
{
	return MonthData(0, 0, 0, 0, 0);
}

vector<MonthData> DataGen::genMonths(YearSolution sol)
{
	return vector<MonthData>();
}
