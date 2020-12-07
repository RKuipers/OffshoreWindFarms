#include "InputData.h"

YearData::YearData(int s, int m, int y, int ip, int ir) : S(s), M(m), Y(y), Ip(ip), Ir(ir)
{
	c = vector<vector<double>>(Y, vector<double>(M, 0.0));
	eH = vector<double>(M, 0.0);
	f = vector<vector<vector<int>>>(M, vector<vector<int>>(Ir, vector<int>(S, 0)));
	H = vector<int>(M, 0);
	L = vector<int>(Y, 0);
	LInst = vector<vector<int>>(Y, vector<int>(M, 0));
	dPy = vector<double>(Y, 0.0);
	dR = vector<vector<double>>(Y, vector<double>(Ir, 0.0));
	A = vector<vector<int>>(Y, vector<int>(M, 0));
	NInst = vector<vector<int>>(Y, vector<int>(M, 0));
	Turbs = vector<int>(M, 0);
	eps = vector<vector<vector<double>>>(S, vector<vector<double>>(M, vector<double>(Y, 0.0)));
	rho = vector<vector<vector<int>>>(S, vector<vector<int>>(M, vector<int>(Y, 0)));
}

YearData* YearData::getYear()
{
	return this;
}

MonthData::MonthData(int y, int v, int im, int ii) : Y(y), V(v), IMaint(im), IInst(ii), I(im + ii)
{
	Vy = vector<int>(Y, 0);
	c = vector<double>(I, 0.0);
	s = vector<vector<double>>(Y, vector<double>(I, 0.0));
	d = vector<vector<double>>(Y, vector<double>(I, 0.0));
	dMax = vector<double>(IMaint, 0.0);
	rho = vector<vector<int>>(Y, vector<int>(I, 0));
	sInst = vector<double>(IInst, 0.0);
	aInst = vector<vector<int>>(V, vector<int>(IInst, 0));
}

MonthData* MonthData::getMonth()
{
	return this;
}

MixedData::MixedData(int s, int m, int y, int ip, int ir) : YearData(s, m, y, ip, ir)
{
	IInst = vector<int>(M, 0);
	sP = vector<double>(Y, 0.0);
	sR = vector<vector<double>>(Y, vector<double>(Ir, 0.0));
	rhoP = vector<int>(Y, 0);
	rhoR = vector<vector<int>>(Y, vector<int>(Ir, 0));
	sInst = vector<vector<double>>(M, vector<double>());	// Empty
	dI = vector<vector<vector<double>>>(M, vector<vector<double>>(Y, vector<double>()));; // Empty
	aInst = vector<vector<int>>(M, vector<int>());	// Empty
	vTypes = vector<vector<int>>(M, vector<int>()); // Empty
}

MixedData::MixedData(const YearData& year) : YearData(year)
{
	IInst = vector<int>(M, 0);
	sP = vector<double>(Y, 0.0);
	sR = vector<vector<double>>(Y, vector<double>(Ir, 0.0));
	rhoP = vector<int>(Y, 0);
	rhoR = vector<vector<int>>(Y, vector<int>(Ir, 0));
	sInst = vector<vector<double>>(M, vector<double>());	// Empty
	dI = vector<vector<vector<double>>>(M, vector<vector<double>>(Y, vector<double>()));; // Empty
	aInst = vector<vector<int>>(M, vector<int>());	// Empty
	vTypes = vector<vector<int>>(M, vector<int>()); // Empty
}

MixedData* MixedData::getMixed()
{
	return this;
}

YearData* InputData::getYear()
{
	return nullptr;
}

MonthData* InputData::getMonth()
{
	return nullptr;
}

MixedData* InputData::getMixed()
{
	return nullptr;
}
