#include "Solution.h"

//-----------------------------------------------BASE----------------------------------------------

Solution::Solution(string name, int id) : modeName(name), modeId(id) 
{ 
	duration = -1;
}

void Solution::setResult(double value, double duration)
{
	this->value = value;
	this->duration = duration;
}

double Solution::getObj()
{
	return value;
}

void Solution::printObj()
{
	cout << "Objective value: " << value << endl;
}

void Solution::printDur()
{
	cout << "Duration: " << duration << endl;
}

//-----------------------------------------------YEAR----------------------------------------------

YearSolution::YearSolution(string name, int id, YearData* data) : Solution(name, id), data(data) { }

void YearSolution::setVessels(vector<vector<int>> N)
{
	int Y = N.size();
	int M = N[0].size();

	vessels = vector<vector<int>>(M, vector<int>(Y, -1));

	for (int m = 0; m < M; ++m)
		for (int y = 0; y < Y; ++y)
			vessels[m][y] = N[y][m];
}

void YearSolution::setPlanned(vector<vector<int>> P)
{
	copy(P.begin(), P.end(), back_inserter(planned));
}

void YearSolution::setRepairs(vector<vector<vector<int>>> R)
{
	int Sig = R[0][0].size();
	int I = R[0].size();
	int M = R.size();

	repairs = vector<vector<vector<int>>>(Sig, vector<vector<int>>(M, vector<int>(I, -1)));

	for (int sig = 0; sig < Sig; ++sig)
		for (int m = 0; m < M; ++m)
			for (int i = 0; i < I; ++i)
				repairs[sig][m][i] = R[m][i][sig];
}

void YearSolution::setUnhandled(vector<vector<vector<int>>> U)
{
	int Sig = U[0][0].size();
	int I = U[0].size();
	int M = U.size();

	unhandled = vector<vector<vector<int>>>(Sig, vector<vector<int>>(M, vector<int>(I, -1)));

	for (int sig = 0; sig < Sig; ++sig)
		for (int m = 0; m < M; ++m)
			for (int i = 0; i < I; ++i)
				unhandled[sig][m][i] = U[m][i][sig];
}

vector<vector<int>> YearSolution::getVessels()
{
	return vessels;
}

vector<vector<int>> YearSolution::getPlanned()
{
	return planned;
}

vector<vector<vector<int>>> YearSolution::getRepairs()
{
	return repairs;
}

vector<vector<vector<int>>> YearSolution::getUnhandled()
{
	return unhandled;
}

void YearSolution::printVessels()
{
	cout << "Vessels chartered (per month V and type >):" << endl;

	for (int m = 0; m < vessels.size(); ++m)
	{
		cout << m << ": " << vessels[m][0];

		for (int y = 1; y < vessels[m].size(); ++y)
			cout << ", " << vessels[m][y];

		cout << endl;
	}
}

void YearSolution::printPlanned()
{
	cout << "Planned tasks (per month V and iteration >):" << endl;

	for (int m = 0; m < planned.size(); ++m)
	{
		cout << m << ": " << planned[m][0];

		for (int i = 1; i < planned[m].size(); ++i)
			cout << ", " << planned[m][i];

		cout << endl;
	}
}

void YearSolution::printFailures()
{
	data->Ft[0][0][0];
	for (int sig = 0; sig < data->S; ++sig)
	{
		cout << "Failures in scenario " << sig << " (per month V and type >):" << endl;

		for (int m = 0; m < data->M; ++m)
		{
			cout << m << ": " << data->Ft[m][0][sig];

			for (int i = 1; i < data->Ir; ++i)
				cout << ", " << data->Ft[m][i][sig];

			cout << endl;
		}
	}
}

void YearSolution::printRepairs()
{
	for (int sig = 0; sig < repairs.size(); ++sig)
	{
		cout << "Repair tasks in scenario " << sig << " (per month V and type >):" << endl;

		for (int m = 0; m < repairs[sig].size(); ++m)
		{
			cout << m << ": " << repairs[sig][m][0];

			for (int i = 1; i < repairs[sig][m].size(); ++i)
				cout << ", " << repairs[sig][m][i];

			cout << endl;
		}
	}
}

void YearSolution::printUnhandled()
{
	for (int sig = 0; sig < unhandled.size(); ++sig)
	{
		cout << "Unhandled tasks in scenario " << sig << " (per month V and type >):" << endl;

		for (int m = 0; m < unhandled[sig].size(); ++m)
		{
			cout << m << ": " << unhandled[sig][m][0];

			for (int i = 1; i < unhandled[sig][m].size(); ++i)
				cout << ", " << unhandled[sig][m][i];

			cout << endl;
		}
	}
}

