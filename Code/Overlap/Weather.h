#pragma once

class WeatherGen
{
public:
	Weather genMonth(int month);
};

class Weather
{
public:
	int getWaveHeight(int timestep);
	int getWindSpeed(int timestep);
};

