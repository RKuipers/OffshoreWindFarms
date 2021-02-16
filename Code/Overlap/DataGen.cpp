#include "DataGen.h"

vector<string> DataGen::readLine(ifstream* datafile, char sep)
{
	string line = "#";
	vector<string> res = vector<string>();

	while (line[0] == '#')
		getline(*datafile, line);

	if (line.empty())
		return res;

	size_t pos = 0;

	while (line.find(sep, pos) != string::npos)
	{
		size_t newPos = line.find(sep, pos);
		res.push_back(line.substr(pos, newPos - pos));
		pos = newPos + 1;
	}

	res.push_back(line.substr(pos));

	return res;
}

bool DataGen::readEmpty(ifstream* datafile, char sep)
{
	vector<string> line = readLine(datafile, sep);
	if (!line.empty())
		cout << "ERROR: Expected empty line" << endl;
	return line.empty();
}

int DataGen::parseArray(vector<string> line, int start, vector<int>* res, int amount)
{
	// Switch based on 3 types: U (universal value), I (intervals), S (single values)

	res->clear();
	(*res) = vector<int>(amount);

	char type = line[start][0];

	switch (type)
	{
	case 'U': // U x -> x used for all periods
	{
		int val = stoi(line[start + 1]);
		fill(res->begin(), res->begin() + amount, val);
		return start + 2;
	}
	case 'A': // A a1 v1 a2 v2 ... an vn -> The next ax periods use vx, sum of ax should be equal to amount of periods
	{
		int filled = 0;
		int loc = start + 1;
		while (filled < amount)
		{
			int amount = stoi(line[loc]);
			int val = stoi(line[loc + 1]);

			fill(res->begin() + filled, res->begin() + filled + amount, val);
			filled += amount;
			loc += 2;
		}
		return loc;
	}
	case 'S': // p1 v1 p2 v2 ... pn vn -> Period px uses vx, every period should be mentioned
	{
		for (int i = 0; i < amount; ++i)
			(*res)[i] = stoi(line[i + start + 1]);
		return start + amount + 1;
	}
	default:
	{
		cout << "Error reading a Periodical" << endl;
		return -1;
	}
	}
}

int DataGen::parseArrayDouble(vector<string> line, int start, vector<double>* res, int amount)
{
	// Switch based on 3 types: U (universal value), I (intervals), S (single values)

	res->clear();
	(*res) = vector<double>(amount);

	char type = line[start][0];

	switch (type)
	{
	case 'U': // U x -> x used for all periods
	{
		double val = stod(line[start + 1]);
		fill(res->begin(), res->begin() + amount, val);
		return start + 2;
	}
	case 'A': // A a1 v1 a2 v2 ... an vn -> The next ax periods use vx, sum of ax should be equal to amount of periods
	{
		int filled = 0;
		int loc = start + 1;
		while (filled < amount)
		{
			int amount = stoi(line[loc]);
			double val = stod(line[loc + 1]);

			fill(res->begin() + filled, res->begin() + filled + amount, val);
			filled += amount;
			loc += 2;
		}
		return loc;
	}
	case 'S': // p1 v1 p2 v2 ... pn vn -> Period px uses vx, every period should be mentioned
	{
		for (int i = 0; i < amount; ++i)
			(*res)[i] = stod(line[i + start + 1]);
		return start + amount + 1;
	}
	default:
	{
		cout << "Error reading a Periodical" << endl;
		return -1;
	}
	}
}

int DataGen::genRandomFailures(double yearlyFailRate, int nTurbines)
{
	double mFR = yearlyFailRate / 12.0;

	int nFailures = 0;
	for (int t = 0; t < nTurbines; ++t)
		if (rand() / ((double)RAND_MAX) <= mFR)
			nFailures++;

	return nFailures;
}

vector<double> DataGen::genRandomFailureTimes(int failures, int maxTime)
{
	vector<double> res = vector<double>();

	for (int f = 0; f < failures; ++f)
		res.push_back((double)(rand() % maxTime));

	return res;
}

