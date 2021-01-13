#include "MonthModel.h"

//-----------------------------------------------BASE----------------------------------------------

MonthData* MonthModel::getData()
{
	return data->getMonth();
}

MonthSolution* MonthModel::genSolution(XPRBprob* p, double duration)
{
	solution = new MonthSolution(mode->getCurrentName(), mode->getCurrentId());
	solution->setResult(p->getObjVal(), duration);

	vector<double> s(getData()->I, 0.0);
	vector<vector<int>> a(getData()->V, vector<int>(getData()->J, -1));

	for (int v = 0; v < getData()->V; ++v)
		for (int j = 0; j < getData()->J; ++j)
			for (int i = 0; i < getData()->I; ++i)
				if (round(this->a[v][i][j].getSol()) == 1)
				{
					s[i] = abs(round(this->s[v][j].getSol()));
					a[v][j] = i;
					continue;
				}

	solution->setStarts(s);
	solution->setOrders(a);

	return solution;
}

void MonthModel::genProblem()
{
	genDecVars();
	genObj();

	genOrderCon();
	genLimitCon();
	genResourceCon();
	genDurationCon();
	genReleaseCon();
	genFinishCon();
	genFixedCons();
}

void MonthModel::genDecVars()
{
	for (int v = 0; v < getData()->V; ++v)
		for (int j = 0; j < getData()->J; ++j)
		{
			s[v][j] = p.newVar(("s_" + to_string(v) + "_" + to_string(j)).c_str(), XPRB_PL);

			for (int i = 0; i < getData()->I; ++i)
				a[v][i][j] = p.newVar(("a_" + to_string(v) + "_" + to_string(i) + "_" + to_string(j)).c_str(), XPRB_BV);
		}
}

void MonthModel::genObj()
{
	XPRBctr Obj = p.newCtr();

	for (int y = 0; y < getData()->Y; ++y)
	{
		int VyStart = 0;
		if (y > 0)
			VyStart = getData()->Vy[y - 1];

		for (int v = VyStart; v < getData()->Vy[y]; ++v)
			for (int i = 0; i < getData()->I; ++i)
				for (int j = 0; j < getData()->J; ++j)
					Obj.add(getData()->c[i] * a[v][i][j] * (s[v][j] + getData()->d[y][i]));
	}

	p.setObj(Obj);
}

void MonthModel::genOrderCon()
{
	for (int v = 0; v < getData()->V; ++v)
	{
		XPRBrelation ctr0 = a[v][0][0] <= 1;

		for (int i = 1; i < getData()->I; ++i)
			ctr0.addTerm(a[v][i][0]);

		p.newCtr(("Ord_" + to_string(v) + "_0").c_str(), ctr0);

		for (int j = 1; j < getData()->J; ++j)
		{
			XPRBrelation ctr = a[v][0][j] <= a[v][0][j-1];

			for (int i = 0; i < getData()->I; ++i)
			{
				ctr.addTerm(a[v][i][j]);
				ctr.addTerm(a[v][i][j-1], -1);
			}

			p.newCtr(("Ord_" + to_string(v) + "_" + to_string(j)).c_str(), ctr);
		}
	}
}

void MonthModel::genLimitCon()
{
	for (int v = 0; v < getData()->V; ++v)
		for (int i = 0; i < getData()->IMaint; ++i)
		{
			XPRBrelation ctr = a[v][i][0] <= getData()->A[i];

			for (int j = 1; j < getData()->J; ++j)
				ctr.addTerm(a[v][i][j]);

			p.newCtr(("Lim_" + to_string(v) + "_" + to_string(i)).c_str(), ctr);
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

			XPRBrelation ctr = a[VyStart][i][0] >= getData()->A[i] * getData()->rho[y][i];

			for (int v = VyStart; v < getData()->Vy[y]; ++v)
				for (int j = 0; j < getData()->J; ++j)
					if ((j > 0 || v > VyStart))
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
			for (int j = 1; j < getData()->J; ++j)
			{
				XPRBrelation ctr = s[v][j] - s[v][j-1] >= a[v][0][j-1] * getData()->d[y][0];

				for (int i = 1; i < getData()->I; ++i)
					ctr.addTerm(a[v][i][j - 1], -1 * getData()->d[y][i]);

				p.newCtr(("Dur_" + to_string(y) + "_" + to_string(v) + "_" + to_string(j)).c_str(), ctr);
			}
	}
}

void MonthModel::genReleaseCon()
{
	for (int v = 0; v < getData()->V; ++v)
		for (int i = 0; i < getData()->IMaint; ++i)
			for (int j = 0; j < getData()->J; ++j)
			{
				XPRBrelation ctr = s[v][j] >= getData()->r[i] * a[v][i][j];

				p.newCtr(("Rel_" + to_string(v) + "_" + to_string(i) + "_" + to_string(j)).c_str(), ctr);
			}
}

