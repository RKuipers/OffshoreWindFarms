#include "YearModel.h"

YearData* YearModel::getData()
{
	return data->getYear();
}

YearSolution* YearModel::genSolution(XPRBprob* p, double duration)
{
	solution = new YearSolution(mode->getCurrentName(), mode->getCurrentId(), getData());
	solution->setResult(p->getObjVal(), duration);

	vector<vector<int>> N(getData()->Y, vector<int>());

	for (int y = 0; y < getData()->Y; ++y)
		for (int m = 0; m < getData()->M; ++m)
			N[y].push_back(round(this->N[y][m].getSol()));

	solution->setVessels(N);

	vector<vector<int>> P(getData()->M, vector<int>());

	for (int m = 0; m < getData()->M; ++m)
		for (int ip = 0; ip < getData()->Ip; ++ip)
			P[m].push_back(round(this->P[m][ip].getSol()));

	solution->setPlanned(P);

	vector<vector<vector<int>>> R(getData()->M, vector<vector<int>>(getData()->Ir, vector<int>()));

	for (int m = 0; m < getData()->M; ++m)
		for (int ir = 0; ir < getData()->Ir; ++ir)
			for (int sig = 0; sig < getData()->S; ++sig)
				R[m][ir].push_back(round(this->R[m][ir][sig].getSol()));

	solution->setRepairs(R);
	
	vector<vector<vector<int>>> U(getData()->M, vector<vector<int>>(getData()->Ir, vector<int>()));

	for (int m = 0; m < getData()->M; ++m)
		for (int ir = 0; ir < getData()->Ir; ++ir)
			for (int sig = 0; sig < getData()->S; ++sig)
				U[m][ir].push_back(round(this->U[m][ir][sig].getSol()));

	solution->setUnhandled(U);

	return solution;
}

void YearModel::genProblem()
{
	genDecVars();
	genObj();

	genCapacityCon();
	genResourceCon();
	genRhoCon();
	genRepairCon();
	genRegMaintCon();
}

