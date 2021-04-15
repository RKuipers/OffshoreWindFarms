#include "MathHelp.h"

double MathHelp::Sum(vector<double>* vec)
{
    return accumulate(vec->begin(), vec->end(), 0.0);
}

int MathHelp::Sum(vector<int>* vec)
{
    return accumulate(vec->begin(), vec->end(), 0);
}

double WeightedSum(vector<double>* vec, vector<double>* w)
{
    vector<double> v = vector<double>(vec->size);
    transform(vec->begin(), vec->end(), w->begin(), v.begin(), multiplies<double>());
    return MathHelp::Sum(&v);
}

double MathHelp::Mean(vector<double>* vec)
{
	return Sum(vec) / vec->size();
}

double MathHelp::Mean(vector<int>* vec)
{
    return Sum(vec) / vec->size();
}

double MathHelp::WeightedMean(vector<double>* vec, vector<double>* w)
{
    return WeightedSum(vec, w) / vec->size();
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
