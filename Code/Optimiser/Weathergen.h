#pragma once

#include <vector>		// vector
#include <algorithm>    // max, count

using namespace std;

class WeatherGenerator
{
private:
	int base, variety, bonus, nTimes, tpp;
	vector<int> waveheights;

public:
	WeatherGenerator(int base, int variety, int nTimes, int tpp);

	vector<int> generateWeather();
	vector<vector<int>> generateStartValues(vector<int> durs, vector<int> limits);
};