YearData* DataGen::readYear(ifstream* file)
{
	vector<int> arr = vector<int>();
	vector<double> arrD = vector<double>();

	// Set sizes
	vector<string> split = readLine(file);
	int S = stoi(split[0]);
	int M = stoi(split[1]);
	int Y = stoi(split[2]);
	int Ip = stoi(split[3]);
	int Ir = stoi(split[4]);
	YearData* year = new YearData(S, M, Y, Ip, Ir);
	readEmpty(file);

	// Vessels
	for (int y = 0; y < Y; ++y)
	{
		split = readLine(file);
		int ind = 2;
		year->L[y] = stoi(split[1]);

		ind = parseArrayDouble(split, ind, &arrD, M);
		for (int m = 0; m < M; ++m)
			year->c[y][m] = arrD[m];

		ind = parseArray(split, ind, &arr, M);
		for (int m = 0; m < M; ++m)
			year->A[y][m] = arr[m];

		year->dPy[y] = stod(split[ind]);
		ind++;

		ind = parseArrayDouble(split, ind, &arrD, Ir);
		for (int i = 0; i < Ir; ++i)
			year->dRy[y][i] = arrD[i];

		ind = parseArray(split, ind, &arr, M);
		for (int m = 0; m < M; ++m)
			year->LInst[y][m] = arr[m];

		ind = parseArray(split, ind, &arr, M);
		for (int m = 0; m < M; ++m)
			year->NInst[y][m] = arr[m];
	}
	readEmpty(file);

	// Offline duration for planned task
	year->dP = stoi(readLine(file)[0]);
	readEmpty(file);

	// Hours in given month
	parseArray(readLine(file), 0, &arr, M);
	for (int m = 0; m < M; ++m)
		year->H[m] = arr[m];
	readEmpty(file);

	// Energy$ per hour in given month
	parseArrayDouble(readLine(file), 0, &arrD, M);
	for (int m = 0; m < M; ++m)
		year->eH[m] = arrD[m];
	readEmpty(file);

	// Min/Max months between maint
	split = readLine(file);
	year->GL = stoi(split[0]);
	year->GU = stoi(split[1]);
	readEmpty(file);

	//Failure characteristics
	for (int ir = 0; ir < Ir; ++ir)
	{
		split = readLine(file);
		year->lambda[ir] = stod(split[1]);
		year->dD[ir] = stod(split[2]);
		year->dR[ir] = stod(split[3]);
	}
	readEmpty(file);

	// Turbines
	parseArray(readLine(file), 0, &arr, M);
	for (int m = 0; m < M; ++m)
		year->Turbs[m] = arr[m];
	readEmpty(file);

	// Failures
	year->randFailMode = readLine(file)[0] == "R";
	if (!year->randFailMode)
	{
		for (int s = 0; s < S; ++s)
		{
			split = readLine(file);
			int ind = 1;
			for (int i = 0; i < Ir; i++)
			{
				ind = parseArray(split, ind, &arr, M);
				for (int m = 0; m < M; ++m)
					year->Ft[m][i][s] = arr[m];
			}
		}
	}
	else
	{
		split = readLine(file);
		vector<int> turbs = vector<int>(M, 0);

		turbs[0] = year->Turbs[0];
		for (int m = 1; m < M; ++m)
			turbs[m] = turbs[m - 1] + year->Turbs[m];

		for (int s = 0; s < S; ++s)
			for (int ir = 0; ir < Ir; ++ir)
			{
				double yFR = stod(split[ir]);
				for (int m = 0; m < M; ++m)
					year->Ft[m][ir][s] = genRandomFailures(yFR, turbs[m]);
			}
	}

	return year;
}

MonthData* DataGen::readMonth(ifstream* file)
{
	vector<int> arr = vector<int>();
	vector<double> arrD = vector<double>();
	int ind = 0;

	// Set sizes
	vector<string> split = readLine(file);
	int Y = stoi(split[0]);
	int V = stoi(split[1]);
	int IMaint = stoi(split[2]);
	int IInst = stoi(split[3]);
	int J = stoi(split[4]);
	MonthData* month = new MonthData(Y, V, IMaint, IInst, J);
	readEmpty(file);

	int I = IMaint + IInst;
	int VyTotal = 0;
	// Vessels
	for (int y = 0; y < Y; ++y)
	{
		split = readLine(file);
		int ind = 2;
		month->Vy[y] = stoi(split[1]) + VyTotal;
		VyTotal += month->Vy[y];
	}
	readEmpty(file);

	// Maintenance tasks
	for (int i = 0; i < IMaint; ++i)
	{
		split = readLine(file);
		month->c[i] = stod(split[1]);
		month->r[i] = stod(split[2]);
		month->A[i] = stoi(split[3]);

		ind = parseArrayDouble(split, 4, &arrD, Y);
		for (int y = 0; y < Y; ++y)
			month->d[y][i] = arrD[y];

		ind = parseArray(split, ind, &arr, Y);
		for (int y = 0; y < Y; ++y)
			month->rho[y][i] = arr[y];

		for (; ind < split.size(); ++ind)
			month->PR.push_back(pair<int,int>(stoi(split[ind]), i));
	}
	readEmpty(file);

	// Installation tasks
	for (int i = 0; i < IInst; ++i)
	{
		split = readLine(file);
		month->vInst[i] = stoi(split[1]);
		month->aInst[month->vInst[i]][i] = 1;
		month->sInst[i] = stod(split[2]);
		int type = -1;
		for (int y = 0; y < Y; ++y)
			if (month->vInst[i] < month->Vy[y])
			{
				type = y;
				break;
			}
		month->d[type][i + IMaint] = stod(split[3]);
	}
	readEmpty(file);

	// End Time
	month->T = stod(readLine(file)[0]);

	return month;
}

