#pragma once

#include <vector>
#include <numeric>
#include <algorithm>

using namespace std;

class MathHelp
{
public:
	static double Sum(vector<double>* vec);
	static int Sum(vector<int>* vec);
	static double WeightedSum(vector<double>* vec, vector<double>* w);
	static double Mean(vector<double>* vec);
	static double Mean(vector<int>* vec);
	static double WeightedMean(vector<double>* vec, vector<double>* w);
	static double Median(vector<double>* vec);
};

