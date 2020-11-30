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
	genDecVars();
	genObj();

	genLimitCon();
	genOrderCon();
	genResourceCon();
	genDurationCon();
	genFinishCon();
	genFixedACon();
	genFixedSCon();
}

void MonthModel::genDecVars()
{
	for (int i = 0; i < getData()->I; ++i)
	{
		s[i] = p.newVar(("s_" + to_string(i)).c_str(), XPRB_PL);

		for (int v = 0; v < getData()->V; ++v)
			for (int j = 0; j < getData()->J; ++j)
				a[v][i][j] = p.newVar(("a_" + to_string(v) + "_" + to_string(i) + to_string(j)).c_str(), XPRB_BV);
	}
}

void MonthModel::genObj()
{
	XPRBctr Obj = p.newCtr();

	for (int i = 0; i < getData()->I; ++i)
	{		
		Obj.addTerm(s[i], getData()->c[i]);
		Obj.add(getData()->c[i] * getData()->dMax[i]);
	}
	p.setObj(Obj);
}

void MonthModel::genLimitCon()
{
	for (int v = 0; v < getData()->V; ++v)
		for (int j = 0; j < getData()->J; ++j)
		{
			XPRBrelation ctr = a[v][0][j] <= 0;

			for (int i = 1; i < getData()->I; ++i)
				ctr.addTerm(a[v][i][j]);

			p.newCtr(("Lim_" + to_string(v) + "_" + to_string(j)).c_str(), ctr);
		}
}

void MonthModel::genOrderCon()
{
	for (int v = 0; v < getData()->V; ++v)
		for (int j = 1; j < getData()->J; ++j)
		{
			XPRBrelation ctr = a[v][0][j] <= a[v][0][j-1];

			for (int i = 1; i < getData()->I; ++i)
			{
				ctr.addTerm(a[v][i][j]);
				ctr.addTerm(a[v][i][j-1], -1);
			}

			p.newCtr(("Ord_" + to_string(v) + "_" + to_string(j)).c_str(), ctr);
		}
}

void MonthModel::genResourceCon()
{
	for (int y = 0; y < getData()->Y; ++y)
		for (int i = 0; i < getData()->I; ++i)
		{
			XPRBrelation ctr = getData()->rho[y][i] <= a[getData()->Vy[y]][i][0];

			int VyCap;
			if (y + 1 >= getData()->Y)
				VyCap = y + 1;
			else
				VyCap = getData()->Vy[y+1];

			for (int v = getData()->Vy[y]; v < VyCap; ++v)
				for (int j = 0; j < getData()->J; ++j)
					if (j > 0 || v > getData()->Vy[y])
						ctr.addTerm(a[v][i][j], -1);

			p.newCtr(("Res_" + to_string(y) + "_" + to_string(i)).c_str(), ctr);
		}
}

void MonthModel::genDurationCon()
{
	for (int y = 0; y < getData()->Y; ++y)
	{
		int VyCap;
		if (y + 1 >= getData()->Y)
			VyCap = y + 1;
		else
			VyCap = getData()->Vy[y + 1];

		for (int v = getData()->Vy[y]; v < VyCap; ++v)
			for (int i = 0; i < getData()->I; ++i)
				for (int i_ = 0; i_ < getData()->I; ++i_)
					for (int j = 1; j < getData()->J; ++j)
					{
						XPRBrelation ctr = getData()->M * (a[v][i][j] + a[v][i_][j-1]) + getData()->d[y][i_] * a[v][i_][j - 1] - 2 * getData()->M <= s[i] + getData()->s[y][i] - s[i_] - getData()->s[y][i_];

						p.newCtr(("Dur_" + to_string(y) + "_" + to_string(v) + "_" + to_string(i) + "_" + to_string(i_) + "_" + to_string(j)).c_str(), ctr);
					}
	}
}

void MonthModel::genFinishCon()
{
	for (int i = 0; i < getData()->I; ++i)
	{
		XPRBrelation ctr = s[i] + getData()->dMax[i] <= getData()->T;

		p.newCtr(("Fin_" + to_string(i)).c_str(), ctr);
	}
}

void MonthModel::genFixedACon()
{
	for (int v = 0; v < getData()->V; ++v)
		for (int i = 0; i < getData()->IInst; ++i)
		{
			if (getData()->aInst[v][i] == 0)
				for (int j = 0; j < getData()->J; ++j)
					a[v][i + getData()->IMaint][j].fix(0);
			else
			{
				XPRBrelation ctr = a[v][i + getData()->IMaint][0] == 1;

				for (int j = 1; j < getData()->J; ++j)
					ctr.addTerm(a[v][i][j]);

				p.newCtr(("Fix_" + to_string(v) + "_" + to_string(i)).c_str(), ctr);
			}
		}
}

void MonthModel::genFixedSCon()
{
	for (int i = 0; i < getData()->IInst; ++i)
		s[i + getData()->IMaint].fix(getData()->sInst[i]);
}

MonthModel::MonthModel(MonthData* data, Mode* mode) : Model(data, mode, "Month")
{ 
	s = vector<XPRBvar>(data->I);
	a = vector<vector<vector<XPRBvar>>>(data->V, vector<vector<XPRBvar>>(data->I, vector<XPRBvar>(data->J)));
}

MonthSolution* MonthModel::solve()
{
	double dur = solveBasics();

	return genSolution(&p, dur);
}