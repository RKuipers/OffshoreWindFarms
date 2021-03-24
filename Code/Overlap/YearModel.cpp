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
		for (int sig = 0; sig < getData()->S; ++sig)
		{
			for (int y = 0; y < getData()->Y; ++y)
			{
				N[y][m][sig] = p.newVar(("N_" + to_string(y) + "_" + to_string(m) + "_" + to_string(sig)).c_str(), XPRB_UI);

				Mp[y][m][sig] = p.newVar(("Mp_" + to_string(y) + "_" + to_string(m) + "_" + to_string(sig)).c_str(), XPRB_UI);
				for (int ir = 0; ir < getData()->Ir; ++ir)
					Mr[y][m][sig][ir] = p.newVar(("Mr_" + to_string(y) + "_" + to_string(m) + "_" + to_string(sig) + "_" + to_string(ir)).c_str(), XPRB_UI);
			}

			for (int ir = 0; ir < getData()->Ir; ++ir)
			{
				R[m][ir][sig] = p.newVar(("R_" + to_string(m) + "_" + to_string(ir) + "_" + to_string(sig)).c_str(), XPRB_UI);
				U[m][ir][sig] = p.newVar(("U_" + to_string(m) + "_" + to_string(ir) + "_" + to_string(sig)).c_str(), XPRB_UI);
			}
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
				Obj.addTerm(N[y][m][sig], getData()->c[y][m] * sFac);

			for (int ip = 0; ip < getData()->Ip; ++ip)
				Obj.addTerm(P[m][ip], getData()->dP * getData()->eH[m] * sFac);

			for (int ir = 0; ir < getData()->Ir; ++ir)
			{
				Obj.addTerm(R[m][ir][sig], (getData()->dR[ir] + getData()->dD[ir]) * getData()->eH[m] * sFac);
				Obj.addTerm(U[m][ir][sig], getData()->H[m] * getData()->eH[m] * sFac);
			}
		}

		for (int ir = 0; ir < getData()->Ir; ++ir)
			Obj.addTerm(U[getData()->M - 1][ir][sig], getData()->lambda[ir]);
	}
	p.setObj(Obj);
}

void YearModel::genCapacityCon()
{
	for (int sig = 0; sig < getData()->S; ++sig)
		for (int m = 0; m < getData()->M; ++m)
			for (int y = 0; y < getData()->Y; ++y)
			{
				XPRBrelation ctr = getData()->L[y] * N[y][m][sig] + getData()->LInst[y][m] >= getData()->eps[sig][m][y];

				for (int ip = 0; ip < getData()->Ip; ++ip)
					ctr.addTerm(P[m][ip], -1 * getData()->dPy[y]);

				for (int ir = 0; ir < getData()->Ir; ++ir)
					ctr.addTerm(R[m][ir][sig], -1 * getData()->dRy[y][ir]);

				p.newCtr(("Cap_" + to_string(sig) + "_" + to_string(m) + "_" + to_string(y)).c_str(), ctr);
			}
}

void YearModel::genResourceCon()
{
	for (int sig = 0; sig < getData()->S; ++sig)
		for (int m = 0; m < getData()->M; ++m)
			for (int y = 0; y < getData()->Y; ++y)
			{
				N[y][m][sig].setUB(getData()->A[y][m]);

				//XPRBrelation ctr = N[y][m][sig] >= getData()->rho[sig][m][y] - getData()->NInst[y][m];

				//p.newCtr(("Res_" + to_string(sig) + "_" + to_string(m) + "_" + to_string(y)).c_str(), ctr);
			}
}

