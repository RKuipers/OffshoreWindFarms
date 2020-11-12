#pragma once

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
};

