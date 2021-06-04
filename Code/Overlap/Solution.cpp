#include "Solution.h"

//-----------------------------------------------BASE----------------------------------------------

Solution::Solution(string name) : name(name)
{ 
	duration = -1;
	printToConsole = true;
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

void Solution::setPrintMode(bool ptc)
{
	printToConsole = ptc;
}

string Solution::toCSV(double d)
{
	string s = to_string(d);
	replace(s.begin(), s.end(), '.', ',');
	return s;
}

//-----------------------------------------------YEAR----------------------------------------------

YearSolution::YearSolution(string name, YearData* data) : Solution(name), data(data) { }

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

void YearSolution::calcSecondaries()
{
	avail = vector<vector<double>>(data->S, vector<double>(data->M, 0.0));
	timeUnavailP = vector<vector<double>>(data->S, vector<double>(data->M, 0.0));
	timeUnavailR = vector<vector<vector<double>>>(data->S, vector<vector<double>>(data->M, vector<double>(data->Ir, 0.0)));
	timeUnavailU = vector<vector<vector<double>>>(data->S, vector<vector<double>>(data->M, vector<double>(data->Ir, 0.0)));

	for (int sig = 0; sig < data->S; ++sig)
	{
		int turbs = 0;
		for (int m = 0; m < data->M; ++m)
		{
			// Base time
			turbs += data->Turbs[m];
			double maxTime = turbs * data->H[m];
			double timeAvail = maxTime;

			// Planned
			timeUnavailP[sig][m] = MathHelp::Sum(&planned[m]) * data->dP;
			timeAvail -= timeUnavailP[sig][m];

			for (int ir = 0; ir < data->Ir; ir++)// The code in this loop causes discrepancies between Objective and printed results
			{
				// Repairs
				int newRepairs = min(repairs[sig][m][ir], data->Ft[m][ir][sig]);				
				timeUnavailR[sig][m][ir] += newRepairs * (data->dR[ir] + data->dD[ir]);			// Repairs that come from new failures
				timeUnavailR[sig][m][ir] += (repairs[sig][m][ir] - newRepairs) * data->dR[ir];	// Repairs that come from previously unhandled failures
				timeAvail -= timeUnavailR[sig][m][ir];

				// Unhandled
				int partiallyInactive = 0;
				if (m == 0)
					partiallyInactive = unhandled[sig][m][ir];
				else if (unhandled[sig][m][ir] > unhandled[sig][m - 1][ir])
					partiallyInactive = unhandled[sig][m][ir] - unhandled[sig][m - 1][ir];

				timeUnavailU[sig][m][ir] += (unhandled[sig][m][ir] - partiallyInactive) * data->H[m];
				for (int f = 0; f < partiallyInactive; ++f)
					timeUnavailU[sig][m][ir] += (double)(rand() % data->H[m]);
				timeAvail -= timeUnavailU[sig][m][ir];
			}

			avail[sig][m] = 100 * timeAvail / maxTime;
		}
	}

	vCosts = vector<double>(data->M, 0.0);
	tCosts = vector<double>(data->M, 0.0);

	for (int m = 0; m < data->M; ++m)
	{
		for (int y = 0; y < data->Y; ++y)
			vCosts[m] += vessels[m][y] * data->cV[y][m];

		vCosts[m] -= vessels[m][data->techId] * data->cV[data->techId][m];
		tCosts[m] += vessels[m][data->techId] * data->cV[data->techId][m];
	}
}

void YearSolution::printAvailability()
{
	if (printToConsole)
		cout << "Availability and production losses (per month V and scenario >): " << endl;

	vector<vector<double>> prodLosses = vector<vector<double>>(data->S, vector<double>(data->M, 0.0));

	for (int m = 0; m < data->M; ++m)
	{
		for (int sig = 0; sig < data->S; ++sig)
			prodLosses[sig][m] = (timeUnavailP[sig][m] + MathHelp::Sum(&timeUnavailR[sig][m]) + MathHelp::Sum(&timeUnavailU[sig][m])) * data->eH[m];

		if (printToConsole)
		{
			cout << m << ": " << avail[0][m];
			for (int sig = 1; sig < data->S; ++sig)
				cout << ", " << avail[sig][m];

			cout << " / " << prodLosses[0][m];
			for (int sig = 1; sig < data->S; ++sig)
				cout << ", " << prodLosses[sig][m];
			cout << endl;
		}
	}

	vector<double> availScen = vector<double>(data->S);
	for (int sig = 0; sig < data->S; ++sig)
		availScen[sig] = MathHelp::Mean(&avail[sig]);
	avgAvailT = MathHelp::Mean(&availScen);

	double meanEnergy = MathHelp::Mean(&data->eH);
	vector<double> availEn = vector<double>(data->S);
	for (int sig = 0; sig < data->S; ++sig)
		availEn[sig] = MathHelp::WeightedMean(&avail[sig], &data->eH) / meanEnergy;
	avgAvailE = MathHelp::Mean(&availEn);

	vector<double> prodLossesScen = vector<double>(data->S);
	for (int sig = 0; sig < data->S; ++sig)
		prodLossesScen[sig] = MathHelp::Sum(&prodLosses[sig]);
	totProdLoss = MathHelp::Mean(&prodLossesScen);

	if (printToConsole)
	{
		cout << "Average time availability: " << avgAvailT << " (";
		cout << availScen[0];
		for (int sig = 1; sig < data->S; ++sig)
			cout << ", " << availScen[sig];
		cout << ")" << endl;

		cout << "Average energy availability: " << avgAvailE << " (";
		cout << availEn[0];
		for (int sig = 1; sig < data->S; ++sig)
			cout << ", " << availEn[sig];
		cout << ")" << endl;

		cout << "Average production losses: " << totProdLoss << " (";
		cout << prodLossesScen[0];
		for (int sig = 1; sig < data->S; ++sig)
			cout << ", " << prodLossesScen[sig];
		cout << ")" << endl;
	}

	cout << endl;
}

void YearSolution::printScenarios()
{
	int nSigs = data->S;
	int nMonths = data->M;

	vector<double> sigWeight = vector<double>(nSigs, 1.0 / (float)nSigs);
	vector<double> sigTotals = vector<double>(nSigs, 0.0);

	// Scenario independant
	vector<double> vesselRental = vector<double>(nMonths, 0.0);
	vector<double> technicians = vector<double>(nMonths, 0.0);
	vector<double> plannedDowntime = vector<double>(nMonths, 0.0);
	double vr = 0.0;
	double te = 0.0;
	double pd = 0.0;

	// Scenario dependant
	vector<vector<double>> repairDowntime = vector<vector<double>>(nSigs, vector<double>(nMonths, 0.0));
	vector<vector<double>> repairFlatCost = vector<vector<double>>(nSigs, vector<double>(nMonths, 0.0));
	vector<vector<double>> unhandledFailures = vector<vector<double>>(nSigs, vector<double>(nMonths, 0.0));
	vector<double> rd = vector<double>(nSigs, 0.0);
	vector<double> rf = vector<double>(nSigs, 0.0);
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
		cout << "Month: Total (Vessel rentals + Technicians + Planned downtime + Repair downtime + Repair costs + Unhandled failures)" << endl;

		for (int m = 0; m < data->M; ++m)
		{
			double e = data->eH[m];

			// Repair costs
			for (int ir = 0; ir < data->Ir; ++ir)
			{
				repairDowntime[sig][m] += getRepairs()[sig][m][ir] * (data->dR[ir] + data->dD[ir]) * e;
				repairFlatCost[sig][m] += getRepairs()[sig][m][ir] * data->cR[ir];
			}

			// Unhandled failures
			for (int ir = 0; ir < data->Ir; ++ir)
				unhandledFailures[sig][m] += getUnhandled()[sig][m][ir] * data->H[m] * e;

			double total = vesselRental[m] + technicians[m] + plannedDowntime[m] + repairDowntime[sig][m] + repairFlatCost[sig][m] + unhandledFailures[sig][m];
			rd[sig] += repairDowntime[sig][m];
			rf[sig] += repairFlatCost[sig][m];
			uf[sig] += unhandledFailures[sig][m];

			cout << m << ": " << total << " (" << vesselRental[m] << " + " << technicians[m] << " + " << plannedDowntime[m] << " + " << repairDowntime[sig][m] << " + " << repairFlatCost[sig][m] << " + " << unhandledFailures[sig][m] << ")" << endl;

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
		cout << "Repair downtime: " << rd[sig] << endl;
		cout << "Repair costs: " << rf[sig] << endl;
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
		cout << "Repair downtime: " << MathHelp::Mean(&rd) << endl;
		cout << "Repair costs: " << MathHelp::Mean(&rf) << endl;
		cout << "Unhandled failures: " << MathHelp::Mean(&uf) << endl;
		cout << "Leftover failures: " << MathHelp::Mean(&lo) << endl;
		cout << endl;
	}

	cout << endl;
}

void YearSolution::printDinwoodie()
{
	rCosts = 0.0;
	double sigWeight = 1.0 / (double)data->S;
	for (int m = 0; m < data->M; ++m)
	{
		for (int ip = 0; ip < data->Ip; ++ip)
			rCosts += data->cP * getPlanned()[m][ip];
		for (int ir = 0; ir < data->Ir; ++ir)
			for (int sig = 0; sig < data->S; ++sig)
				rCosts += data->cR[ir] * getRepairs()[sig][m][ir] * sigWeight;
	}
	if (!printToConsole)
		return;

	double vCostsSum = MathHelp::Sum(&vCosts);
	double tCostsSum = MathHelp::Sum(&tCosts);
	double costs = vCostsSum + rCosts + tCostsSum;

	double unit = 1.0 / (1000000.0 * (double)data->M / (double)data->monthsPerYear);

	cout << "-----------DINWOODIE-----------" << endl;
	cout << "Availability (time): " << avgAvailT << "%" << endl;
	cout << "Availability (energy): " << avgAvailE << "%" << endl;
	cout << "Annual production losses: " << totProdLoss * unit << " m" << endl;
	cout << "Annual direct O&M costs: " << costs * unit << " m" << endl;
	cout << "Annual vessel costs: " << vCostsSum * unit << " m" << endl;
	cout << "Annual repair costs: " << rCosts * unit << " m" << endl;
	cout << "Annual technician costs: " << tCostsSum * unit << " m" << endl << endl;

	cout << "Annual Costs + losses: " << (totProdLoss + costs) * unit << " m" << endl;
	cout << "Total Costs + losses: " << (totProdLoss + costs) / 1000000.0 << " m" << endl;
}

void YearSolution::writeCSV()
{
	ofstream file;
	file.open("Output Files/" + name + ".csv", ofstream::out | ofstream::trunc);
	string sep = ";";

	// Columns
	file << "Month_Scenario" << sep;
	for (int y = 0; y < data->Y; ++y)
		file << "Vessels type " << y << sep;
	for (int ip = 0; ip < data->Ip; ++ip)
		file << "Planned round  " << ip << sep;
	for (int ir = 0; ir < data->Ir; ++ir)
		file << "Repairs type " << ir << sep;
	for (int ir = 0; ir < data->Ir; ++ir)
		file << "Unhandled type " << ir << sep;
	for (int ir = 0; ir < data->Ir; ++ir)
		file << "Failures type " << ir << sep;
	file << "Available %" << sep << "Energy Value" << sep;
	file << "Production Losses P" << sep << "Production Losses R" << sep << "Production Losses U" << sep;
	file << "Vessel costs" << sep << "Repair costs" << sep << "Technician costs" << endl;

	for (int sig = 0; sig < data->S; ++sig)
		for (int m = 0; m < data->M; ++m)
		{
			// ID
			file << m << "_" << sig << sep;

			// Decision Variables
			// Vessels
			for (int y = 0; y < data->Y; ++y)
				file << vessels[m][y] << sep;

			// Planned
			for (int ip = 0; ip < data->Ip; ++ip)
				file << planned[m][ip] << sep;

			// Repair
			for (int ir = 0; ir < data->Ir; ++ir)
				file << repairs[sig][m][ir] << sep;

			// Unhandled
			for (int ir = 0; ir < data->Ir; ++ir)
				file << unhandled[sig][m][ir] << sep;

			// Failures
			for (int ir = 0; ir < data->Ir; ++ir)
				file << data->Ft[m][ir][sig] << sep;

			// Secondary statistics
			// Availability
			file << toCSV(avail[sig][m]) << sep << data->eH[m] << sep;

			// Production Losses
			file << timeUnavailP[sig][m] * data->eH[m] << sep << MathHelp::Sum(&timeUnavailR[sig][m]) * data->eH[m] << sep << MathHelp::Sum(&timeUnavailU[sig][m]) * data->eH[m] << sep;

			// Costs
			file << (int)round(vCosts[m]) << sep << MathHelp::WeightedSum(&repairs[sig][m], &data->cR) + MathHelp::Sum(&planned[m]) * data->cP << sep << tCosts[m];

			file << endl;
		}

	file.close();
}

void YearSolution::writeCSVLine()
{
	ofstream file;
	file.open("Output Files/Collective.csv", ofstream::out | ofstream::app);
	string sep = ";";

	double years = (double)data->M / (double)data->monthsPerYear;

	// Basics
	file << name << sep << toCSV(getObj()) << sep << toCSV(duration) << sep;

	// Losses
	file << toCSV(avgAvailT) << sep << toCSV(avgAvailE) << sep << toCSV(totProdLoss / years) << sep;

	// Costs
	double vCostsSum = MathHelp::Sum(&vCosts);
	double tCostsSum = MathHelp::Sum(&tCosts);
	file << toCSV((vCostsSum + rCosts + tCostsSum)/years) << sep << toCSV(vCostsSum/years) << sep << toCSV(rCosts/years) << sep << toCSV(tCostsSum/years) << sep;

	// Setup info
	int prev = 0;
	vector<string> parts = vector<string>();
	for (int i = 0; i <= name.size(); ++i)
		if (i == name.size() || name[i] == '_')
		{
			parts.push_back(name.substr(prev, i - prev));
			prev = i+1;
		}
	for (string s : parts)
		file << s << sep;
	file << endl;
}

void YearSolution::print(bool collective)
{
	// Calculate secondary metrics
	calcSecondaries();

	if (printToConsole)
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
		cout << endl;
	}

	// Breakdowns
	printAvailability();
	if (printToConsole)
		printScenarios();

	// Summary
	printDinwoodie();

	// Individual output file
	writeCSV();

	if (collective)
	{
		// Collective output file
		writeCSVLine();
	}
}

//-----------------------------------------------MONTH---------------------------------------------

MonthSolution::MonthSolution(string name) : Solution(name) { }

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

void MonthSolution::print(bool collective)
{
	printObj();
	printDur();
	printFinishes();
	printStarts();
	printOrders();
}