void YearModel::genRhoCon()
{
	for (int sig = 0; sig < getData()->S; ++sig)
		for (int m = 0; m < getData()->M; ++m)
			for (int y = 0; y < getData()->Y; ++y)
			{
				XPRBrelation ctrL = N[y][m][sig] * (1.0 / (float)getData()->rhoP[y]) >= Mp[y][m][sig];
				XPRBrelation ctrR = Mp[y][m][sig] >= P[m][0] * (getData()->dPy[y] / (float)getData()->L[y]);

				for (int ip = 1; ip < getData()->Ip; ++ip)
					ctrR.addTerm(P[m][ip], (getData()->dPy[y] / (float)getData()->L[y]));

				p.newCtr(("RhoPL_" + to_string(sig) + "_" + to_string(m) + "_" + to_string(y)).c_str(), ctrL);
				p.newCtr(("RhoPR_" + to_string(sig) + "_" + to_string(m) + "_" + to_string(y)).c_str(), ctrR);

				for (int ir = 0; ir < getData()->Ir; ++ir)
				{
					XPRBrelation ctrL = N[y][m][sig] * (1.0 / (float)getData()->rhoR[y][ir]) >= Mr[y][m][sig][ir];
					XPRBrelation ctrR = Mr[y][m][sig][ir] >= R[m][ir][sig] * (getData()->dRy[y][ir] / (float)getData()->L[y]);

					p.newCtr(("RhoRL_" + to_string(sig) + "_" + to_string(m) + "_" + to_string(y) + "_" + to_string(ir)).c_str(), ctrL);
					p.newCtr(("RhoRR_" + to_string(sig) + "_" + to_string(m) + "_" + to_string(y) + "_" + to_string(ir)).c_str(), ctrR);
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

YearModel::YearModel(YearData* data, Mode* mode) : Model(data, mode, "Year")
{
	N = vector<vector<vector<XPRBvar>>>(data->Y, vector<vector<XPRBvar>>(data->M, vector<XPRBvar>(data->S)));
	P = vector<vector<XPRBvar>>(data->M, vector<XPRBvar>(data->Ip));
	R = vector<vector<vector<XPRBvar>>>(data->M, vector<vector<XPRBvar>>(data->Ir, vector<XPRBvar>(data->S)));
	U = vector<vector<vector<XPRBvar>>>(data->M, vector<vector<XPRBvar>>(data->Ir, vector<XPRBvar>(data->S)));
	Mp = vector<vector<vector<XPRBvar>>>(data->Y, vector<vector<XPRBvar>>(data->M, vector<XPRBvar>(data->S)));
	Mr = vector<vector<vector<vector<XPRBvar>>>>(data->Y, vector<vector<vector<XPRBvar>>>(data->M, vector<vector<XPRBvar>>(data->S, vector<XPRBvar>(data->Ir))));
}

YearSolution* YearModel::solve(int maxTime)
{
	double dur = solveBasics(maxTime);

	return genSolution(&p, dur);
}

double YearModel::printMixedValue(vector<MonthSolution*> months)
{
	cout << "Costs per month:" << endl;
	cout << "Month: Total (Vessel rentals + Planned downtime + Repairs + Unhandled failures)" << endl;

	double res = 0.0;
	int sig = 0;

	vector<double> vesselRental = vector<double>(getData()->M, 0.0);
	vector<double> plannedDowntime = vector<double>(getData()->M, 0.0);
	vector<double> repairs = vector<double>(getData()->M, 0.0);
	vector<double> unhandledFailures = vector<double>(getData()->M, 0.0);
	double vr = 0.0, pd = 0.0, re = 0.0, reexp = 0.0, uf = 0.0, lo = 0.0;

	for (int m = 0; m < getData()->M; ++m)
	{
		double e = getData()->eH[m];

		// Vessel Rentals
		for (int y = 0; y < getData()->Y; ++y)
			vesselRental[m] += solution->getVessels()[sig][m][y] * getData()->c[y][m];
		// Downtime planned
		for (int ip = 0; ip < getData()->Ip; ++ip)
			plannedDowntime[m] += solution->getPlanned()[m][ip] * getData()->dP * e;

		// Task costs
		if (months[m] != nullptr)
			repairs[m] += months[m]->getObj();
		for (int ir = 0; ir < getData()->Ir; ++ir)
			reexp += (getData()->dR[ir] + getData()->dD[ir]) * e;

		// Unhandled failures
		for (int ir = 0; ir < getData()->Ir; ++ir)
			unhandledFailures[m] += U[m][ir][sig].getSol() * getData()->H[m] * 0.5 * e;

		double total = vesselRental[m] + plannedDowntime[m] + repairs[m] + unhandledFailures[m];
		vr += vesselRental[m];
		pd += plannedDowntime[m];
		re += repairs[m];
		uf += unhandledFailures[m];

		cout << m << ": " << total << " (" << vesselRental[m] << " + " << plannedDowntime[m] << " + " << repairs[m] << " + " << unhandledFailures[m] << ")" << endl;

		res += total;
	}

	// Last month penalty
	for (int ir = 0; ir < getData()->Ir; ++ir)
		lo += U[getData()->M - 1][ir][sig].getSol() * getData()->lambda[ir];
	res += lo;

	cout << endl;
	cout << "Total: " << res << endl;
	cout << "Totals per category:" << endl;
	cout << "Vessel rentals: " << vr << endl;
	cout << "Planned downtime: " << pd << endl;
	cout << "Repairs: " << re << endl;
	cout << "Expected repairs: " << reexp << endl;
	cout << "Unhandled failures: " << uf << endl;
	cout << "Leftover failures: " << lo << endl;
	cout << endl;

	return res;
}
