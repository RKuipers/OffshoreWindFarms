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
	
	vector<vector<vector<int>>> F(getData()->M, vector<vector<int>>(getData()->Ir, vector<int>()));

	for (int m = 0; m < getData()->M; ++m)
		for (int ir = 0; ir < getData()->Ir; ++ir)
			for (int sig = 0; sig < getData()->S; ++sig)
			{
				int leftover = 0;

				int fuO = 0;
				if (m > 0)
					fuO = round(this->FU[m - 1][ir][sig].getSol());
				int fuN = round(this->FU[m][ir][sig].getSol());
				int r = round(this->R[m][ir][sig].getSol());
				int ft = getData()->Ft[m][ir][sig];

				if (m > 0)
					leftover = round(this->FU[m-1][ir][sig].getSol()) - round(this->R[m][ir][sig].getSol());

				if (ft + leftover - fuN < 0)
					ft = ft;

				F[m][ir].push_back(getData()->Ft[m][ir][sig] + leftover - round(this->FU[m][ir][sig].getSol()));

				if (ir == 1)
				{
					cout << m << ": " << ft << ", " << fuN << ", " << r << ", " << (ft + leftover - fuN) << ", " << leftover << endl;
				}
			}

	solution->setReactive(F);

	return solution;
}

void YearModel::genProblem()
{
	genDecVars();
	genObj();

	genCapacityCon();
	genResourceCon();
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
				N[y][m][sig] = p.newVar(("N_" + to_string(y) + "_" + to_string(m) + "_" + to_string(sig)).c_str(), XPRB_UI);

			for (int ir = 0; ir < getData()->Ir; ++ir)
			{
				FU[m][ir][sig] = p.newVar(("FU_" + to_string(m) + "_" + to_string(ir) + "_" + to_string(sig)).c_str(), XPRB_UI);
				R[m][ir][sig] = p.newVar(("R_" + to_string(m) + "_" + to_string(ir) + "_" + to_string(sig)).c_str(), XPRB_UI);
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
				Obj.addTerm(FU[m][ir][sig], 0.5 * getData()->H[m] * getData()->eH[m] * sFac);
				Obj.add(getData()->Ft[m][ir][sig] * (getData()->dR[ir] + getData()->dD[ir]) * getData()->eH[m] * sFac);
				Obj.addTerm(FU[m][ir][sig], -1 * (getData()->dR[ir] + getData()->dD[ir]) * getData()->eH[m] * sFac);
				Obj.addTerm(R[m][ir][sig], getData()->dR[ir] * getData()->eH[m] * sFac);

				if (m > 0)
				{
					Obj.addTerm(FU[m - 1][ir][sig], getData()->H[m] * getData()->eH[m] * sFac);
					Obj.addTerm(R[m][ir][sig], -1 * getData()->H[m] * getData()->eH[m] * sFac);
				}
			}
		}

		for (int ir = 0; ir < getData()->Ir; ++ir)
			Obj.addTerm(FU[getData()->M - 1][ir][sig], getData()->lambda[ir]);
	}
	p.setObj(Obj);
}

void YearModel::genCapacityCon()
{
	for (int sig = 0; sig < getData()->S; ++sig)
		for (int m = 0; m < getData()->M; ++m)
			for (int y = 0; y < getData()->Y; ++y)
			{
				XPRBrelation ctr = getData()->L[y] * N[y][m][sig] + getData()->LInst[y][m] - getData()->eps[sig][m][y] >= 0;

				for (int ip = 0; ip < getData()->Ip; ++ip)
					ctr.addTerm(P[m][ip], -1 * getData()->dPy[y]);

				for (int ir = 0; ir < getData()->Ir; ++ir)
				{
					if (m >= 1)
						ctr.addTerm(FU[m-1][ir][sig], -1 * getData()->dRy[y][ir]);
					ctr.addTerm(FU[m][ir][sig], getData()->dRy[y][ir]);
					ctr.add(getData()->Ft[m][ir][sig] * getData()->dRy[y][ir] * -1);
				}

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

				XPRBrelation ctr = N[y][m][sig] >= getData()->rho[sig][m][y] - getData()->NInst[y][m];

				p.newCtr(("Res_" + to_string(sig) + "_" + to_string(m) + "_" + to_string(y)).c_str(), ctr);
			}
}

void YearModel::genRepairCon()
{
	for (int sig = 0; sig < getData()->S; ++sig)
		for (int ir = 0; ir < getData()->Ir; ++ir)
		{
			FU[0][ir][sig].setUB(getData()->Ft[0][ir][sig]);
			R[0][ir][sig].fix(0);

			for (int m = 1; m < getData()->M; ++m)
			{
				p.newCtr(("RepL_" + to_string(sig) + "_" + to_string(m) + "_" + to_string(ir)).c_str(), R[m][ir][sig] >= FU[m - 1][ir][sig] - FU[m][ir][sig]);
				p.newCtr(("RepU_" + to_string(sig) + "_" + to_string(m) + "_" + to_string(ir)).c_str(), R[m][ir][sig] <= FU[m - 1][ir][sig]);
				p.newCtr(("UnhU_" + to_string(sig) + "_" + to_string(m) + "_" + to_string(ir)).c_str(), FU[m][ir][sig] <= getData()->Ft[m][ir][sig] + FU[m - 1][ir][sig]);
			}
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
	FU = vector<vector<vector<XPRBvar>>>(data->M, vector<vector<XPRBvar>>(data->Ir, vector<XPRBvar>(data->S)));
	R = vector<vector<vector<XPRBvar>>>(data->M, vector<vector<XPRBvar>>(data->Ir, vector<XPRBvar>(data->S)));
}

YearSolution* YearModel::solve()
{
	double dur = solveBasics();

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
	double vr = 0.0, pd = 0.0, re = 0.0, uf = 0.0, lo = 0.0;

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
		{
			// Unhandled failures		keep
			unhandledFailures[m] += FU[m][ir][sig].getSol() * getData()->H[m] * 0.5 * e;

			// Old unhandled failures	keep
			if (m > 0)
				unhandledFailures[m] += (FU[m - 1][ir][sig].getSol() - solution->getRepairs()[sig][m][ir]) * getData()->H[m] * e;
		}

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
		lo += FU[getData()->M - 1][ir][sig].getSol() * getData()->lambda[ir];
	res += lo;

	cout << endl;
	cout << "Total: " << res << endl;
	cout << "Totals per category:" << endl;
	cout << "Vessel rentals: " << vr << endl;
	cout << "Planned downtime: " << pd << endl;
	cout << "Repairs: " << re << endl;
	cout << "Unhandled failures: " << uf << endl;
	cout << "Leftover failures: " << lo << endl;
	cout << endl;

	return res;
}
