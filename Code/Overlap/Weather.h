#pragma once

#define _USE_MATH_DEFINES
#include <cmath>
#include <vector>

using namespace std;

class Weather
{
public:
	int getWaveHeight(int timestep);
	int getWindSpeed(int timestep);
};

class WeatherGen
{
public:
	Weather genMonth(int month);

	//vector<vector<vector<double>>> genPercentages(vector<double> e, vector<double> limits, int S, int mode = 0, double sd = 5);
};