void YearSolution::printAvailability()
{
	cout << "Availability and production losses (per month V and scenario >): " << endl;

	vector<vector<double>> avails = vector<vector<double>>(data->S, vector<double>(data->M));
	vector<double> availsScen = vector<double>(data->S);
	vector<double> eAvail = vector<double>(data->S);
	vector<vector<double>> losses = vector<vector<double>>(data->S, vector<double>(data->M));
	vector<double> lossesScen = vector<double>(data->S);

	for (int sig = 0; sig < data->S; ++sig)
	{
		int turbs = 0;
		double eProd = 0.0;
		double eMax = 0.0;

		for (int m = 0; m < data->M; ++m)
		{
			if (m == 105)
				m = m;

			// Base time
			turbs += data->Turbs[m];
			double timeAvail = turbs * data->H[m];

			// Planned
			timeAvail -= MathHelp::Sum(&planned[m]) * data->dP;

			for (int ir = 0; ir < data->Ir; ir++)
			{
				// Repairs
				int newRepairs = min(repairs[sig][m][ir], data->Ft[m][ir][sig]);	
				timeAvail -= newRepairs * (data->dR[ir] + data->dD[ir]);			// Repairs that come from new failures
				timeAvail -= (repairs[sig][m][ir] - newRepairs) * data->dR[ir];		// Repairs that come from previously unhandled failures

				// Unhandled
				int partiallyInactive = 0;
				if (m == 0)
					partiallyInactive = unhandled[sig][m][ir];
				else if (unhandled[sig][m][ir] > unhandled[sig][m - 1][ir])
					partiallyInactive = unhandled[sig][m][ir] - unhandled[sig][m - 1][ir];

				timeAvail -= (unhandled[sig][m][ir] - partiallyInactive) * data->H[m];
				for (int f = 0; f < partiallyInactive; ++f)
					timeAvail -= (double)(rand() % data->H[m]);
			}

			eProd += timeAvail * data->eH[m];
			eMax += turbs * data->H[m] * data->eH[m];

			avails[sig][m] = 100 * timeAvail / (turbs * data->H[m]);
			losses[sig][m] = ((turbs * data->H[m]) - timeAvail) * data->eH[m];
		}

		availsScen[sig] = MathHelp::Mean(&avails[sig]);
		eAvail[sig] = 100 * eProd / eMax;
		lossesScen[sig] = MathHelp::Sum(&losses[sig]);
	}

	for (int m = 0; m < data->M; ++m)
	{
		cout << m << ": " << avails[0][m];
		for (int sig = 1; sig < data->S; ++sig)
			cout << ", " << avails[sig][m];
		cout << " / " << losses[0][m];
		for (int sig = 1; sig < data->S; ++sig)
			cout << ", " << losses[sig][m];
		cout << endl;
	}

	cout << "Average time availability: " << MathHelp::Mean(&availsScen) << " (";
	cout << availsScen[0];
	for (int sig = 1; sig < data->S; ++sig)
		cout << ", " << availsScen[sig];
	cout << ")" << endl;
	timeAvail = MathHelp::Mean(&availsScen);

	cout << "Average energy availability: " << MathHelp::Mean(&eAvail) << " (";
	cout << eAvail[0];
	for (int sig = 1; sig < data->S; ++sig)
		cout << ", " << eAvail[sig];
	cout << ")" << endl;
	enerAvail = MathHelp::Mean(&eAvail);

	cout << "Average production losses: " << MathHelp::Mean(&lossesScen) << " (";
	cout << lossesScen[0];
	for (int sig = 1; sig < data->S; ++sig)
		cout << ", " << lossesScen[sig];
	cout << ")" << endl;
	prodLosses = MathHelp::Mean(&lossesScen);
}

void YearSolution::printScenarios()
{
	int nSigs = data->S;
	int nMonths = data->M;

	vector<double> sigWeight = vector<double>(nSigs, 1.0 / (float)nSigs);
	vector<double> sigTotals = vector<double>(nSigs, 0.0);

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

	for (int m = 0; m < data->M; ++m)
	{
		double e = data->eH[m];

		// Vessel Rentals
		for (int y = 0; y < data->Y - 1; ++y)
			vesselRental[m] += getVessels()[m][y] * data->cV[y][m];
		vr += vesselRental[m];

		// Technicians
		technicians[m] = getVessels()[m][data->techId] * data->cV[data->techId][m];
		te += technicians[m];

		// Downtime planned
		for (int ip = 0; ip < data->Ip; ++ip)
			plannedDowntime[m] += getPlanned()[m][ip] * data->dP * e;
		pd += plannedDowntime[m];
	}

	for (int sig = 0; sig < nSigs; ++sig)
	{
		double scenTotal = 0.0;

		cout << "SCENARIO " << sig << endl;
		cout << "Costs per month:" << endl;
		cout << "Month: Total (Vessel rentals + Technicians + Planned downtime + Repairs + Unhandled failures)" << endl;

		for (int m = 0; m < data->M; ++m)
		{
			double e = data->eH[m];

			// Task costs
			for (int ir = 0; ir < data->Ir; ++ir)
				repairsExpected[sig][m] += getRepairs()[sig][m][ir] * (data->dR[ir] + data->dD[ir]) * e;
			repairs[sig][m] = repairsExpected[sig][m];

			// Unhandled failures
			for (int ir = 0; ir < data->Ir; ++ir)
				unhandledFailures[sig][m] += getUnhandled()[sig][m][ir] * data->H[m] * e;

			double total = vesselRental[m] + technicians[m] + plannedDowntime[m] + repairs[sig][m] + unhandledFailures[sig][m];
			re[sig] += repairs[sig][m];
			reexp[sig] += repairsExpected[sig][m];
			uf[sig] += unhandledFailures[sig][m];

			cout << m << ": " << total << " (" << vesselRental[m] << " + " << technicians[m] << " + " << plannedDowntime[m] << " + " << repairs[sig][m] << " + " << unhandledFailures[sig][m] << ")" << endl;

			scenTotal += total;
		}

		// Last month penalty
		for (int ir = 0; ir < data->Ir; ++ir)
			lo[sig] += getUnhandled()[sig][data->M - 1][ir] * data->lambda[ir];
		scenTotal += lo[sig];

		sigTotals[sig] = scenTotal;

		cout << endl;
		cout << "Total: " << scenTotal << endl;
		cout << "Totals per category:" << endl;
		cout << "Vessel rentals: " << vr << endl;
		cout << "Technician costs: " << te << endl;
		cout << "Planned downtime: " << pd << endl;
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
		cout << "Expected repairs: " << MathHelp::Mean(&reexp) << endl;
		cout << "Unhandled failures: " << MathHelp::Mean(&uf) << endl;
		cout << "Leftover failures: " << MathHelp::Mean(&lo) << endl;
		cout << endl;
	}
}

