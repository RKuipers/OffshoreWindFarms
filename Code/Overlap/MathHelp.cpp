#include "MathHelp.h"

double MathHelp::Mean(vector<double>* vec)
{
	return accumulate(vec->begin(), vec->end(), 0.0) / vec->size();
}

double MathHelp::Median(vector<double>* vec)
{
	vector<double> temp = *vec;
	sort(temp.begin(), temp.end());
    if (temp.size() % 2 == 0)
        return (temp[temp.size() / 2 - 1] + temp[temp.size() / 2]) / 2;
    else
        return temp[temp.size() / 2];
}
