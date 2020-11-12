#include "MonthModel.h"

MonthData* MonthModel::getData()
{
	return data->getMonth();
}

Solution* MonthModel::genSolution(XPRBprob* p)
{
	MonthSolution sol(mode->getCurrentName(), mode->getCurrentId());

	vector<double> s;

	for (int i = 0; i < getData()->I; ++i)
		s.push_back(this->s[i].getSol());

	sol.setStarts(s);

	vector<vector<vector<int>>> a(getData()->V, vector<vector<int>>(getData()->I, vector<int>()));

	for (int v = 0; v < getData()->V; ++v)
		for (int i = 0; i < getData()->I; ++i)
			for (int j = 0; j < getData()->J; ++j)
				a[v][i].push_back(round(this->s[i].getSol()));

	sol.setOrders(a);

	return &sol;
}

void MonthModel::genProblem()
{
}

void MonthModel::genDecVars()
{
}

void MonthModel::genObj()
{
}

MonthModel::MonthModel(MonthData* data) : Model(data)
{ 
	// TODO: Remove this test code
	a = vector<vector<vector<XPRBvar>>>(1, vector<vector<XPRBvar>>(1, vector<XPRBvar>(1, p.newVar("a000", XPRB_BV))));
	s = vector<XPRBvar>(1, p.newVar("s0", XPRB_PL, 0));

	p.setObj(s[0]);
	p.setSense(XPRB_MINIM);

	p.newCtr(a[0][0][0] >= 0.5);
	p.newCtr(s[0] >= a[0][0][0]);
}
