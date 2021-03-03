#pragma once

#include <vector>

using namespace std;

class InputData;
class YearData;
class MonthData;
class MixedData;

class InputData
{
public:
	virtual YearData* getYear();
	virtual MonthData* getMonth();
	virtual MixedData* getMixed();
};

class YearData
	: public InputData
{
public:
	// Sets:
	int S, M, Y, Ip, Ir;	
	// Parameters:
	vector<vector<double>> c;			// y m
	vector<double> eH;					// m
	vector<vector<vector<int>>> Ft;		// m ir sig
	vector<int> H;						// m
	vector<int> L;						// y
	vector<vector<int>> LInst;			// y m
	double dP;
	vector<double> dPy;					// y
	vector<double> dR;					// ir
	vector<vector<double>> dRy;			// y ir
	vector<double> dD;					// ir
	int GU, GL;
	vector<vector<int>> A;				// y m
	vector<vector<int>> NInst;			// y m
	vector<int> Turbs;					// m
	vector<vector<vector<double>>> eps;	// sig m y
	vector<vector<vector<int>>> rho;	// sig m y
	vector<double> lambda;				// ir
	bool randFailMode;

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
	vector<vector<double>> d;		// y i
	vector<vector<int>> rho;		// y im
	vector<double> r;				// im
	vector<int> A;					// im
	double T;
	vector<pair<int, int>> PR;		// 'p'
	vector<double> sInst;			// ii
	vector<vector<int>> aInst;		// v ii
	vector<int> vInst;				// ii
	vector<int> yTrans;				// y (local to global)

	MonthData(int y, int v, int im, int ii, int j);

	MonthData* getMonth() override;
};

class MixedData
	: public YearData
{
public:
	// Sets:
	vector<int> IInst;						// m
	// Parameters:
	vector<int> rhoP;						// y
	vector<vector<int>> rhoR;				// y ir
	double T, DT;
	vector<vector<vector<double>>> FTime;	// m ir 'f'
	vector<vector<double>> sInst;			// m ii
	vector<vector<vector<double>>> dI;		// m y ii
	vector<vector<int>> aInst;				// m ii
	vector<vector<int>> vTypes;				// m 'v'

	MixedData(int s, int m, int y, int ip, int ir33);
	MixedData(const YearData& year);

	MixedData* getMixed() override;
};

