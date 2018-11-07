#ifndef WEATHER_H
#define WEATHER_H

#include <vector>

class WeatherGen
{
public:
    int GenerateWeather(vector<int> prevWeather);   // Generate weather given previous weather
};

#endif