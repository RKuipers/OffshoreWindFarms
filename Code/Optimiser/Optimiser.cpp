#include <tuple>		// tuple
#include <iostream>		// cout
#include <math.h>		// round
#include <cmath>		// pow
#include <algorithm>    // max
#include <string>		// string, to_string
#include <fstream>		// ifstream, ofstream
#include <stdlib.h>     // srand, rand
#include <vector>		// vector
#include <ctime>		// clock
#include "xprb_cpp.h"

using namespace std;
using namespace ::dashoptimization;

// Program settings
#define SEED 42 * NTIMES
#define WEATHERTYPE 1
#define VERBOSITY 1
#define NAMES 1
#define DATAFILE "installSimple.dat"
#define OUTPUTFILE "install.sol"

// Model settings
#define NPERIODS 3
#define TPP 4 // Timesteps per Period
#define NTIMES NPERIODS * TPP
#define NTASKS 4
#define NIP 3
#define NRES 2
#define NASSETS 2
#define DIS 1.0

// Weather characteristics
int base = 105;
int variety = 51;
int bonus = -25;

int mode = 2;

// Model parameters
int OMEGA[NTASKS][NTIMES];
int v[NPERIODS];
int C[NRES][NPERIODS];
int d[NTASKS];
int rho[NRES][NTASKS];
int m[NRES][NPERIODS];
tuple<int, int> IP[NIP];

// Model variables
XPRBvar O[NPERIODS];
XPRBvar N[NRES][NPERIODS];
XPRBvar n[NRES][NTIMES];
XPRBvar s[NASSETS][NTASKS][NTIMES];
XPRBvar f[NASSETS][NTASKS][NTIMES];

XPRBprob prob("Install1"); // Initialize a new problem in BCL

void generateWeather(ifstream* datafile)
{
	string line;
	getline(*datafile, line);
	if (line.compare("WAVEHEIGHT LIMITS") != 0)
		cout << "Error reading WAVEHEIGHT LIMITS" << endl;

	int limits[NTASKS];
	for (int i = 0; i < NTASKS; ++i)
	{
		getline(*datafile, line);
		limits[i] = stoi(line);
	}

	int waveHeight[NTIMES];
	if (WEATHERTYPE == 0)
	{
		waveHeight[0] = 120; 
		if (VERBOSITY > 1)
			cout << 0 << ": " << waveHeight[0] << endl;
		for (int t = 1; t < NTIMES; ++t)
		{
			bonus += (base - waveHeight[t - 1]) / 40;

			waveHeight[t] = max(0, waveHeight[t - 1] + bonus + (rand() % variety));
			if (VERBOSITY > 1)
				cout << t << ": " << waveHeight[t] << endl;
		}
	}
	else if (WEATHERTYPE == 1)
	{
		for (int p = 0; p < NPERIODS; ++p)
		{
			waveHeight[p * TPP] = 130;
			if (VERBOSITY > 1)
				cout << p * TPP << ": " << waveHeight[p * TPP] << endl;
			for (int t = (p * TPP) + 1; t < (p + 1) * TPP; ++t)
			{
				waveHeight[t] = max(0, waveHeight[t - 1] + bonus +  (rand() % variety));
				if (VERBOSITY > 1)
					cout << t << ": " << waveHeight[t] << endl;
			}
		}
	}

	for (int i = 0; i < NTASKS; ++i)
		for (int t = 0; t < NTIMES; ++t)
		{
			if (waveHeight[t] < limits[i])
				OMEGA[i][t] = 1;
			else
				OMEGA[i][t] = 0;
		}

	getline(*datafile, line);
}

void readList(ifstream* datafile, string name, int list[], int n)
{
	string line;
	getline(*datafile, line);
	if (line.compare(name) != 0)
		cout << "Error reading " << name << endl;

	for (int i = 0; i < n; ++i)
	{
		getline(*datafile, line);
		list[i] = stoi(line);
	}

	getline(*datafile, line);
}

void readLine(ifstream* datafile, int list[], int n)
{
	string line;
	getline(*datafile, line);
	vector<string> parsed;

	size_t current, previous = 0;
	current = line.find(' ');
	while (current <= line.size()) {
		parsed.push_back(line.substr(previous, current - previous));
		previous = current + 1;
		current = line.find(' ', previous);
	}
	parsed.push_back(line.substr(previous, current - previous));

	for (int i = 0; i < n; ++i)
		list[i] = stoi(parsed[i]);
}