MixedData* DataGen::readMixed(ifstream* file)
{
	srand(SEED); // TODO: Lose seed

	vector<int> arr = vector<int>();
	vector<double> arrD = vector<double>();

	const YearData& year = (*readYear(file));
	readEmpty(file);
	MixedData* mixed = new MixedData(year);

	int VInst = 0;
	for (int y = 0; y < year.Y; ++y)
		for (int m = 0; m < year.M; ++m)
			VInst += year.NInst[y][m];

	vector<string> split;

	// Vessels
	for (int y = 0; y < year.Y; ++y)
	{
		split = readLine(file);

		mixed->rhoP[y] = stoi(split[1]);

		parseArray(split, 2, &arr, year.Ir);
		for (int i = 0; i < year.Ir; ++i)
			mixed->rhoR[y][i] = arr[i];
	}
	readEmpty(file);

	// Installation vessels
	int tasksAdded = 0;
	for (int v = 0; v < VInst; ++v)
	{
		split = readLine(file);
		int y = stoi(split[1]);
		int m = stoi(split[2]);
		int II = stoi(split[3]);
		mixed->IInst[m] += II;

		if (mixed->aInst[m].empty())
		{
			mixed->aInst[m] = vector<int>();
			mixed->vTypes[m] = vector<int>();
		}
		mixed->vTypes[m].push_back(y);

		int index = 4;
		for (int i = 0; i < II; ++i)
		{
			mixed->dI[m][y].push_back(stod(split[index + 1]));
			for (int y_ = 0; y_ < year.Y; ++y_)
				if (y != y_)
					mixed->dI[m][y_].push_back(0);
			mixed->sInst[m].push_back(stod(split[index]));
			mixed->aInst[m].push_back(mixed->vTypes[m].size() - 1);
			index += 2;
		}
		tasksAdded += II;
	}
	readEmpty(file);

	// Month and Day Length
	split = readLine(file);
	mixed->T = stod(split[0]);
	mixed->DT = stod(split[1]);

	// Failure times
	if (!year.randFailMode)
	{
		readEmpty(file);

		for (int m = 0; m < year.M; ++m)
		{
			split = readLine(file);
			int ind = 1;
			for (int ir = 0; ir < year.Ir; ++ir)
			{
				for (int f = 0; f < year.Ft[m][ir][0]; ++f) // TODO: sig is always 0?
					mixed->FTime[m][ir].push_back(stod(split[ind + f]));
				ind += year.Ft[m][ir][0];
			}
		}
	}
	else
	{
		for (int ir = 0; ir < year.Ir; ++ir)
			for (int m = 0; m < year.M; ++m)
				mixed->FTime[m][ir] = genRandomFailureTimes(year.Ft[m][ir][0], mixed->T);
	}
	for (int ir = 0; ir < year.Ir; ++ir)
		for (int m = 0; m < year.M; ++m)
		{
			int index = 0;
			sort(mixed->FTime[m][ir].begin(), mixed->FTime[m][ir].end());
			for (int d = 1; d * mixed->DT <= mixed->T; ++d)
				for (; index < mixed->FTime[m][ir].size() && mixed->FTime[m][ir][index] < d * mixed->DT; ++index)
					mixed->FTime[m][ir][index] = d * mixed->DT;
		}

	return mixed;
}