void YearModel::genDecVars()
{
	for (int m = 0; m < getData()->M; ++m)
	{
		for (int sig = 0; sig < getData()->S; ++sig)for (int ir = 0; ir < getData()->Ir; ++ir)
			{
				R[m][ir][sig] = p.newVar(("R_" + to_string(m) + "_" + to_string(ir) + "_" + to_string(sig)).c_str(), XPRB_UI);
				U[m][ir][sig] = p.newVar(("U_" + to_string(m) + "_" + to_string(ir) + "_" + to_string(sig)).c_str(), XPRB_UI);
			}

		for (int y = 0; y < getData()->Y; ++y)
		{
			N[y][m] = p.newVar(("N_" + to_string(y) + "_" + to_string(m)).c_str(), XPRB_UI);

			Mp[y][m] = p.newVar(("Mp_" + to_string(y) + "_" + to_string(m)).c_str(), XPRB_UI);
			for (int ir = 0; ir < getData()->Ir; ++ir)
				Mr[y][m][ir] = p.newVar(("Mr_" + to_string(y) + "_" + to_string(m) + "_" + to_string(ir)).c_str(), XPRB_UI);
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
	{
		for (int m = 0; m < getData()->M; ++m)
		{
			for (int y = 0; y < getData()->Y; ++y)
				Obj.addTerm(N[y][m], getData()->cV[y][m] * sFac);

			for (int ip = 0; ip < getData()->Ip; ++ip)
			{
				Obj.addTerm(P[m][ip], getData()->dP * getData()->eH[m] * sFac);
				Obj.addTerm(P[m][ip], getData()->cP * sFac);
			}

			for (int ir = 0; ir < getData()->Ir; ++ir)
			{
				Obj.addTerm(R[m][ir][sig], (getData()->dR[ir] + getData()->dD[ir]) * getData()->eH[m] * sFac);
				Obj.addTerm(U[m][ir][sig], getData()->H[m] * getData()->eH[m] * sFac);
				Obj.addTerm(R[m][ir][sig], getData()->cR[ir] * sFac);
			}
		}

		for (int ir = 0; ir < getData()->Ir; ++ir)
			Obj.addTerm(U[getData()->M - 1][ir][sig], getData()->lambda[ir] * sFac);
	}
	p.setObj(Obj);
}

void YearModel::genCapacityCon()
{
	for (int sig = 0; sig < getData()->S; ++sig)
		for (int m = 0; m < getData()->M; ++m)
			for (int y = 0; y < getData()->Y; ++y)
			{
				XPRBrelation ctr = getData()->L[y] * N[y][m] + getData()->LInst[y][m] >= 0;

				for (int ip = 0; ip < getData()->Ip; ++ip)
					ctr.addTerm(P[m][ip], -1 * getData()->dPy[y] * getData()->rhoP[y]);

				for (int ir = 0; ir < getData()->Ir; ++ir)
					ctr.addTerm(R[m][ir][sig], -1 * getData()->dRy[y][ir] * getData()->rhoR[y][ir]);

				p.newCtr(("Cap_" + to_string(sig) + "_" + to_string(m) + "_" + to_string(y)).c_str(), ctr);
			}
}

void YearModel::genResourceCon()
{
	for (int m = 0; m < getData()->M; ++m)
		for (int y = 0; y < getData()->Y; ++y)
		{
			N[y][m].setUB(getData()->A[y][m]);

			//XPRBrelation ctr = N[y][m][sig] >= getData()->rho[sig][m][y] - getData()->NInst[y][m];

			//p.newCtr(("Res_" + to_string(sig) + "_" + to_string(m) + "_" + to_string(y)).c_str(), ctr);
		}
}

void YearModel::genRhoCon()
{
	for (int m = 0; m < getData()->M; ++m)
		for (int y = 0; y < getData()->Y; ++y)
		{
			XPRBrelation ctrL = (N[y][m] + getData()->NInst[y][m]) * (1.0 / (float)getData()->rhoP[y]) >= Mp[y][m];
			XPRBrelation ctrR = Mp[y][m] >= P[m][0] * (getData()->dPy[y] / (float)getData()->L[y]);

			for (int ip = 1; ip < getData()->Ip; ++ip)
				ctrR.addTerm(P[m][ip], (getData()->dPy[y] / (float)getData()->L[y]));

			if (getData()->rhoP[y] > 0)
				p.newCtr(("RhoPL_" + to_string(m) + "_" + to_string(y)).c_str(), ctrL);
			p.newCtr(("RhoPR_" + to_string(m) + "_" + to_string(y)).c_str(), ctrR);

			for (int ir = 0; ir < getData()->Ir; ++ir)
			{
				XPRBrelation ctrL = (N[y][m] + getData()->NInst[y][m]) * (1.0 / (float)getData()->rhoR[y][ir]) >= Mr[y][m][ir];

				if (getData()->rhoR[y][ir] > 0)
					p.newCtr(("RhoRL_" + to_string(m) + "_" + to_string(y) + "_" + to_string(ir)).c_str(), ctrL);

				for (int sig = 0; sig < getData()->S; ++sig)
				{
					XPRBrelation ctrR = Mr[y][m][ir] >= R[m][ir][sig] * (getData()->dRy[y][ir] / (float)getData()->L[y]);

					p.newCtr(("RhoRR_" + to_string(sig) + "_" + to_string(m) + "_" + to_string(y) + "_" + to_string(ir)).c_str(), ctrR);
				}
			}
		}
}

void YearModel::genRepairCon()
{
	for (int sig = 0; sig < getData()->S; ++sig)
		for (int ir = 0; ir < getData()->Ir; ++ir)
		{
			p.newCtr(("Rep_" + to_string(sig) + "_0_" + to_string(ir)).c_str(), getData()->Ft[0][ir][sig] == R[0][ir][sig] + U[0][ir][sig]);

			for (int m = 1; m < getData()->M; ++m)
				p.newCtr(("Rep_" + to_string(sig) + "_" + to_string(m) + "_" + to_string(ir)).c_str(), getData()->Ft[m][ir][sig] + U[m-1][ir][sig] == R[m][ir][sig] + U[m][ir][sig]);
		}
}

void YearModel::genRegMaintCon()
{
	for (int m = 0; m < getData()->M; ++m)
		for (int ip = 0; ip < getData()->Ip; ++ip)
		{
			XPRBrelation ctrL = P[m][ip] >= 0;
			XPRBrelation ctrU = P[m][ip] <= 0;

			for (int m_ = 0; m_ <= m - 1; ++m_)
			{
				ctrL.addTerm(P[m_][ip]);
				ctrU.addTerm(P[m_][ip]);

				if (ip >= 1)
				{
					if (m_ <= m - getData()->GU)
						ctrL.addTerm(P[m_][ip - 1], -1);
					if (m_ <= m - getData()->GL)
						ctrU.addTerm(P[m_][ip - 1], -1);
				}
				else
				{
					if (m_ <= m - getData()->GU)
						ctrL.add(-1 * getData()->Turbs[m_]);
					if (m_ <= m - getData()->GL)
						ctrU.add(-1 * getData()->Turbs[m_]);
				}
				
			}

			p.newCtr(("MaxM_" + to_string(m) + "_" + to_string(ip)).c_str(), ctrU);
			p.newCtr(("MinM_" + to_string(m) + "_" + to_string(ip)).c_str(), ctrL);
		}
}

YearModel::YearModel(YearData* data, Mode* mode, string name) : Model(data, mode, name)
{
	N = vector<vector<XPRBvar>>(data->Y, vector<XPRBvar>(data->M));
	P = vector<vector<XPRBvar>>(data->M, vector<XPRBvar>(data->Ip));
	R = vector<vector<vector<XPRBvar>>>(data->M, vector<vector<XPRBvar>>(data->Ir, vector<XPRBvar>(data->S)));
	U = vector<vector<vector<XPRBvar>>>(data->M, vector<vector<XPRBvar>>(data->Ir, vector<XPRBvar>(data->S)));
	Mp = vector<vector<XPRBvar>>(data->Y, vector<XPRBvar>(data->M));
	Mr = vector<vector<vector<XPRBvar>>>(data->Y, vector<vector<XPRBvar>>(data->M, vector<XPRBvar>(data->Ir)));
}

YearSolution* YearModel::solve(int maxTime)
{
	double dur = solveBasics(maxTime);

	return genSolution(&p, dur);
}
