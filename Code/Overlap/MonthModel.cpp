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
		
	vector<vector<double>> s(getData()->V, vector<double>());
	vector<double> f;
	vector<vector<int>> a(getData()->V, vector<int>());

	int stat = p->getMIPStat();

	if (p->getMIPStat() >= XPRB_MIP_SOLUTION)
	{
		for (int v = 0; v < getData()->V; ++v)
			for (int j = 0; j < getData()->J; ++j)
				for (int i = 0; i < getData()->I; ++i)
					if (round(this->a[v][i][j].getSol()) == 1)
					{
						s[v].push_back(abs(this->s[v][j].getSol()));
						a[v].push_back(i);
						continue;
					}

		for (int i = 0; i < getData()->I; ++i)
			f.push_back(abs(this->f[i].getSol()));
	}

	solution->setFinishes(f);
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
	genDeadlineCon();
	genFixedCons();
	genFinishCon();	
	genPrecedenceCon();
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

	for (int i = 0; i < getData()->I; ++i)
		f[i] = p.newVar(("f_" + to_string(i)).c_str(), XPRB_PL);
}

void MonthModel::genObj()
{
	XPRBctr Obj = p.newCtr();

	for (int i = 0; i < getData()->IMaint; ++i)
	{
		Obj.addTerm(f[i], getData()->c[i]);
		Obj.add(-1 * getData()->r[i] * getData()->c[i]);
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

			for (int i = 1; i < getData()->I; ++i)
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
	for (int y = 0; y < getData()->Y; ++y)
		for (int i = 0; i < getData()->IMaint; ++i)
		{
			int VyStart = 0;
			if (y > 0)
				VyStart = getData()->Vy[y - 1];

			XPRBrelation ctr = a[VyStart][i][0] == getData()->A[i] * getData()->rho[y][i];

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

void MonthModel::genDeadlineCon()
{
	for (int i = 1; i < getData()->I; ++i)
	{
		XPRBrelation ctr = f[i] <= getData()->T;

		p.newCtr(("Dead_" + to_string(i)).c_str(), ctr);
	}
}

void MonthModel::genFixedCons()
{
	for (int ii = 0; ii < getData()->IInst; ++ii)
	{
		int i = ii + getData()->IMaint;

		for (int j = 0; j < getData()->J; ++j)
		{
			int v = getData()->vInst[ii];

			XPRBrelation ctrSL = getData()->sInst[ii] * a[v][i][j] <= s[v][j];
			XPRBrelation ctrSU = s[v][j] <= getData()->T - a[v][i][j] * (getData()->T - getData()->sInst[ii]);

			p.newCtr(("FixSL_" + to_string(i) + "_" + to_string(j)).c_str(), ctrSL);
			p.newCtr(("FixSU_" + to_string(i) + "_" + to_string(j)).c_str(), ctrSU);
		}

		for (int v = 0; v < getData()->V; ++v)
		{
			XPRBrelation ctrA = a[v][i][0] == getData()->aInst[v][ii];

			for (int j = 1; j < getData()->J; ++j)
				ctrA.add(a[v][i][j]);

			p.newCtr(("FixA_" + to_string(v) + "_" + to_string(i)).c_str(), ctrA);
		}
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
			for (int i = 0; i < getData()->I; ++i)
				for (int j = 0; j < getData()->J; ++j)
				{
					XPRBrelation ctr = getData()->d[y][i] + s[v][j] <= f[i] + 3 * getData()->T * (1 - a[v][i][j]);

					p.newCtr(("Fin_" + to_string(v) + "_" + to_string(i) + "_" + to_string(j)).c_str(), ctr);
				}
	}
}

void MonthModel::genPrecedenceCon()
{
	for(int ip = 0; ip  < getData()->PR.size(); ++ip)
		for (int v = 0; v < getData()->V; ++v)
			for (int j = 0; j < getData()->J; ++j)
			{
				int i = getData()->PR[ip].first;
				int i_ = getData()->PR[ip].second;

				XPRBrelation ctr = f[i] - (1 - a[v][i_][j]) * 3 * getData()->T <= s[v][j];

				p.newCtr(("Prec_" + to_string(ip) + "_" + to_string(v) + "_" + to_string(j)).c_str(), ctr);
			}
				
}

MonthModel::MonthModel(MonthData* data, Mode* mode, string name, int id) : Model(data, mode, name + to_string(id))
{ 
	this->id = id;
	s = vector<vector<XPRBvar>>(data->V, vector<XPRBvar>(data->J));
	a = vector<vector<vector<XPRBvar>>>(data->V, vector<vector<XPRBvar>>(data->I, vector<XPRBvar>(data->J)));
	f = vector<XPRBvar>(data->I);
}

void MonthModel::getRequirements(vector<double>* eps, vector<int>* rho, int globalY)
{
	FeedbackModel* model = new FeedbackModel(getData(), mode, id);
	model->genProblem();

	(*eps) = model->getEps(globalY);

	(*rho) = vector<int>(globalY, 0);
	for (int y = 0; y < getData()->Y; ++y)
		for (int i = 0; i < getData()->IMaint; ++i)
			(*rho)[getData()->yTrans[y]] = std::max((*rho)[y], getData()->rho[y][i]);
}

MonthSolution* MonthModel::solve(int maxTime)
{
	/*if (getData()->I > 12)
	{
		XPRBloadmat(p.getCRef());
		XPRSprob opt_prob = XPRBgetXPRSprob(p.getCRef());
		XPRStune(opt_prob, "g");
	}*/

	double dur = solveBasics(maxTime);

	if (p.getMIPStat() != XPRB_MIP_SOLUTION && p.getMIPStat() != XPRB_MIP_OPTIMAL)
		return nullptr;

	/*for (int i = 0; i < getData()->I; ++i)
		cout << "f_" << i << ": " << f[i].getSol() << endl;

	cout << endl;

	for (int v = 0; v < getData()->V; ++v)
		for (int j = 0; j < getData()->J; ++j)
			cout << "s_" << v << "_" << j << ": " << s[v][j].getSol() << endl;

	cout << endl;

	for (int v = 0; v < getData()->V; ++v)
		for (int i = 0; i < getData()->I; ++i)
			for (int j = 0; j < getData()->J; ++j)
				cout << "a_" << v << "_" << i << "_" << j << ": " << a[v][i][j].getSol() << endl;

	cout << endl;*/

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

void FeedbackModel::genDeadlineCon()
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

				p.newCtr(("Dead_" + to_string(y) + "_" + to_string(v) + "_" + to_string(j)).c_str(), ctr);
			}
	}
}

void FeedbackModel::genMaxFinCon()
{
	for (int y = 0; y < getData()->Y; ++y)
		p.newCtr(("MaxFin_" + to_string(y)).c_str(), Ty[y] <= T);
}

FeedbackModel::FeedbackModel(MonthData* data, Mode* mode, int id) : MonthModel(data, mode, "Feedback", id)
{
	Ty = vector<XPRBvar>(data->Y);
}

vector<double> FeedbackModel::getEps(int globalY)
{
	solveBasics(0, false);

	vector<double> res = vector<double>(globalY, 0.0);

	for (int y = 0; y < getData()->Y; ++y)
		res[getData()->yTrans[y]] = max(0.0, this->Ty[y].getSol() - getData()->T);

	return res;
}
