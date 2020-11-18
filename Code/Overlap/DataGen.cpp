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
	// TODO: Update I with an easier format (I a1 v1 a2 v2 where a is amount) (do for parseArrayDouble too)
	case 'I': // I s1 e1 v1 s2 e2 v2 ... sn en vn -> Periods sx (inclusive) through ex (exclusive) use vx, s1 through en should cover all periods
	{
		int filled = 0;
		int loc = start + 1;
		while (filled < amount)
		{
			int intBeg = stoi(line[loc]);
			int intEnd = stoi(line[loc + 1]);
			int val = stoi(line[loc + 2]);

			fill(res->begin() + intBeg, res->begin() + intEnd, val);
			filled += intEnd - intBeg;
			loc += 3;
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
	case 'I': // I s1 e1 v1 s2 e2 v2 ... sn en vn -> Periods sx (inclusive) through ex (exclusive) use vx, s1 through en should cover all periods
	{
		int filled = 0;
		int loc = start + 1;
		while (filled < amount)
		{
			int intBeg = stoi(line[loc]);
			int intEnd = stoi(line[loc + 1]);
			double val = stod(line[loc + 2]);

			fill(res->begin() + intBeg, res->begin() + intEnd, val);
			filled += intEnd - intBeg;
			loc += 3;
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

YearData DataGen::readYear(ifstream* file)
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
	YearData year = YearData(S, M, Y, Ip, Ir);
	readEmpty(file);

	// Vessels
	for (int y = 0; y < Y; ++y)
	{
		split = readLine(file);
		int ind = 2;
		year.L[y] = stoi(split[1]);

		ind = parseArrayDouble(split, ind, &arrD, M);
		for (int m = 0; m < M; ++m)
			year.c[y][m] = arrD[m];

		ind = parseArray(split, ind, &arr, M);
		for (int m = 0; m < M; ++m)
			year.A[y][m] = arr[m];

		year.dPy[y] = stod(split[ind]);
		ind++;

		ind = parseArrayDouble(split, ind, &arrD, Ir);
		for (int i = 0; i < Ir; ++i)
			year.dR[y][i] = arrD[i];

		ind = parseArray(split, ind, &arr, M);
		for (int m = 0; m < M; ++m)
			year.LInst[y][m] = arr[m];

		ind = parseArray(split, ind, &arr, M);
		for (int m = 0; m < M; ++m)
			year.NInst[y][m] = arr[m];
	}
	readEmpty(file);

	// Offline duration for planned task
	year.dP = stoi(readLine(file)[0]);
	readEmpty(file);

	// Hours in given month
	parseArray(readLine(file), 0, &arr, M);
	for (int m = 0; m < M; ++m)
		year.H[m] = arr[m];
	readEmpty(file);

	// Energy$ per hour in given month
	parseArrayDouble(readLine(file), 0, &arrD, M);
	for (int m = 0; m < M; ++m)
		year.eH[m] = arrD[m];
	readEmpty(file);

	// Min/Max months between maint
	split = readLine(file);
	year.Gmin = stoi(split[0]);
	year.Gmax = stoi(split[1]);
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
				year.f[m][i][s] = arr[m];
		}
	}

	return year;
}

MonthData DataGen::readMonth(ifstream* file)
{
	vector<int> arr = vector<int>();
	vector<double> arrD = vector<double>();

	// Set sizes
	vector<string> split = readLine(file);
	int Y = stoi(split[0]);
	int V = stoi(split[1]);
	int IMaint = stoi(split[2]);
	int IInst = stoi(split[3]);
	int J = stoi(split[4]);
	MonthData month = MonthData(Y, V, IMaint, IInst, J);
	readEmpty(file);

	int I = IMaint + IInst;
	int VyTotal = 0;
	// Vessels
	for (int y = 0; y < Y; ++y)
	{
		split = readLine(file);
		int ind = 2;
		month.Vy[y] = stoi(split[1]) + VyTotal;
		VyTotal += month.Vy[y];

		ind = parseArrayDouble(split, ind, &arrD, I);
		for (int i = 0; i < I; ++i)
			month.s[y][i] = arrD[i];

		ind = parseArrayDouble(split, ind, &arrD, I);
		for (int i = 0; i < I; ++i)
			month.d[y][i] = arrD[i];

		ind = parseArray(split, ind, &arr, IMaint);
		for (int i = 0; i < IMaint; ++i)
			month.rho[y][i] = arr[i];
	}
	readEmpty(file);

	// Costs per task
	parseArrayDouble(readLine(file), 0, &arrD, IMaint);
	for (int i = 0; i < IMaint; ++i)
		month.c[i] = arrD[i];
	readEmpty(file);

	// Installation tasks
	for (int i = 0; i < IInst; ++i)
	{
		split = readLine(file);
		month.sInst[i] = stod(split[1]);
		int vessel = stoi(split[2]);
		for (int v = 0; v < V; ++v)
			month.aInst[v][i] = 0;
		month.aInst[vessel][i] = 1;
	}
	readEmpty(file);

	// End Time
	month.T = stod(readLine(file)[0]);
	month.M = 3 * month.T;

	return month;
}

vector<MonthData> DataGen::genMonths(YearSolution sol)
{
	return vector<MonthData>();
}
