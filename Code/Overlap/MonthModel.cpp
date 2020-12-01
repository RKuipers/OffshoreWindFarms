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
		s.push_back(abs(this->s[i].getSol()));

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

	genMomentCon();
	genRepeatCon();
	genOrderCon();
	genResourceCon();
	genDurationCon();
	genFinishCon();
	genFixedCon();
}

void MonthModel::genDecVars()
{
	for (int i = 0; i < getData()->I; ++i)
	{
		s[i] = p.newVar(("s_" + to_string(i)).c_str(), XPRB_PL);

		for (int v = 0; v < getData()->V; ++v)
			for (int j = 0; j < getData()->J; ++j)
				a[v][i][j] = p.newVar(("a_" + to_string(v) + "_" + to_string(i) + "_" + to_string(j)).c_str(), XPRB_BV);
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

void MonthModel::genMomentCon()
{
	for (int v = 0; v < getData()->V; ++v)
		for (int j = 0; j < getData()->J; ++j)
		{
			XPRBrelation ctr = a[v][0][j] <= 1;

			for (int i = 1; i < getData()->I; ++i)
				ctr.addTerm(a[v][i][j]);

			p.newCtr(("Mom_" + to_string(v) + "_" + to_string(j)).c_str(), ctr);
		}
}

void MonthModel::genRepeatCon()
{	
	for (int v = 0; v < getData()->V; ++v)
		for (int i = 0; i < getData()->I; ++i)
		{
			XPRBrelation ctr = a[v][i][0] <= 1;

			for (int j = 1; j < getData()->J; ++j)
				ctr.addTerm(a[v][i][j]);

			p.newCtr(("Rep_" + to_string(v) + "_" + to_string(i)).c_str(), ctr);
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
	MonthData* data = getData();

	for (int y = 0; y < getData()->Y; ++y)
		for (int i = 0; i < getData()->IMaint; ++i)
		{
			int VyStart = 0;
			if (y > 0)
				VyStart = getData()->Vy[y - 1];

			XPRBrelation ctr = a[VyStart][i][0] == getData()->rho[y][i];

			for (int v = VyStart; v < getData()->Vy[y]; ++v)
				for (int j = 0; j < getData()->J; ++j)
					if (j > 0 || v > VyStart)
						ctr.addTerm(a[v][i][j]);

			p.newCtr(("Res_" + to_string(y) + "_" + to_string(i)).c_str(), ctr);
		}
}

void MonthModel::genDurationCon()
{
	for (int y = 0; y < getData()->Y; ++y)
	{
		int VyStart = 0;
		if (y > 0)
			VyStart = getData()->Vy[y - 1];

		for (int v = VyStart; v < getData()->Vy[y]; ++v)
			for (int i = 0; i < getData()->I; ++i)
				for (int i_ = 0; i_ < getData()->I; ++i_)
					for (int j = 1; j < getData()->J; ++j)
					{
						if (i == i_)
							continue;

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

void MonthModel::genFixedCon()
{
	for (int i = 0; i < getData()->IInst; ++i)
	{
		s[i + getData()->IMaint].fix(getData()->sInst[i]);

		for (int v = 0; v < getData()->V; ++v)
		{
			if (getData()->aInst[v][i] == 0)
				for (int j = 0; j < getData()->J; ++j)
					a[v][i + getData()->IMaint][j].fix(0);
			else
			{
				XPRBrelation ctr = a[v][i + getData()->IMaint][0] == 1;

				for (int j = 1; j < getData()->J; ++j)
					ctr.addTerm(a[v][i + getData()->IMaint][j]);

				p.newCtr(("Fix_" + to_string(v) + "_" + to_string(i + getData()->IMaint)).c_str(), ctr);
			}
		}
	}
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