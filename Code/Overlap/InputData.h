#pragma once

#include <vector>

using namespace std;

class InputData;
class YearData;
class MonthData;

class InputData
{
public:
	virtual YearData* getYear();
	virtual MonthData* getMonth();
};

class YearData
	: public InputData
{
public:
	// Sets:
	int S, M, Y, Ip, Ir;	
	// Parameters:
	vector<vector<double>> c;		// y m
	double dP;
	vector<double> eH;				// m
	vector<vector<vector<int>>> f;	// m ir sig
	vector<int> H;					// m
	vector<int> L;					// y
	vector<vector<int>> LInst;		// y m
	vector<double> dPy;				// y
	vector<vector<double>> dR;		// y ir
	int Gmin, Gmax;
	vector<vector<int>> A;			// y m
	vector<vector<int>> NInst;		// y m

	YearData(int s, int m, int y, int ip, int ir);

	YearData* getYear() override;
};

class MonthData
	: public InputData
{
public:
	// Sets:
	int Y, V, IMaint, IInst, I, J;
	vector<int> Vy;					// y
	// Parameters:
	vector<double> c;				// im
	vector<vector<double>> s;		// y i
	vector<vector<double>> d;		// y i
	vector<vector<int>> rho;		// y im
	int M;
	double T;
	vector<double> sInst;			// ii
	vector<vector<int>> aInst;		// v ii

	MonthData(int y, int v, int im, int ii, int j);

	MonthData* getMonth() override;
};

