#include "MonthModel.h"

MonthData* MonthModel::getData()
{
	return data->getMonth();
}

MonthSolution* MonthModel::genSolution(XPRBprob* p, double duration)
{
	solution = new MonthSolution(mode->getCurrentName(), mode->getCurrentId());
	solution->setResult(p->getObjVal(), duration);

	vector<double> s;

	for (int i = 0; i < getData()->I; ++i)
		s.push_back(this->s[i].getSol());

	solution->setStarts(s);

	vector<vector<vector<int>>> a(getData()->V, vector<vector<int>>(getData()->I, vector<int>()));

	for (int v = 0; v < getData()->V; ++v)
		for (int i = 0; i < getData()->I; ++i)
			for (int j = 0; j < getData()->J; ++j)
				a[v][i].push_back(round(this->a[v][i][j].getSol()));

	solution->setOrders(a);

	return solution;
}

void MonthModel::genProblem()
{
	// TODO
}

void MonthModel::genDecVars()
{
	// TODO
}

void MonthModel::genObj()
{
	// TODO
}

MonthModel::MonthModel(MonthData* data, Mode* mode) : Model(data, mode, "Month")
{ 
	// TODO: Remove this test code
	vector<XPRBvar> a00;
	a00.push_back(p.newVar("a000", XPRB_BV));
	a00.push_back(p.newVar("a001", XPRB_BV));
	vector<XPRBvar> a01;
	a01.push_back(p.newVar("a010", XPRB_BV));
	a01.push_back(p.newVar("a011", XPRB_BV));
	a = vector<vector<vector<XPRBvar>>>(1, {a00, a01});

	s.push_back(p.newVar("s0", XPRB_PL, 0));
	s.push_back(p.newVar("s1", XPRB_PL, 0));

	p.setObj(s[0] + s[1]);
	p.setSense(XPRB_MINIM);

	p.newCtr(a[0][0][0] + a[0][0][1] == 1);
	p.newCtr(a[0][1][0] + a[0][1][1] == 1);
	p.newCtr(a[0][0][0] + a[0][1][0] <= 1);
	p.newCtr(a[0][0][1] + a[0][1][1] <= 1);
	p.newCtr(s[0] >= 3 * a[0][0][1]);
	p.newCtr(s[1] >= 5 * a[0][1][1]);
}