vector<MonthData> DataGen::genMonths(MixedData* data, YearSolution* sol)
{
	int sig = 0; // TODO: FIND SOLUTION FOR SCENARIOS

	vector<MonthData> res = vector<MonthData>();

	for (int m = 0; m < data->M; ++m)
	{
		vector<int> Vy = vector<int>(data->Y, 0);
		vector<int> VInds = vector<int>(data->Y, 0);
		int VyTotal = 0;

		for (int y = 0; y < data->Y; ++y)
		{
			VInds[y] = VyTotal;
			VyTotal += sol->getVessels()[sig][m][y] + data->NInst[y][m];
			Vy[y] = VyTotal;
		}

		int plannedAmount = 0, repairs = 0;
		for (int i = 0; i < data->Ip; ++i)
			plannedAmount += sol->getPlanned()[m][i];
		for (int i = 0; i < data->Ir; ++i)
			repairs += sol->getRepairs()[sig][m][i];

		int plannedTypes = 0;
		if (plannedAmount > 0)
			plannedTypes = 1;

		int Im = plannedTypes + repairs;
		int I = Im + data->IInst[m];
		int J = plannedAmount + repairs + data->IInst[m];

		MonthData month = MonthData(data->Y, VyTotal, Im, data->IInst[m], J);

		// Vessels
		for (int y = 0; y < data->Y; ++y)
		{
			month.Vy[y] = Vy[y];

			for (int i = 0; i < plannedTypes; ++i) // Planned tasks
			{
				month.d[y][i] = data->dPy[y];
				month.rho[y][i] = data->rhoP[y];
			}

			int repDone = 0, reactDone = 0;
			for (int ir = 0; ir < data->Ir; ++ir) // Failure TYPES
			{
				for (int i = 0; i < sol->getRepairs()[sig][m][ir]; ++i) // Repair tasks
				{
					month.d[y][i + plannedTypes + repDone] = data->dRy[y][ir];
					month.rho[y][i + plannedTypes + repDone] = data->rhoR[y][ir];
				}
				repDone += sol->getRepairs()[sig][m][ir];
			}

			for (int i = 0; i < data->IInst[m]; ++i) // Installation tasks
				month.d[y][i + Im] = data->dI[m][y][i];
		}

		// Costs per task
		for (int i = 0; i < plannedTypes; ++i) // Planned tasks
			month.c[i] = 0;
		for (int i = plannedTypes; i < Im; ++i) // Repair tasks
			month.c[i] = data->eH[m];

		// Release times
		for (int i = 0; i < plannedTypes + repairs; ++i) // Planned & repair tasks
			month.r[i] = 0;

		// Amount of tasks
		if (plannedAmount > 1)
			month.A[0] = plannedAmount;
		for (int i = plannedTypes; i < Im; ++i)
			month.A[i] = 1;

		// Installation tasks
		for (int i = 0; i < data->IInst[m]; ++i)
			month.sInst[i] = data->sInst[m][i];

		// Installation assignments
		vector<int> vTrans = vector<int>();
		vector<int> used = vector<int>(data->Y, 0);
		for (int v = 0; v < data->vTypes[m].size(); ++v)
		{
			int y = data->vTypes[m][v];
			vTrans.push_back(VInds[y] + used[y]);
			used[y]++;
			if (used[y] > data->NInst[y][m])
				cout << "ERROR: Error in conversion between global and local vessel indices" << endl;
		}
		for (int i = 0; i < data->aInst[m].size(); ++i)
		{
			month.aInst[vTrans[data->aInst[m][i]]][i] = 1;
			month.vInst[i] = vTrans[data->aInst[m][i]];
		}

		// End Time
		month.T = data->T;

		res.push_back(month);
	}

	return res;
}

