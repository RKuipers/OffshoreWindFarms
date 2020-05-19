#include <tuple>		// tuple
#include <iostream>		// cout
#include <cmath>		// pow
#include <algorithm>    // max
#include <string>		// string, to_string
#include <fstream>		// ifstream, ofstream
#include <stdlib.h>     // srand, rand
#include <vector>		// vector
#include "xprb_cpp.h"

using namespace std;
using namespace ::dashoptimization;

#define DATAFILE "install.dat"
#define OUTPUTFILE "install.sol"
#define NPERIODS 10
#define TPP 4 // Timesteps per Period
#define NTIMES NPERIODS * TPP
#define NTASKS 4
#define NIP 2
#define NRES 2
#define NASSETS 2
#define DIS 1.0

int OMEGA[NTASKS][NTIMES];
int v[NPERIODS];
int C[NRES][NPERIODS];
int d[NTASKS];
int rho[NRES][NTASKS];
int m[NRES][NPERIODS];
tuple<int, int> IP[NIP];

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
	waveHeight[0] = 140;
	for (int t = 1; t < NTIMES; ++t)
	{
		int bonus = -21;
		bonus += (100 - waveHeight[t - 1]) / 40;

		waveHeight[t] = max(0, waveHeight[t - 1] + bonus + (rand() % 51));
		cout << t << ": " << waveHeight[t] << endl;
	}


	for (int i = 0; i < NTASKS; ++i)
		for (int t = 1; t < NTIMES; ++t)
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

	XPRBexpr Obj;
	XPRBexpr TaskStart[NASSETS][NTASKS];
	XPRBexpr TaskFinish[NASSETS][NTASKS];
	XPRBexpr Prec[NASSETS][NIP][NTIMES][NTIMES];
	XPRBexpr Weat[NASSETS][NTASKS][NTIMES][NTIMES];
	XPRBexpr Res[NRES][NPERIODS][TPP];
	XPRBexpr Onl[NPERIODS];
	XPRBexpr Cos[NPERIODS];

	XPRBvar O[NPERIODS];
	XPRBvar N[NRES][NPERIODS];
	XPRBvar s[NASSETS][NTASKS][NTIMES];
	XPRBvar f[NASSETS][NTASKS][NTIMES];

	srand(69);

	// Read data from file
	readData();

	//---------------------------Decision Variables---------------------------

	// Create the period-based decision variables
	for (p = 0; p < NPERIODS; ++p)
	{
		O[p] = prob.newVar(("O_" + to_string(p)).c_str(), XPRB_UI);
		O[p].setLB(0);

		for (r = 0; r < NRES; ++r)
		{
			N[r][p] = prob.newVar(("N_" + to_string(r) + " " + to_string(p)).c_str(), XPRB_UI);
			N[r][p].setLB(0);
			N[r][p].setUB(m[r][p]);
		}
	}

	// Create the timestep-based decision variables
	for (a = 0; a < NASSETS; ++a)
		for (i = 0; i < NTASKS; ++i)
			for (t = 0; t < NTIMES; ++t)
			{
				s[a][i][t] = prob.newVar(("s_" + to_string(a) + "_" + to_string(i) + "_" + to_string(t)).c_str(), XPRB_BV);
				f[a][i][t] = prob.newVar(("f_" + to_string(a) + "_" + to_string(i) + "_" + to_string(t)).c_str(), XPRB_BV);
			}

	//--------------------------------Objective--------------------------------

	for (p = 0; p < NPERIODS; ++p)
	{
		for (r = 0; r < NRES; ++r)
			Cos[p] += N[r][p] * C[r][p];

		Obj += pow(DIS, p) * (O[p] * v[p] - Cos[p]);
	}
	prob.setObj(Obj); // Set the objective function

	//-------------------------------Constraints-------------------------------

	// Forces every task to start and end
	for (a = 0; a < NASSETS; ++a)
		for (i = 0; i < NTASKS; ++i)
		{
			for (t = 0; t < NTIMES; ++t)
			{
				TaskStart[a][i] += s[a][i][t];
				TaskFinish[a][i] += f[a][i][t];
			}
			prob.newCtr(("Sta_" + to_string(a) + "_" + to_string(i)).c_str(), TaskStart[a][i] == 1);
			prob.newCtr(("Fin_" + to_string(a) + "_" + to_string(i)).c_str(), TaskFinish[a][i] == 1);
		}

	// Precedence constraints
	for (a = 0; a < NASSETS; ++a)
		for (x = 0; x < NIP; ++x)
			for (t1 = 0; t1 < NTIMES; ++t1)
				for (t2 = t1; t2 < NTIMES; ++t2)
				{
					int j;
					tie(i, j) = IP[x];

					Prec[a][x][t1][t2] = s[a][j][t1] + f[a][i][t2];
					prob.newCtr(("Prec_" + to_string(a) + "_" + to_string(x) + "_" + to_string(t1) + "_" + to_string(t2)).c_str(), Prec[a][x][t1][t2] <= 1);
				}

	// Weather conditions
	for (a = 0; a < NASSETS; ++a)
		for (i = 0; i < NTASKS; ++i)
			for (t1 = 0; t1 < NTIMES; ++t1)
				for (t2 = 0; t2 < NTIMES; ++t2)
				{
					int weather = 0;
					for (t3 = t1; t3 <= t2; ++t3)
						weather += OMEGA[i][t3];

					Weat[a][i][t1][t2] += (f[a][i][t2] + s[a][i][t1]) * 0.5 * weather - d[i] * (s[a][i][t1] + f[a][i][t2] - 1);

					prob.newCtr(("Weat_" + to_string(a) + "_" + to_string(i) + "_" + to_string(t1) + "_" + to_string(t2)).c_str(), Weat[a][i][t1][t2] >= 0);
				}

	// Resource amount link
	for (r = 0; r < NRES; ++r)
		for (p = 0; p < NPERIODS; ++p)
			for (t = p * TPP; t < (p + 1) * TPP; ++t)
			{
				for (a = 0; a < NASSETS; a++)
					for (i = 0; i < NTASKS; ++i)
					{
						Res[r][p][t - p * TPP] += rho[r][i] * s[a][i][0];
						for (t1 = 1; t1 <= t; ++t1)
							Res[r][p][t - p * TPP] += rho[r][i] * (s[a][i][t1] - f[a][i][t1 - 1]);
					}

				prob.newCtr(("Res_" + to_string(r) + "_" + to_string(p) + "_" + to_string(t)).c_str(), Res[r][p][t - p * TPP] <= N[r][p]);
			}

	// Online turbines link
	for (p = 0; p < NPERIODS; ++p)
	{
		for (t = 0; t < p * TPP; ++t)
			for (a = 0; a < NASSETS; a++)
				Onl[p] += f[a][NTASKS - 1][t];

		prob.newCtr(("Onl_" + to_string(p)).c_str(), Onl[p] == O[p]);
	}

	//---------------------------------Solving---------------------------------
	//prob.setMsgLevel(1);
	prob.setSense(XPRB_MAXIM);
	prob.exportProb(XPRB_LP, "Install1");
	prob.mipOptimize("");

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
		cout << p << ": " << O[p].getSol() << endl;
		file << "O_" << p << ": " << O[p].getSol() << endl;
	}

	cout << "Resources needed per period and type: " << endl;
	for (p = 0; p < NPERIODS; ++p)
	{
		cout << p << ": " << N[0][p].getSol();
		file << "N_0_" << p << ": " << N[0][p].getSol() << endl;;


		for (r = 1; r < NRES; ++r)
		{
			cout << ", " << N[r][p].getSol();
			file << "N_" << r << "_" << p << ": " << N[r][p].getSol() << endl;;
		}
		cout << endl;
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
				double st = s[a][i][t].getSol(); 
				double fi = f[a][i][t].getSol();

				file << "s_" << a << "_" << i << "_" << t << ": " << st << endl;
				file << "f_" << a << "_" << i << "_" << t << ": " << fi << endl;

				if (a == 0 && i == 2 && (t == 27 || t == 28 || t == 30))
					t = t;

				if (st == 1)
					start = t;
				if (fi == 1)
					finish = t;
			}
			cout << i << ": " << start << " " << finish << endl;
			file << "Asset " << a << " task " << i << ": " << start << " " << finish << endl;
		}
	}

	file.close();
	return 0;
}
