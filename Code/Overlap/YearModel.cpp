#include "YearModel.h"

YearData* YearModel::getData()
{
	return data->getYear();
}

YearSolution* YearModel::genSolution(XPRBprob* p, double duration)
{
	solution = new YearSolution(mode->getCurrentName(), mode->getCurrentId());
	solution->setResult(p->getObjVal(), duration);

	vector<vector<vector<int>>> N(getData()->Y, vector<vector<int>>(getData()->M, vector<int>()));

	for (int y = 0; y < getData()->Y; ++y)
		for (int m = 0; m < getData()->M; ++m)
			for (int sig = 0; sig < getData()->S; ++sig)
				N[y][m].push_back(round(this->N[y][m][sig].getSol()));

	solution->setVessels(N);

	vector<vector<int>> P(getData()->M, vector<int>());

	for (int m = 0; m < getData()->M; ++m)
		for (int i = 0; i < getData()->Ip; ++i)
			P[m].push_back(round(this->P[m][i].getSol()));

	solution->setPlanned(P);
	
	vector<vector<vector<int>>> R(getData()->M, vector<vector<int>>(getData()->Ir, vector<int>()));

	for (int m = 0; m < getData()->M; ++m)
		for (int i = 0; i < getData()->Ir; ++i)
			for (int sig = 0; sig < getData()->S; ++sig)
				R[m][i].push_back(round(this->R[m][i][sig].getSol()));

	solution->setReactive(R);

	return solution;
}

void YearModel::genProblem()
{
	genDecVars();
	genObj();

	genCapacityCon();
	genRepairCon();
	genMaxMaintCon();
	genMinMaintCon();
	genAvailableCon();
}

void YearModel::genDecVars()
{
	for (int m = 0; m < getData()->M; ++m)
	{
		for (int sig = 0; sig < getData()->S; ++sig)
		{
			for (int y = 0; y < getData()->Y; ++y)
				N[y][m][sig] = p.newVar(("N_" + to_string(y) + "_" + to_string(m) + to_string(sig)).c_str(), XPRB_UI);

			for (int ir = 0; ir < getData()->Ir; ++ir)
				R[m][ir][sig] = p.newVar(("R_" + to_string(m) + "_" + to_string(ir) + to_string(sig)).c_str(), XPRB_UI);
		}

		for (int ip = 0; ip < getData()->Ip; ++ip)
			P[m][ip] = p.newVar(("P_" + to_string(m) + "_" + to_string(ip)).c_str(), XPRB_UI);
	}
}

void YearModel::genObj()
{
	double sFac = 1.0 / getData()->S; 
	XPRBctr Obj = p.newCtr();

	for (int sig = 0; sig < getData()->S; ++sig)
		for (int m = 0; m < getData()->M; ++m)
		{
			for (int y = 0; y < getData()->Y; ++y)
				Obj.addTerm(N[y][m][sig], getData()->c[y][m] * sFac);

			for (int ip = 0; ip < getData()->Ip; ++ip)
				Obj.addTerm(P[m][ip], getData()->dP * getData()->eH[m] * sFac);

			for (int ir = 0; ir < getData()->Ir; ++ir)
				for (int m_ = 0; m_ <= m; ++m_)
				{
					Obj.addTerm(R[m_][ir][sig], -1 * getData()->eH[m] * getData()->H[m] * sFac);
					Obj.add(getData()->f[m_][ir][sig] * getData()->eH[m] * getData()->H[m] * sFac);
				}
		}
	p.setObj(Obj);
}

void YearModel::genCapacityCon()
{
	for (int sig = 0; sig < getData()->S; ++sig)
		for (int m = 0; m < getData()->M; ++m)
			for (int y = 0; y < getData()->Y; ++y)
			{
				XPRBrelation ctr = getData()->L[y] * N[y][m][sig] + getData()->LInst[y][m] >= 0;

				for (int ip = 0; ip < getData()->Ip; ++ip)
					ctr.addTerm(P[m][ip], -1 * getData()->dPy[y]);

				for (int ir = 0; ir < getData()->Ir; ++ir)
					ctr.addTerm(R[m][ir][sig], -1 * getData()->dR[y][ir]);

				p.newCtr(("Cap_" + to_string(sig) + "_" + to_string(m) + "_" + to_string(y)).c_str(), ctr);
			}
}

void YearModel::genRepairCon()
{
	for (int sig = 0; sig < getData()->S; ++sig)
		for (int m = 0; m < getData()->M; ++m)
			for (int ir = 0; ir < getData()->Ir; ++ir)
			{
				XPRBrelation ctr = R[m][ir][sig] <= 0;

				for (int m_ = 0; m_ <= m - 1; ++m_) 
				{
					ctr.add(-1 * getData()->f[m_][ir][sig]);
					ctr.addTerm(R[m_][ir][sig]);
				}

				p.newCtr(("Rep_" + to_string(sig) + "_" + to_string(m) + "_" + to_string(ir)).c_str(), ctr);
			}
}

void YearModel::genMaxMaintCon()
{
	for (int m = 0; m < getData()->M; ++m)
		for (int ip = 0; ip < getData()->Ip; ++ip)
		{
			XPRBrelation ctr = P[m][ip] <= 0;

			for (int m_ = 0; m_ <= m - getData()->Gmin; ++m_)
				if (ip >= 1)
					ctr.addTerm(P[m_][ip - 1], -1);
				else
					ctr.add(-1 * getData()->Turbs[m_]);

			for (int m_ = 0; m_ <= m - 1; ++m_)
				ctr.addTerm(P[m_][ip]);

			p.newCtr(("MaxM_" + to_string(m) + "_" + to_string(ip)).c_str(), ctr);
		}
}

void YearModel::genMinMaintCon()
{
	for (int m = 0; m < getData()->M; ++m)
		for (int ip = 0; ip < getData()->Ip; ++ip)
		{
			XPRBrelation ctr = P[m][ip] >= 0;

			for (int m_ = 0; m_ <= m - getData()->Gmax; ++m_)
				if (ip >= 1)
					ctr.addTerm(P[m_][ip - 1], -1);
				else
					ctr.add(-1 * getData()->Turbs[m_]);

			for (int m_ = 0; m_ <= m - 1; ++m_)
				ctr.addTerm(P[m_][ip]);

			p.newCtr(("MinM_" + to_string(m) + "_" + to_string(ip)).c_str(), ctr);
		}
}

void YearModel::genAvailableCon()
{
	for (int sig = 0; sig < getData()->S; ++sig)
		for (int m = 0; m < getData()->M; ++m)
			for (int y = 0; y < getData()->Y; ++y)
			{
				XPRBrelation ctr = N[y][m][sig] <= getData()->A[y][m] - getData()->NInst[y][m];

				p.newCtr(("Avai_" + to_string(sig) + "_" + to_string(m) + "_" + to_string(y)).c_str(), ctr);
			}
}

YearModel::YearModel(YearData* data, Mode* mode) : Model(data, mode, "Year")
{
	N = vector<vector<vector<XPRBvar>>>(data->Y, vector<vector<XPRBvar>>(data->M, vector<XPRBvar>(data->S)));
	P = vector<vector<XPRBvar>>(data->M, vector<XPRBvar>(data->Ip));
	R = vector<vector<vector<XPRBvar>>>(data->M, vector<vector<XPRBvar>>(data->Ir, vector<XPRBvar>(data->S)));
}

YearSolution* YearModel::solve()
{
	double dur = solveBasics();

	return genSolution(&p, dur);
}
