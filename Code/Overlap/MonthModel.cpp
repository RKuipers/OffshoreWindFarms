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

	vector<double> s;

	for (int i = 0; i < getData()->I; ++i)
		s.push_back(abs(this->s[i].getSol()));

	solution->setStarts(s);

	vector<vector<vector<int>>> a(getData()->V, vector<vector<int>>(getData()->I, vector<int>()));

	for (int v = 0; v < getData()->V; ++v)
		for (int i = 0; i < getData()->I; ++i)
			for (int i_ = 0; i_ < getData()->I; ++i_)
				if (i != i_)
					a[v][i].push_back(round(this->a[v][i][i_].getSol()));
				else
					a[v][i].push_back(-1);

	vector<vector<int>> aF(getData()->V, vector<int>());
	vector<vector<int>> aL(getData()->V, vector<int>());

	for (int v = 0; v < getData()->V; ++v)
		for (int i = 0; i < getData()->I; ++i)
		{
			aF[v].push_back(round(this->aF[v][i].getSol()));
			aL[v].push_back(round(this->aL[v][i].getSol()));
		}

	solution->setOrders(a, aF, aL);

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
	genFixedCon();
}

void MonthModel::genDecVars()
{
	for (int i = 0; i < getData()->I; ++i)
	{
		s[i] = p.newVar(("s_" + to_string(i)).c_str(), XPRB_PL);

		for (int v = 0; v < getData()->V; ++v)
		{
			aF[v][i] = p.newVar(("aF_" + to_string(v) + "_" + to_string(i)).c_str(), XPRB_BV);
			aL[v][i] = p.newVar(("aL_" + to_string(v) + "_" + to_string(i)).c_str(), XPRB_BV);

			for (int i_ = 0; i_ < getData()->I; ++i_)
				if (i_ != i)
					a[v][i][i_] = p.newVar(("a_" + to_string(v) + "_" + to_string(i) + "_" + to_string(i_)).c_str(), XPRB_BV);
		}
	}
}

void MonthModel::genObj()
{
	XPRBctr Obj = p.newCtr();

	for (int i = 0; i < getData()->IMaint; ++i)
	{		
		Obj.addTerm(s[i], getData()->c[i]);
		Obj.add(getData()->c[i] * getData()->dMax[i]);
	}
	p.setObj(Obj);
}

void MonthModel::genLimitCon()
{
	for (int v = 0; v < getData()->V; ++v)
	{
		XPRBrelation ctr = aF[v][0] <= 1;

		for (int i = 1; i < getData()->I; ++i)
			ctr.addTerm(aF[v][i]);

		p.newCtr(("Lim_" + to_string(v)).c_str(), ctr);
	}
}

void MonthModel::genOrderCon()
{
	for (int v = 0; v < getData()->V; ++v)
		for (int i = 0; i < getData()->I; ++i)
		{
			XPRBrelation ctr = aF[v][i] == aL[v][i];

			for (int i_ = 0; i_ < getData()->I; ++i_)
			{
				if (i == i_)
					continue;

				ctr.addTerm(a[v][i_][i]);
				ctr.addTerm(a[v][i][i_], -1);
			}

			p.newCtr(("Ord_" + to_string(v) + "_" + to_string(i)).c_str(), ctr);
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

			XPRBrelation ctr = aF[VyStart][i] >= getData()->rho[y][i];

			for (int v = VyStart; v < getData()->Vy[y]; ++v)
			{
				if (v != VyStart)
					ctr.addTerm(aF[v][i]);

				for (int i_ = 0; i_ < getData()->I; ++i_)
					if ((i_ > 0 || v > VyStart) && i != i_)
						ctr.addTerm(a[v][i_][i]);
			}

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
				{
					if (i == i_)
						continue;

					XPRBrelation ctr = s[i_] + getData()->s[y][i_] - s[i] - getData()->s[y][i] >= (getData()->M + getData()->d[y][i]) * a[v][i][i_] - getData()->M;

					p.newCtr(("Dur_" + to_string(y) + "_" + to_string(v) + "_" + to_string(i) + "_" + to_string(i_)).c_str(), ctr);
				}
	}
}

void MonthModel::genFinishCon()
{
	for (int i = 0; i < getData()->IMaint; ++i)
	{
		XPRBrelation ctr = s[i] + getData()->dMax[i] <= getData()->T;

		p.newCtr(("Fin_" + to_string(i)).c_str(), ctr);
	}
}

void MonthModel::genFixedCon()
{
	for (int ii = 0; ii < getData()->IInst; ++ii)
	{
		int i = ii + getData()->IMaint;
		s[i].fix(getData()->sInst[ii]);

		for (int v = 0; v < getData()->V; ++v)
		{
			if (getData()->aInst[v][ii] == 0)
			{
				aF[v][i].fix(0);
				aL[v][i].fix(0);

				for (int i_ = 0; i_ < getData()->I; ++i_)
				{
					if (i == i_)
						continue;

					a[v][i][i_].fix(0);
					a[v][i_][i].fix(0);
				}
			}
			else
			{
				XPRBrelation ctr = aF[v][i] == 1;

				for (int i_ = 0; i_ < getData()->I; ++i_)
					if (i != i_)
						ctr.addTerm(a[v][i_][i]);

				p.newCtr(("Fix_" + to_string(v) + "_" + to_string(i)).c_str(), ctr);
			}
		}
	}
}

MonthModel::MonthModel(MonthData* data, Mode* mode, string name) : Model(data, mode, name)
{ 
	s = vector<XPRBvar>(data->I);
	a = vector<vector<vector<XPRBvar>>>(data->V, vector<vector<XPRBvar>>(data->I, vector<XPRBvar>(data->I)));
	aF = vector<vector<XPRBvar>>(data->V, vector<XPRBvar>(data->I));
	aL = vector<vector<XPRBvar>>(data->V, vector<XPRBvar>(data->I));

	genProblem();
}

void MonthModel::getRequirements(vector<double>* eps, vector<int>* rho)
{
	return;

	FeedbackModel* model = new FeedbackModel(getData(), mode);

	(*eps) = model->getEps();

	for (int y = 0; y < getData()->Y; ++y)
		for (int i = 0; i < getData()->IMaint; ++i)
			(*rho)[y] = std::max(rho->at(y), getData()->rho[y][i]);
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
		for (int i = 0; i < getData()->IMaint; ++i)
		{
			XPRBrelation ctr = s[i] + getData()->s[y][i] + getData()->d[y][i] <= Ty[y];

			p.newCtr(("Fin_" + to_string(y) + "_" + to_string(i)).c_str(), ctr);
		}
}

void FeedbackModel::genMaxFinCon()
{
	XPRBrelation ctr = Ty[0] <= T;

	for (int y = 1; y < getData()->Y; ++y)
		ctr.addTerm(Ty[y]);

	p.newCtr("MaxFin", ctr);
}

FeedbackModel::FeedbackModel(MonthData* data, Mode* mode) : MonthModel(data, mode, "Feedback")
{
	vector<XPRBvar> Ty = vector<XPRBvar>(data->Y);
}

vector<double> FeedbackModel::getEps()
{
	genProblem();
	solveBasics();

	vector<double> res = vector<double>(getData()->Y, 0.0);

	for (int y = 0; y < getData()->Y; ++y)
		res[y] = max(0.0, Ty[y].getSol() - getData()->T);

	return res;
}