void YearSolution::printDinwoodie()
{
	double vCosts = 0.0, rCosts = 0.0, tCosts = 0.0;

	for (int m = 0; m < data->M; ++m)
	{
		for (int y = 0; y < data->Y; ++y)
			if (y != data->techId)
				vCosts += data->cV[y][m] * getVessels()[m][y];
			else
				tCosts += data->cV[y][m] * getVessels()[m][y];

		for (int ip = 0; ip < data->Ip; ++ip)
			rCosts += data->cP * getPlanned()[m][ip];
		for (int ir = 0; ir < data->Ir; ++ir)
			for (int sig = 0; sig < data->S; ++sig)
				rCosts += data->cR[ir] * getRepairs()[sig][m][ir];
	}
	double costs = vCosts + rCosts + tCosts;

	double unit = 1.0 / (1000000.0 * (double)data->M / (double)data->monthsPerYear);

	cout << "-----------DINWOODIE-----------" << endl;
	cout << "Availability (time): " << timeAvail << "%" << endl;
	cout << "Availability (energy): " << enerAvail << "%" << endl;
	cout << "Annual production losses: " << prodLosses * unit << " m" << endl;
	cout << "Annual direct O&M costs: " << costs * unit << " m" << endl;
	cout << "Annual vessel costs: " << vCosts * unit << " m" << endl;
	cout << "Annual repair costs: " << rCosts * unit << " m" << endl;
	cout << "Annual technician costs: " << tCosts * unit << " m" << endl << endl;

	cout << "Annual Costs + losses: " << (prodLosses + costs) * unit << " m" << endl;
	cout << "Total Costs + losses: " << (prodLosses + costs) / 1000000.0 << " m" << endl;
}

void YearSolution::print()
{
	// Objective & Duration
	printObj();
	printDur();

	// Decision Variables
	printVessels();
	printPlanned();
	printFailures();
	printRepairs();
	printUnhandled();

	// Breakdowns
	printAvailability();
	printScenarios();

	// Summary
	printDinwoodie();
}

//-----------------------------------------------MONTH---------------------------------------------

MonthSolution::MonthSolution(string name, int id) : Solution(name, id) { }

void MonthSolution::setFinishes(vector<double> f)
{
	copy(f.begin(), f.end(), back_inserter(finishes));
}

void MonthSolution::setStarts(vector<vector<double>> s)
{
	copy(s.begin(), s.end(), back_inserter(starts));
}

void MonthSolution::setOrders(vector<vector<int>> a)
{
	copy(a.begin(), a.end(), back_inserter(orders));
}

void MonthSolution::printFinishes()
{
	cout << "Finish times for each task:" << endl;

	for (int i = 0; i < finishes.size(); ++i)
		cout << i << ": " << finishes[i] << endl;

}

void MonthSolution::printStarts()
{
	cout << "Start times for each vessel:" << endl;

	for (int v = 0; v < starts.size(); ++v)
	{
		if (starts[v].empty())
			continue;

		cout << v << ": " << starts[v][0];

		for (int j = 1; j < starts[v].size(); ++j)
			cout << ", " << starts[v][j];

		cout << endl;
	}

}

void MonthSolution::printOrders()
{
	cout << "Task order (per vessel):" << endl;

	for (int v = 0; v < orders.size(); ++v)
	{
		if (orders[v].empty())
			continue;

		cout << v << ": " << orders[v][0];

		for (int j = 1; j < orders[v].size(); ++j)
			cout << ", " << orders[v][j];

		cout << endl;
	}
}

void MonthSolution::print()
{
	printObj();
	printDur();
	printFinishes();
	printStarts();
	printOrders();
}
