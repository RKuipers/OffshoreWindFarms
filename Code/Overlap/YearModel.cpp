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
				Obj.addTerm(N[y][m], getData()->c[y][m] * sFac);

			for (int ip = 0; ip < getData()->Ip; ++ip)
				Obj.addTerm(P[m][ip], getData()->dP * getData()->eH[m] * sFac);

			for (int ir = 0; ir < getData()->Ir; ++ir)
			{
				Obj.addTerm(R[m][ir][sig], (getData()->dR[ir] + getData()->dD[ir]) * getData()->eH[m] * sFac);
				Obj.addTerm(U[m][ir][sig], getData()->H[m] * getData()->eH[m] * sFac);
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

YearModel::YearModel(YearData* data, Mode* mode) : Model(data, mode, "Year")
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

double YearModel::printCostBreakdown(vector<MonthSolution*> months)
{
	double res = 0.0;
	int nSigs = getData()->S;
	int nMonths = getData()->M;

	vector<double> sigWeight = vector<double>(nSigs, 1.0 / (float)nSigs);
	vector<double> sigTotals = vector<double>(nSigs, 0.0);

	bool mixed = true;
	if (months.size() < getData()->M)
		mixed = false;

	// Scenario dependant
	vector<double> vesselRental = vector<double>(nMonths, 0.0);
	vector<double> technicians = vector<double>(nMonths, 0.0);
	vector<double> plannedDowntime = vector<double>(nMonths, 0.0);
	double vr = 0.0;
	double te = 0.0;
	double pd = 0.0;

	// Scenario independant
	vector<vector<double>> repairs = vector<vector<double>>(nSigs, vector<double>(nMonths, 0.0));
	vector<vector<double>> repairsExpected = vector<vector<double>>(nSigs, vector<double>(nMonths, 0.0));
	vector<vector<double>> unhandledFailures = vector<vector<double>>(nSigs, vector<double>(nMonths, 0.0));
	vector<double> re = vector<double>(nSigs, 0.0);
	vector<double> reexp = vector<double>(nSigs, 0.0);
	vector<double> uf = vector<double>(nSigs, 0.0);
	vector<double> lo = vector<double>(nSigs, 0.0);

	for (int m = 0; m < getData()->M; ++m)
	{
		double e = getData()->eH[m];

		// Vessel Rentals
		for (int y = 0; y < getData()->Y-1; ++y)
			vesselRental[m] += solution->getVessels()[m][y] * getData()->c[y][m];
		vr += vesselRental[m];
		
		// Technicians
		int techId = getData()->Y - 1;
		technicians[m] = solution->getVessels()[m][techId] * getData()->c[techId][m];
		te += technicians[m];

		// Downtime planned
		for (int ip = 0; ip < getData()->Ip; ++ip)
			plannedDowntime[m] += solution->getPlanned()[m][ip] * getData()->dP * e;
		pd += plannedDowntime[m];
	}

	for (int sig = 0; sig < nSigs; ++sig)
	{
		double scenTotal = 0.0;

		cout << "SCENARIO " << sig << endl;
		cout << "Costs per month:" << endl;
		cout << "Month: Total (Vessel rentals + Technicians + Planned downtime + Repairs + Unhandled failures)" << endl;

		for (int m = 0; m < getData()->M; ++m)
		{
			double e = getData()->eH[m];

			// Task costs
			if (mixed && months[m] != nullptr)
				repairs[sig][m] += months[m]->getObj();
			for (int ir = 0; ir < getData()->Ir; ++ir)
				repairsExpected[sig][m] += solution->getRepairs()[sig][m][ir] * (getData()->dR[ir] + getData()->dD[ir]) * e;
			if (!mixed)
				repairs[sig][m] = repairsExpected[sig][m];

			// Unhandled failures
			for (int ir = 0; ir < getData()->Ir; ++ir)
				unhandledFailures[sig][m] += solution->getUnhandled()[sig][m][ir] * getData()->H[m] * e;

			double total = vesselRental[m] + technicians[m] + plannedDowntime[m] + repairs[sig][m] + unhandledFailures[sig][m];
			re[sig] += repairs[sig][m];
			reexp[sig] += repairsExpected[sig][m];
			uf[sig] += unhandledFailures[sig][m];

			cout << m << ": " << total << " (" << vesselRental[m] << " + " << technicians[m] << " + " << plannedDowntime[m] << " + " << repairs[sig][m] << " + " << unhandledFailures[sig][m] << ")" << endl;

			scenTotal += total;
		}

		// Last month penalty
		for (int ir = 0; ir < getData()->Ir; ++ir)
			lo[sig] += U[getData()->M - 1][ir][sig].getSol() * getData()->lambda[ir];
		scenTotal += lo[sig];

		sigTotals[sig] = scenTotal;
		res += sigTotals[sig] * sigWeight[sig];

		cout << endl;
		cout << "Total: " << scenTotal << endl;
		cout << "Totals per category:" << endl;
		cout << "Vessel rentals: " << vr << endl;
		cout << "Technician costs: " << te << endl;
		cout << "Planned downtime: " << pd << endl;
		if (mixed)
			cout << "Repairs: " << re[sig] << endl;
		cout << "Expected repairs: " << reexp[sig] << endl;
		cout << "Unhandled failures: " << uf[sig] << endl;
		cout << "Leftover failures: " << lo[sig] << endl;
		cout << endl;
	}

	if (nSigs > 1)
	{
		auto it = minmax_element(sigTotals.begin(), sigTotals.end());
		int minId = distance(sigTotals.begin(), it.first);
		double min = *it.first;
		int maxId = distance(sigTotals.begin(), it.second);
		double max = *it.second;

		cout << endl;
		cout << "-----------SUMMARY-----------" << endl;
		cout << "Mean: " << MathHelp::Mean(&sigTotals) << endl;
		cout << "Median: " << MathHelp::Median(&sigTotals) << endl;
		cout << "Cheapest: " << minId << " costing " << min << endl;
		cout << "Most expensive: " << maxId << " costing " << max << endl;
		cout << endl; 
		cout << "Vessel rentals: " << vr << endl;
		cout << "Technician costs: " << te << endl;
		cout << "Planned downtime: " << pd << endl;
		if (mixed)
			cout << "Repairs: " << MathHelp::Mean(&re) << endl;
		cout << "Expected repairs: " << MathHelp::Mean(&reexp) << endl;
		cout << "Unhandled failures: " << MathHelp::Mean(&uf) << endl;
		cout << "Leftover failures: " << MathHelp::Mean(&lo) << endl;
		cout << endl;
	}

	return res;
}