void readIP(ifstream* datafile)
{
	string line;
	getline(*datafile, line);
	if (line.compare("PREREQUISITES") != 0)
		cout << "Error reading PREREQUISITES" << endl;

	for (int i = 0; i < NIP; ++i)
	{
		getline(*datafile, line);
		size_t split = line.find(' ');
		IP[i] = make_tuple(stoi(line.substr(0, split)), stoi(line.substr(split + 1, line.length())));
	}

	getline(*datafile, line);
}

void readData()
{
	string line;
	ifstream datafile(DATAFILE);
	if (!datafile.is_open())
	{
		cout << "Unable to open file" << endl;
		return;
	}

	/* Reads limits (weather limits of tasks), 
	generates a weather series, 
	and fills in OMEGA (weather a task can be performed in a certain timestep) */
	generateWeather(&datafile);

	// Reads v (values of energy)
	readList(&datafile, "VALUES", v, NPERIODS);

	// Reads C (costs of resources)
	getline(datafile, line);
	if (line.compare("COSTS") != 0)
		cout << "Error reading COSTS" << endl;
	for (int r = 0; r < NRES; ++r)
		readLine(&datafile, C[r], NPERIODS);
	getline(datafile, line);

	// Reads d (durations of tasks)
	readList(&datafile, "DURATIONS", d, NTASKS);

	// Reads IP (prerequisite tasks)
	readIP(&datafile);

	// Reads rho (required resources)
	getline(datafile, line);
	if (line.compare("RESOURCES") != 0)
		cout << "Error reading RESOURCES" << endl;
	for (int r = 0; r < NRES; ++r)
		readLine(&datafile, rho[r], NTASKS);
	getline(datafile, line);

	// Reads m (maximum chartered resources)
	getline(datafile, line);
	if (line.compare("MAX RESOURCES") != 0)
		cout << "Error reading MAX RESOURCES" << endl;
	for (int r = 0; r < NRES; ++r)
		readLine(&datafile, m[r], NPERIODS);
	getline(datafile, line);

	datafile.close();
}