void MonthModel::genFinishCon()
{
	for (int y = 0; y < getData()->Y; ++y)
	{
		int VyStart = 0;
		if (y > 0)
			VyStart = getData()->Vy[y - 1];

		for (int v = VyStart; v < getData()->Vy[y]; ++v)
			for (int j = 0; j < getData()->J; ++j)
			{
				XPRBrelation ctr = s[v][j] + a[v][0][j] * getData()->d[y][0] <= getData()->T;

				for (int i = 0; i < getData()->I; ++i)
					ctr.addTerm(a[v][i][j], getData()->d[y][i]);

				p.newCtr(("Fin_" + to_string(y) + "_" + to_string(v) + "_" + to_string(j)).c_str(), ctr);
			}
	}
}

void MonthModel::genFixedCons()
{
	for (int ii = 0; ii < getData()->IInst; ++ii)
	{
		int i = ii + getData()->IMaint;

		XPRBrelation ctrS = s[getData()->vInst[ii]][0] * a[getData()->vInst[ii]][i][0] == getData()->sInst[ii];

		for (int j = 1; j < getData()->J; ++j)
			ctrS.add(s[getData()->vInst[ii]][j] * a[getData()->vInst[ii]][i][j]);

		p.newCtr(("FixS_" + to_string(i)).c_str(), ctrS);

		for (int v = 0; v < getData()->V; ++v)
		{
			XPRBrelation ctrA = a[v][i][0] == getData()->aInst[v][ii];

			for (int j = 1; j < getData()->J; ++j)
				ctrA.add(a[v][i][j]);

			p.newCtr(("FixA_" + to_string(v) + "_" + to_string(i)).c_str(), ctrA);
		}
	}
}

MonthModel::MonthModel(MonthData* data, Mode* mode, string name) : Model(data, mode, name)
{ 
	s = vector<vector<XPRBvar>>(data->V, vector<XPRBvar>(data->J));
	a = vector<vector<vector<XPRBvar>>>(data->V, vector<vector<XPRBvar>>(data->I, vector<XPRBvar>(data->J)));
}

void MonthModel::getRequirements(vector<double>* eps, vector<int>* rho)
{
	FeedbackModel* model = new FeedbackModel(getData(), mode);
	model->genProblem();

	(*eps) = model->getEps();

	(*rho) = vector<int>(getData()->Y, 0);
	for (int y = 0; y < getData()->Y; ++y)
		for (int i = 0; i < getData()->IMaint; ++i)
			(*rho)[y] = std::max((*rho)[y], getData()->rho[y][i]);
}

MonthSolution* MonthModel::solve()
{
	/*if (getData()->I > 12)
	{
		XPRBloadmat(p.getCRef());
		XPRSprob opt_prob = XPRBgetXPRSprob(p.getCRef());
		XPRStune(opt_prob, "g");
	}*/

	double dur = solveBasics();

	if (p.getMIPStat() == XPRB_MIP_INFEAS)
		return nullptr;

	return genSolution(&p, dur);
}

//---------------------------------------------FEEDBACK--------------------------------------------

void FeedbackModel::genProblem()
{
	MonthModel::genProblem();

	genMaxFinCon();
}

void FeedbackModel::genDecVars()
{
	MonthModel::genDecVars();

	T = p.newVar("T", XPRB_PL);
	for (int y = 0; y < getData()->Y; ++y)
		Ty[y] = p.newVar(("Ty_" + to_string(y)).c_str(), XPRB_PL);
}

void FeedbackModel::genObj()
{
	p.setObj(T);
}

void FeedbackModel::genFinishCon()
{
	for (int y = 0; y < getData()->Y; ++y)
	{
		int VyStart = 0;
		if (y > 0)
			VyStart = getData()->Vy[y - 1];

		for (int v = VyStart; v < getData()->Vy[y]; ++v)
			for (int j = 0; j < getData()->J; ++j)
			{
				XPRBrelation ctr = s[v][j] + a[v][0][j] * getData()->d[y][0] <= Ty[y];

				for (int i = 0; i < getData()->I; ++i)
					ctr.addTerm(a[v][i][j], getData()->d[y][i]);

				p.newCtr(("Fin_" + to_string(y) + "_" + to_string(v) + "_" + to_string(j)).c_str(), ctr);
			}
	}
}

void FeedbackModel::genMaxFinCon()
{
	for (int y = 0; y < getData()->Y; ++y)
		p.newCtr(("MaxFin_" + to_string(y)).c_str(), Ty[y] <= T);
}

FeedbackModel::FeedbackModel(MonthData* data, Mode* mode) : MonthModel(data, mode, "Feedback")
{
	Ty = vector<XPRBvar>(data->Y);
}

vector<double> FeedbackModel::getEps()
{
	solveBasics();

	vector<double> res = vector<double>(getData()->Y, 0.0);

	for (int y = 0; y < getData()->Y; ++y)
		res[y] = max(0.0, Ty[y].getSol() - getData()->T);

	return res;
}
