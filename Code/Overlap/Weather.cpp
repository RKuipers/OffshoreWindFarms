#include "Weather.h"

vector<vector<vector<double>>> WeatherGen::genPercentages(vector<double> e, vector<double> limits, int S, int mode, double sd)
{
	int M = e.size();
	int Y = limits.size();

	vector<vector<vector<double>>> W = vector<vector<vector<double>>>(S, vector<vector<double>>(M, vector<double>(Y, 0.0)));

	for (int s = 0; s < S; ++s)
		for (int m = 0; m < M; ++m)
			for (int y = 0; y < Y; ++y)
			{
				double stdv;
				if (mode == 0)
					stdv = e[m] * sd * 0.01;
				else if (mode == 1)
					stdv = sd;

				double normalized = (limits[y] - e[m]) / stdv;
				//W[s][m][y] = 0.5 * erfc(-normalized * M_SQRT1_2);	// If the wind speed is normally distributed this gives the percentage of time it is above the limit
				W[s][m][y] = 0.66;
			}

	return W;
}