int main(int argc, char** argv)
{
	int a, p, r, i, x, t, t1, t2, t3;

	srand(SEED);

	// Read data from file
	if (VERBOSITY > 0)
		cout << "Reading Data" << endl;
	readData();

	if (NAMES == 0)
		prob.setDictionarySize(XPRB_DICT_NAMES, 0);

	clock_t start = clock();

	//---------------------------Decision Variables---------------------------
	if (VERBOSITY > 0)
		cout << "Initialising variables" << endl;

	// Create the period-based decision variables

	for (p = 0; p < NPERIODS; ++p)
	{
		O[p] = prob.newVar(("O_" + to_string(p)).c_str(), XPRB_UI);
		O[p].setLB(0);

		for (r = 0; r < NRES; ++r)
		{
			N[r][p] = prob.newVar(("N_" + to_string(r) + "_" + to_string(p)).c_str(), XPRB_UI);
			N[r][p].setLB(0);
			N[r][p].setUB(m[r][p]);
		}
	}

	// Create the timestep-based decision variables
	for (t = 0; t < NTIMES; ++t)
	{
		for (r = 0; r < NRES; ++r)
			n[r][t] = prob.newVar(("n_" + to_string(r) + "_" + to_string(t)).c_str(), XPRB_UI);

		for (a = 0; a < NASSETS; ++a)
			for (i = 0; i < NTASKS; ++i)
			{
				s[a][i][t] = prob.newVar(("s_" + to_string(a) + "_" + to_string(i) + "_" + to_string(t)).c_str(), XPRB_BV);
				f[a][i][t] = prob.newVar(("f_" + to_string(a) + "_" + to_string(i) + "_" + to_string(t)).c_str(), XPRB_BV);
			}
	}

	//--------------------------------Objective--------------------------------
	if (VERBOSITY > 0)
		cout << "Initialising objective" << endl;

	XPRBctr Obj = prob.newCtr();
	for (p = 0; p < NPERIODS; ++p)
	{
		double dis = pow(DIS, p);
		Obj.addTerm(O[p], v[p] * dis);

		for (r = 0; r < NRES; ++r)
			Obj.addTerm(N[r][p], -C[r][p] * dis);
	}
	prob.setObj(Obj); // Set the objective function

	//-------------------------------Constraints-------------------------------
	if (VERBOSITY > 0)
		cout << "Initialising constraints" << endl;

	// Forces every task to start and end
	for (a = 0; a < NASSETS; ++a)
		for (i = 0; i < NTASKS; ++i)
		{
			XPRBctr ctrs, ctrf;
			if (NAMES == 0)
			{
				ctrs = prob.newCtr(s[a][i][0] == 1);
				ctrf = prob.newCtr(f[a][i][0] == 1);
			}
			else
			{
				ctrs = prob.newCtr(("Sta_" + to_string(a) + "_" + to_string(i)).c_str(), s[a][i][0] == 1);
				ctrf = prob.newCtr(("Fin_" + to_string(a) + "_" + to_string(i)).c_str(), f[a][i][0] == 1);
			}

			for (t = 1; t < NTIMES; ++t)
			{
				ctrs.addTerm(s[a][i][t]);
				ctrf.addTerm(f[a][i][t]);
			}			
		}

	// Precedence constraints
	for (a = 0; a < NASSETS; ++a)
		for (x = 0; x < NIP; ++x)
			for (t = 0; t < NTIMES; ++t)
			{
				if (mode <= 1)
				{
					XPRBctr ctr;
					int j;
					tie(i, j) = IP[x];

					if (NAMES == 0)
						ctr = prob.newCtr(s[a][j][t] <= f[a][i][t]);
					else
						ctr = prob.newCtr(("Prec_" + to_string(a) + "_" + to_string(x) + "_" + to_string(t)).c_str(), s[a][j][t] <= f[a][i][t]);

					for (t1 = 0; t1 < t; ++t1)
					{
						ctr.addTerm(s[a][j][t1]);
						ctr.addTerm(f[a][i][t1], -1);
					}
				}
				else if (mode >= 2)
				{
					for (t1 = t; t1 < NTIMES; ++t1)
					{
						XPRBctr ctr;
						int j;
						tie(i, j) = IP[x];

						if (NAMES == 0)
							ctr = prob.newCtr(s[a][j][t] + f[a][i][t1] <= 1);
						else
							ctr = prob.newCtr(("Prec_" + to_string(a) + "_" + to_string(x) + "_" + to_string(t) + "_" + to_string(t1)).c_str(), s[a][j][t] + f[a][i][t1] <= 1);
					}
				}
			}

	// Duration constraints
	for (a = 0; a < NASSETS; ++a)
		for (i = 0; i < NTASKS; ++i)
			for (t1 = 0; t1 < NTIMES; ++t1)
			{
				if (mode % 2 == 0)
				{
					int worked = 0, reald = 0;
					while (worked < d[i])
					{
						if (OMEGA[i][t1 + reald] == 1)
							worked++;
						reald++;
					}

					XPRBctr ctr;
					if (NAMES == 0)
						ctr = prob.newCtr(s[a][i][t1] <= f[a][i][t1 + reald - 1]);
					else
						ctr = prob.newCtr(("Dur_" + to_string(a) + "_" + to_string(i) + "_" + to_string(t1)).c_str(), s[a][i][t1] <= f[a][i][t1 + reald - 1]);
				}
				else if (mode % 2 == 1)
				{
					for (t2 = 0; t2 < NTIMES; ++t2)
					{
						int weather = 0;
						for (t3 = t1; t3 <= t2; ++t3)
							weather += OMEGA[i][t3];

						XPRBctr ctr;
						if (NAMES == 0)
							ctr = prob.newCtr((f[a][i][t2] + s[a][i][t1]) * 0.5 * weather >= d[i] * (s[a][i][t1] + f[a][i][t2] - 1));
						else
							ctr = prob.newCtr(("Dur_" + to_string(a) + "_" + to_string(i) + "_" + to_string(t1) + "_" + to_string(t2)).c_str(), (f[a][i][t2] + s[a][i][t1]) * 0.5 * weather >= d[i] * (s[a][i][t1] + f[a][i][t2] - 1));
					}
				}
			}

	// Resource amount link
	for (r = 0; r < NRES; ++r)
		for (p = 0; p < NPERIODS; ++p)
			for (t = p * TPP; t < (p + 1) * TPP; ++t)
			{
				XPRBctr ctr;
				if (t == 0)
				{
					if (NAMES == 0)
						ctr = prob.newCtr(n[r][t] == 0);
					else
						ctr = prob.newCtr(("Nee_" + to_string(r) + "_" + to_string(t)).c_str(), n[r][t] == 0);

					for (i = 0; i < NTASKS; ++i)
					{
						if (rho[r][i] == 0)
							continue;

						for (a = 0; a < NASSETS; a++)
							ctr.addTerm(s[a][i][t], -rho[r][i]);
					}
				}
				else
				{
					if (NAMES == 0)
						ctr = prob.newCtr(n[r][t] - n[r][t - 1] == 0);
					else
						ctr = prob.newCtr(("Nee_" + to_string(r) + "_" + to_string(t)).c_str(), n[r][t] - n[r][t - 1] == 0);

					for (i = 0; i < NTASKS; ++i)
					{
						if (rho[r][i] == 0)
							continue;

						for (a = 0; a < NASSETS; a++)
						{
							ctr.addTerm(s[a][i][t], -rho[r][i]);
							ctr.addTerm(f[a][i][t-1], rho[r][i]);
						}
					}
				}

				if (NAMES == 0)
					ctr = prob.newCtr(N[r][p] >= n[r][t]);
				else
					ctr = prob.newCtr(("Hav_" + to_string(r) + "_" + to_string(p) + "_" + to_string(t)).c_str(), N[r][p] >= n[r][t]);
			}

	// Online turbines link
	for (p = 0; p < NPERIODS; ++p)
	{
		XPRBctr ctr;
		if (NAMES == 0)
			ctr = prob.newCtr(NULL, O[p] == 0);
		else
			ctr = prob.newCtr(("Onl_" + to_string(p)).c_str(), O[p] == 0);

		for (t = 0; t < p * TPP; ++t)
			for (a = 0; a < NASSETS; a++)
				ctr.addTerm(f[a][NTASKS - 1][t], -1);
	}

	double duration = ((double)clock() - start) / (double)CLOCKS_PER_SEC;
	if (VERBOSITY > 0)
		cout << "Initialising duration: " << duration << " seconds" << endl;

	//---------------------------------Solving---------------------------------
	if (VERBOSITY > 0)
		cout << "Solving problem" << endl;
	start = clock();
	if (VERBOSITY == 0)
		prob.setMsgLevel(1);
	prob.setSense(XPRB_MAXIM);
	prob.exportProb(XPRB_LP, "Install1");
	prob.mipOptimize("");
	duration = ((double)clock() - start) / (double)CLOCKS_PER_SEC;
	cout << "Solving duration: " << duration << " seconds" << endl;

	const char* MIPSTATUS[] = { "not loaded", "not optimized", "LP optimized", "unfinished (no solution)", "unfinished (solution found)", "infeasible", "optimal", "unbounded" };
	cout << "Problem status: " << MIPSTATUS[prob.getMIPStat()] << endl;

	//----------------------------Solution printing----------------------------
	ofstream file;
	file.open(OUTPUTFILE);
	cout << "Total return: " << prob.getObjVal() << endl;
	file << "Objective: " << prob.getObjVal() << endl;

	cout << "Online turbines per period: " << endl;
	for (p = 0; p < NPERIODS; ++p)
	{
		int v = round(O[p].getSol());
		cout << p << ": " << v << endl;
		file << "O_" << p << ": " << v << endl;
	}

	cout << "Resources needed per period and type: " << endl;
	for (p = 0; p < NPERIODS; ++p)
	{
		if (VERBOSITY > 1)
			cout << "Period ";

		int v = round(N[0][p].getSol());
		cout << p << ": " << v;
		file << "N_0_" << p << ": " << v << endl;;

		for (r = 1; r < NRES; ++r)
		{
			v = round(N[r][p].getSol());
			cout << ", " << v;
			file << "N_" << r << "_" << p << ": " << v << endl;;
		}
		cout << endl;

		if (VERBOSITY > 1)
		{
			for (t = p * TPP; t < (p + 1) * TPP; ++t)
			{
				v = round(n[0][t].getSol());
				cout << t << ": " << v;
				file << "n_0_" << t << ": " << v << endl;;

				for (r = 1; r < NRES; ++r)
				{
					v = round(n[r][t].getSol());
					cout << ", " << v;
					file << "n_" << r << "_" << t << ": " << v << endl;;
				}
				cout << endl;
			}
		}
	}

	cout << "Start and finish time per asset and task: " << endl;
	for (a = 0; a < NASSETS; ++a)
	{
		cout << "Asset: " << a << endl;
		for (i = 0; i < NTASKS; ++i)
		{
			int start = -1;
			int finish = -1;

			for (t = 0; t < NTIMES; ++t)
			{
				int sv = round(s[a][i][t].getSol()); 
				int fv = round(f[a][i][t].getSol());

				file << "s_" << a << "_" << i << "_" << t << ": " << sv << endl;
				file << "f_" << a << "_" << i << "_" << t << ": " << fv << endl;

				if (sv == 1)
					start = t;
				if (fv == 1)
					finish = t;
			}
			cout << i << ": " << start << " " << finish << endl;
			file << "Asset " << a << " task " << i << ": " << start << " " << finish << endl;
		}
	}

	file.close();
	return 0;
}
