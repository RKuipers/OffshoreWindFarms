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

	// Duration of delay and work per type of failure
	split = readLine(file);
	int ind = parseArrayDouble(split, 0, &arrD, Ir);
	for (int i = 0; i < Ir; ++i)
		year->dD[i] = arrD[i];
	ind = parseArrayDouble(split, ind, &arrD, Ir);
	for (int i = 0; i < Ir; ++i)
		year->dR[i] = arrD[i];
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

	// Failures
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
	readEmpty(file);

	// Turbines
	parseArray(readLine(file), 0, &arr, M);
	for (int m = 0; m < M; ++m)
		year->Turbs[m] = arr[m];

	return year;
}

MonthData* DataGen::readMonth(ifstream* file)
{
	vector<int> arr = vector<int>();
	vector<double> arrD = vector<double>();

	// Set sizes
	vector<string> split = readLine(file);
	int Y = stoi(split[0]);
	int V = stoi(split[1]);
	int IMaint = stoi(split[2]);
	int IInst = stoi(split[3]);
	MonthData* month = new MonthData(Y, V, IMaint, IInst);
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

		ind = parseArrayDouble(split, ind, &arrD, IMaint);
		for (int i = 0; i < IMaint; ++i)
			month->s[y][i] = arrD[i];

		ind = parseArrayDouble(split, ind, &arrD, IMaint);
		for (int i = 0; i < IMaint; ++i)
		{
			month->d[y][i] = arrD[i];
			if (month->dMax[i] < month->s[y][i] + month->d[y][i])
				month->dMax[i] = month->s[y][i] + month->d[y][i];
		}

		ind = parseArray(split, ind, &arr, IMaint);
		for (int i = 0; i < IMaint; ++i)
			month->rho[y][i] = arr[i];
	}
	readEmpty(file);

	// Costs and release times per task
	split = readLine(file);
	int ind = parseArrayDouble(split, 0, &arrD, IMaint);
	for (int i = 0; i < IMaint; ++i)
		month->c[i] = arrD[i];
	ind = parseArrayDouble(split, ind, &arrD, IMaint);
	for (int i = 0; i < IMaint; ++i)
		month->r[i] = arrD[i];
	readEmpty(file);

	// Installation tasks
	for (int i = 0; i < IInst; ++i)
	{
		split = readLine(file);
		int vessel = stoi(split[1]);
		month->aInst[vessel][i] = 1;
		month->sInst[i] = stod(split[2]);
		int type = -1;
		for (int y = 0; y < Y; ++y)
			if (vessel < month->Vy[y])
			{
				type = y;
				break;
			}
		month->d[type][i + IMaint] = stod(split[3]);
	}
	readEmpty(file);

	// End Time
	month->T = stod(readLine(file)[0]);
	month->M = 3 * month->T;

	return month;
}

MixedData* DataGen::readMixed(ifstream* file)
{
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
		int ind = 2;
		mixed->sP[y] = stod(split[1]);

		ind = parseArrayDouble(split, ind, &arrD, year.Ir);
		for (int i = 0; i < year.Ir; ++i)
			mixed->sR[y][i] = arrD[i];

		mixed->rhoP[y] = stoi(split[ind]);

		ind = parseArray(split, ind+1, &arr, year.Ir);
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

	// Failure times
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
	readEmpty(file);

	// End Time
	mixed->T = stod(readLine(file)[0]);

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

		int planned = 0, repairs = 0, react = 0;
		for (int i = 0; i < data->Ip; ++i)
			planned += sol->getPlanned()[m][i];
		for (int i = 0; i < data->Ir; ++i)
		{
			repairs += sol->getRepairs()[sig][m][i];
			react += sol->getReactive()[sig][m][i];
		}

		int Im = planned + repairs + react;
		int I = Im + data->IInst[m];

		MonthData month = MonthData(data->Y, VyTotal, Im, data->IInst[m]);

		// Vessels
		for (int y = 0; y < data->Y; ++y)
		{
			month.Vy[y] = Vy[y];

			for (int i = 0; i < planned; ++i) // Planned tasks
			{
				month.s[y][i] = data->sP[y];
				month.d[y][i] = data->dPy[y];
				month.rho[y][i] = data->rhoP[y];
			}

			int repDone = 0, reactDone = 0;
			for (int ir = 0; ir < data->Ir; ++ir) // Failure TYPES
			{
				for (int i = 0; i < sol->getRepairs()[sig][m][ir]; ++i) // Repair tasks
				{
					month.s[y][i + planned + repDone] = data->sR[y][ir];
					month.d[y][i + planned + repDone] = data->dRy[y][ir];
					month.rho[y][i + planned + repDone] = data->rhoR[y][ir];
				}
				repDone += sol->getRepairs()[sig][m][ir];

				for (int i = 0; i < sol->getReactive()[sig][m][ir]; ++i) // Reactive tasks
				{
					month.s[y][i + planned + repairs + reactDone] = data->sR[y][ir];
					month.d[y][i + planned + repairs + reactDone] = data->dRy[y][ir];
					month.rho[y][i + planned + repairs + reactDone] = data->rhoR[y][ir];
				}
				reactDone += sol->getReactive()[sig][m][ir];
			}

			for (int i = 0; i < data->IInst[m]; ++i) // Installation tasks
			{
				month.s[y][i + Im] = 0;
				month.d[y][i + Im] = data->dI[m][y][i];
			}

			for (int i = 0; i < Im; ++i)
				if (month.dMax[i] < month.s[y][i] + month.d[y][i])
					month.dMax[i] = month.s[y][i] + month.d[y][i];
		}

		// Costs per task
		for (int i = 0; i < planned; ++i) // Planned tasks
			month.c[i] = 0;
		for (int i = planned; i < Im; ++i) // Repair & reactive tasks
			month.c[i] = data->eH[m];

		// Release times
		for (int i = 0; i < planned + repairs; ++i) // Planned & repair tasks
			month.r[i] = 0;
		int offset = planned + repairs;
		for (int ir = 0; ir < data->Ir; ++ir)  // Reactive tasks
		{
			for (int i = 0; i < sol->getReactive()[sig][m][ir]; ++i)
				month.r[i + offset] = data->FTime[m][ir][i];
			offset += sol->getReactive()[sig][m][ir];
		}

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
			month.aInst[vTrans[data->aInst[m][i]]][i] = 1;

		// End Time
		month.T = data->T;
		month.M = month.T * 3;

		res.push_back(month);
	}

	return res;
}