vector<MonthData> DataGen::genMonths2(MixedData* data, YearSolution* sol)
{
	int sig = 0; // TODO: FIND SOLUTION FOR SCENARIOS

	vector<MonthData> res = vector<MonthData>();

	for (int m = 0; m < data->M; ++m)
	{
		// Data indexing
		vector<int> Vy = vector<int>(data->Y, 0);
		vector<int> VInds = vector<int>(data->Y, 0);
		int VyTotal = 0;

		for (int y = 0; y < data->Y; ++y)
		{
			VInds[y] = VyTotal;
			VyTotal += sol->getVessels()[sig][m][y] + data->NInst[y][m];
			Vy[y] = VyTotal;
		}

		int mTaskTypes = 0, mTasks = 0, pTypes = 0, rTypes = 0;
		vector<vector<double>> fTimes = vector<vector<double>>(data->Ir, vector<double>());
		vector<vector<int>> fAmounts = vector<vector<int>>(data->Ir, vector<int>());
		vector<int> types = vector<int>();
		for (int ip = 0; ip < data->Ip; ++ip)
			if (sol->getPlanned()[m][ip] > 0)
			{
				mTaskTypes = 1;
				mTasks += sol->getPlanned()[m][ip];
				pTypes = 1;
			}
		int plannedAmount = mTasks;
		for (int ir = 0; ir < data->Ir; ++ir)
			if (sol->getRepairs()[sig][m][ir] > 0)// TODO: This needs testing
			{
				if (sol->getRepairs()[sig][m][ir] >= data->FTime[m][ir].size())
				{
					//copy(data->FTime[m][ir].begin(), data->FTime[m][ir].end(), fTimes[ir]);
					while (fTimes[ir].size() < sol->getRepairs()[sig][m][ir])
						fTimes[ir].push_back(0);
					sort(fTimes[ir].begin(), fTimes[ir].end());
				}
				else if (sol->getRepairs()[sig][m][ir] < data->FTime[m][ir].size()) 
				{
					//copy(data->FTime[m][ir].begin(), data->FTime[m][ir].end(), fTimes[ir]);
					//shuffle(fTimes[ir].begin(), fTimes[ir].end(), SEED);
					while (fTimes[ir].size() > sol->getRepairs()[sig][m][ir])
						fTimes[ir].pop_back();
					sort(fTimes[ir].begin(), fTimes[ir].end());					
				}
				fAmounts[ir].push_back(1);
				for (int i = 1; i < fTimes[ir].size(); ++i)
					if (fTimes[ir][i] == fTimes[ir][i - 1])
						++fAmounts[ir][fAmounts[ir].size() - 1];
					else
						fAmounts[ir].push_back(1);
				fTimes[ir].erase(unique(fTimes[ir].begin(), fTimes[ir].end()), fTimes[ir].end());

				for (int i = 0; i < fTimes[ir].size(); ++i)
					types.push_back(ir);

				mTaskTypes += fAmounts[ir].size();
				mTasks += sol->getRepairs()[sig][m][ir];
				rTypes += fAmounts[ir].size();
			}
		int totTaskTypes = mTaskTypes + data->IInst[m];
		int totTasks = mTasks + data->IInst[m];

		MonthData month = MonthData(data->Y, VyTotal, mTaskTypes, data->IInst[m], totTasks);

		// Vessels (durations and requirements)
		for (int y = 0; y < data->Y; ++y)
		{
			for (int ip = 0; ip < pTypes; ++ip) // Planned
			{
				month.d[y][ip] = data->dPy[y];
				month.rho[y][ip] = data->rhoP[y];
			}
			for (int ir = pTypes; ir < mTaskTypes; ++ir) // Repair
			{
				month.d[y][ir] = data->dRy[y][types[ir - pTypes]];
				month.rho[y][ir] = data->rhoR[y][types[ir - pTypes]];
			}
			for (int ii = mTaskTypes; ii < totTaskTypes; ++ii) // Install
				month.d[y][ii] = data->dI[m][y][ii - mTaskTypes];
		}

		// Costs per task
		for (int ip = 0; ip < pTypes; ++ip) // Planned
			month.c[ip] = 0;
		for (int ir = pTypes; ir < mTaskTypes; ++ir) // Repair
			month.c[ir] = data->eH[m];

		// Release times
		for (int ip = 0; ip < pTypes; ++ip) // Planned
			month.r[ip] = 0; 
		for (int ir = pTypes; ir < mTaskTypes; ++ir) // Repair
			month.r[ir] = fTimes[types[ir - pTypes]][ir - pTypes]; // TODO: Test these indices

		// Amount of tasks
		for (int ip = 0; ip < pTypes; ++ip) // Planned
			month.A[ip] = plannedAmount;
		for (int ir = pTypes; ir < mTaskTypes; ++ir) // Repair
			month.A[ir] = fAmounts[types[ir - pTypes]][ir - pTypes]; // TODO: Test these indices

		// Installation tasks
		for (int ii = 0; ii < data->IInst[m]; ++ii)
			month.sInst[ii] = data->sInst[m][ii];

		// Installation assignments
		vector<int> vTrans = vector<int>();
		vector<int> used = vector<int>(data->Y, 0);
		for (int v = 0; v < data->vTypes[m].size(); ++v)
		{
			int y = data->vTypes[m][v];
			vTrans.push_back(VInds[y] + used[y]);
			used[y]++;
			if (used[y] > data->NInst[y][m])
				cout << "ERROR: Error in conversion between global and local vessel indices" << endl;
		}
		for (int i = 0; i < data->aInst[m].size(); ++i)
		{
			month.aInst[vTrans[data->aInst[m][i]]][i] = 1;
			month.vInst[i] = vTrans[data->aInst[m][i]];
		}

		// End Time
		month.T = data->T;

		res.push_back(month);
	}

	return res;
}
