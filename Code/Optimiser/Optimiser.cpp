#include <tuple>		// tuple
#include <iostream>		// cout
#include <math.h>		// round
#include <cmath>		// pow
#include <algorithm>    // max, count
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
#define NCUTMODES 1
#define NMODES NCUTMODES // Product of all mode types
#define NSETTINGS NCUTMODES // Sum of all mode types
#define WEATHERTYPE 1
#define CUTMODE 0
#define VERBOSITY 1
#define NAMES 1
#define OUTPUTFILE "install"
#define OUTPUTEXT ".sol"

// Model settings
#define DATAFILE "installWeek.dat"
#define NPERIODS 7
#define TPP 12 // Timesteps per Period
#define NTIMES NPERIODS * TPP
#define NTASKS 5
#define NIP 4
#define NRES 3
#define NASSETS 2
#define DIS 0.999972465
#define OPTIMAL -474814 // The optimal solution, if known

// Weather characteristics
int base = 105;
int variety = 51;
int bonus = -25;

// Model parameters
int OMEGA[NTASKS][NTIMES];
int v[NPERIODS];
int C[NRES][NPERIODS];
int d[NTASKS];
int sa[NTASKS][NTIMES + 1];
int rho[NRES][NTASKS];
int m[NRES][NPERIODS];
tuple<int, int> IP[NIP];

// Model variables
XPRBvar O[NPERIODS];
XPRBvar N[NRES][NPERIODS];
XPRBvar n[NRES][NTIMES];
XPRBvar s[NASSETS][NTASKS][NTIMES];

class Mode 
{
	class ModeDim
	{
	protected:
		int curr, max;

	public:
		ModeDim(int max = 2)
		{
			this->curr = 0;
			this->max = max;
		}

		int next() 
		{
			if (curr + 1 < max)
			{
				++curr;
				return curr;
			}
			else
			{
				curr = 0;
				return -1;
			}
		}

		int getCurr()
		{
			return curr;
		}
		
		/*string getName(int index = -1)
		{
			if (index == -1)
				return names[curr];
			else
				return names[index];
		}*/
	};

	class ModeDimComb : ModeDim
	{
	private:
		ModeDim* dims;
	public:
		ModeDimComb(int dims)
		{
			this->dims[dims];
			for (int i = 0; i < dims; ++i)
				this->dims[i] = ModeDim(2);
		}
	};

private:
	int index;
	enum cut {Set = 0, Prec = 1, Res = 2, Onl = 3};
	enum test { A, B, C, NTEST};
	enum direction { East = 11, West = 22, North = 33, South = 44 };

public:
	void Next()
	{
		direction dir;
		dir = South;

		cut c;
		c = Prec;
	}
};

class OutputPrinter
{
private:
	void printObj(ofstream* file, XPRBprob* prob)
	{
		cout << "Total return: " << prob->getObjVal() << endl;
		*file << "Objective: " << prob->getObjVal() << endl;
	}

	void printTurbines(ofstream* file)
	{
		cout << "Online turbines per period: " << endl;
		for (int p = 0; p < NPERIODS; ++p)
		{
			int v = round(O[p].getSol());
			cout << p << ": " << v << endl;
			*file << "O_" << p << ": " << v << endl;
		}
	}

	void printResources(ofstream* file)
	{
		cout << "Resources needed per period and type: " << endl;
		for (int p = 0; p < NPERIODS; ++p)
		{
			printer("Period ", 2, false);

			int v = round(N[0][p].getSol());
			cout << p << ": " << v;
			*file << "N_0_" << p << ": " << v << endl;;

			for (int r = 1; r < NRES; ++r)
			{
				v = round(N[r][p].getSol());
				cout << ", " << v;
				*file << "N_" << r << "_" << p << ": " << v << endl;;
			}
			cout << endl;

			if (VERBOSITY > 1) // Also print resources needed per timestep
			{
				for (int t = p * TPP; t < (p + 1) * TPP; ++t)
				{
					v = round(n[0][t].getSol());
					cout << t << ": " << v;
					*file << "n_0_" << t << ": " << v << endl;;

					for (int r = 1; r < NRES; ++r)
					{
						v = round(n[r][t].getSol());
						cout << ", " << v;
						*file << "n_" << r << "_" << t << ": " << v << endl;;
					}
					cout << endl;
				}
			}
		}
	}

	void printTasks(ofstream* file, bool newVar)
	{
		cout << "Start and finish time per asset and task: " << endl;
		for (int a = 0; a < NASSETS; ++a)
		{
			cout << "Asset: " << a << endl;
			for (int i = 0; i < NTASKS; ++i)
			{
				int start = -1;
				int finish = -1;

				for (int t = 0; t < NTIMES; ++t)
				{
					int sv = round(s[a][i][t].getSol());

					*file << "s_" << a << "_" << i << "_" << t << ": " << sv << endl;

					if (sv == 1 && start == -1)
						start = t;
				}

				for (int t1 = start + d[i] - 1; t1 <= NTIMES; ++t1)
					if (sa[i][t1] >= start)
					{
						finish = t1 - 1;
						break;
					}

				cout << i << ": " << start << " " << finish << endl;
				*file << "Asset " << a << " task " << i << ": " << start << " " << finish << endl;
			}
		}
	}

public:
	void printProbOutput(XPRBprob* prob, int mode)
	{
		if (prob->getProbStat() == 1)
			return;

		ofstream file;
		file.open(OUTPUTFILE + to_string(mode) + OUTPUTEXT);

		printObj(&file, prob);
		printTurbines(&file);
		printResources(&file);
		printTasks(&file,mode % 2 == 0);

		file.close();
	}

	void printModeOutput(double* durs, bool opt)
	{
		cout << "----------------------------------------------------------------------------------------" << endl;

#ifdef OPTIMAL
		if (opt)
			cout << "All solutions are optimal" << endl;
		else
			cout << "Not all solutions are optimal" << endl;
#endif // OPTIMAL

		double tots[NSETTINGS];
		for (int i = 0; i < NSETTINGS; ++i)
			tots[i] = 0.0;

		for (int i = 0; i < NMODES; ++i)
		{
			cout << "MODE: " << i << " DUR: " << durs[i] << endl;

			int cutSetting = i % NCUTMODES;
			//int decSetting = i >> 5;

			tots[cutSetting] += durs[i];
			//tots[decSetting + NCUTMODES] += durs[i];
		}

		/*for (int i = 0; i < NCUTMODES; ++i)
			cout << "SETTING: " << i << " DUR: " << tots[i] / (NMODES / NCUTMODES) << endl;

#if NCUTMODES != NMODES
		for (int i = NCUTMODES; i < NDECVAR + NCUTMODES; ++i)
			cout << "SETTING: " << i << " DUR: " << tots[i] / (NMODES / NDECVAR) << endl;
#endif*/
	}

	int printer(string s, int verbosity, bool end = true)
	{
		if (VERBOSITY >= verbosity)
		{
			cout << s;
			if (end)
				cout << endl;
			return true;
		}
		return false;
	}
};

OutputPrinter outputPrinter;

class DataReaderNEW
{
private:
	vector<int> limits;

	void splitString(string s, vector<string>* res, char sep = ' ')
	{
		res->clear();
		size_t l = 0;

		while (s.find(sep, l) != string::npos)
		{
			size_t t = s.find(sep, l);
			res->push_back(s.substr(l, t - l));
			l = t + 1;
		}

		res->push_back(s.substr(l));
	}

	size_t parsePeriodical(char type, string line, size_t start, vector<int>* res)
	{
		// TODO

		// Switch based on 3 types: U (universal value), I (intervals), S (single values)

		// Possibly change parameter types and stuff

		return 0;
	}

	void readTasks(ifstream* datafile)
	{
		string line;
		vector<string>* split = new vector<string>();

		getline(*datafile, line);
		splitString(line, split);
		if ((*split)[0].compare("TASKS") != 0)
			cout << "Error reading TASKS" << endl;
		if (stoi((*split)[1]) != NTASKS)
			cout << "Error with declared TASKS amount" << endl;

		for (int i = 0; i < NTASKS; ++i)
		{
			getline(*datafile, line);
			splitString(line, split, '\t');

			if (count(line.begin(), line.end(), '\t') != 2 + NRES)
				cout << "Error with column count on TASKS line " << i << endl;

			d[i] = stoi((*split)[1]);
			limits[i] = stoi((*split)[2]);

			for (int r = 0; r < NRES; ++r)
				rho[r][i] = stoi((*split)[r+3]);
		}

		getline(*datafile, line);
	}

	void readResources(ifstream* datafile)
	{
		// TODO
	}

	void readValues(ifstream* datafile)
	{
		// TODO
	}

	void readPreqs(ifstream* datafile)
	{
		// TODO
	}

	// TODO: Check which of these old functions are of use
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
			waveHeight[0] = base;

			outputPrinter.printer("0: " + to_string(waveHeight[0]), 2);
			for (int t = 1; t < NTIMES; ++t)
			{
				bonus += (base - waveHeight[t - 1]) / 40;

				waveHeight[t] = max(0, waveHeight[t - 1] + bonus + (rand() % variety));
				outputPrinter.printer(to_string(t) + ": " + to_string(waveHeight[t]), 2);
			}
		}
		else if (WEATHERTYPE == 1)
		{
			for (int p = 0; p < NPERIODS; ++p)
			{
				waveHeight[p * TPP] = base;
				outputPrinter.printer(to_string(p * TPP) + ": " + to_string(waveHeight[p * TPP]), 2);
				for (int t = (p * TPP) + 1; t < (p + 1) * TPP; ++t)
				{
					waveHeight[t] = max(0, waveHeight[t - 1] + bonus + (rand() % variety));
					outputPrinter.printer(to_string(t) + ": " + to_string(waveHeight[t]), 2);
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

	void generateStartAtValues()
	{
		for (int i = 0; i < NTASKS; ++i)
			for (int t1 = 0; t1 <= NTIMES; ++t1)
			{
				int worked = 0;
				int t2;
				for (t2 = t1 - 1; worked < d[i] && t2 >= 0; --t2)
					if (OMEGA[i][t2] == 1)
						worked++;

				if (worked == d[i])
					sa[i][t1] = t2 + 1;
				else
					sa[i][t1] = -1;
			}
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

public:
	DataReaderNEW()
	{
		limits = vector<int>(NTASKS);
	}

	void readData()
	{
		// Read data from file
		outputPrinter.printer("Reading Data", 1);

		string line;
		ifstream datafile(DATAFILE);
		if (!datafile.is_open())
		{
			cout << "Unable to open file" << endl;
			return;
		}

		// Read the task info
		readTasks(&datafile);

		datafile.close();
	}

};

class DataReader
{
private:
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
			waveHeight[0] = base;

			outputPrinter.printer("0: " + to_string(waveHeight[0]), 2);
			for (int t = 1; t < NTIMES; ++t)
			{
				bonus += (base - waveHeight[t - 1]) / 40;

				waveHeight[t] = max(0, waveHeight[t - 1] + bonus + (rand() % variety));
				outputPrinter.printer(to_string(t) + ": " + to_string(waveHeight[t]), 2);
			}
		}
		else if (WEATHERTYPE == 1)
		{
			for (int p = 0; p < NPERIODS; ++p)
			{
				waveHeight[p * TPP] = base;
				outputPrinter.printer(to_string(p * TPP) + ": " + to_string(waveHeight[p * TPP]), 2);
				for (int t = (p * TPP) + 1; t < (p + 1) * TPP; ++t)
				{
					waveHeight[t] = max(0, waveHeight[t - 1] + bonus + (rand() % variety));
					outputPrinter.printer(to_string(t) + ": " + to_string(waveHeight[t]), 2);
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

	void generateStartAtValues()
	{
		for (int i = 0; i < NTASKS; ++i)
			for (int t1 = 0; t1 <= NTIMES; ++t1)
			{
				int worked = 0;
				int t2;
				for (t2 = t1 - 1; worked < d[i] && t2 >= 0; --t2)
					if (OMEGA[i][t2] == 1)
						worked++;

				if (worked == d[i])
					sa[i][t1] = t2 + 1;
				else
					sa[i][t1] = -1;
			}
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

public:
	void readData()
	{
		// Read data from file
		outputPrinter.printer("Reading Data", 1);

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
		generateStartAtValues();

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

};

class ProblemGen
{
private:

	XPRBctr genCon(XPRBprob* prob, const XPRBrelation& ac, string base, int nInd, int* indices)
	{
		if (NAMES == 0)
			return prob->newCtr(ac);
		else
		{
			string name = base;

			for (int i = 0; i < nInd; ++i)
				name += "_" + to_string(indices[i]);

			return prob->newCtr(name.c_str(), ac);
		}
	}

	void genDecisionVariables(XPRBprob* prob)
	{
		outputPrinter.printer("Initialising variables", 1);

		// Create the period-based decision variables
		for (int p = 0; p < NPERIODS; ++p)
		{
			O[p] = prob->newVar(("O_" + to_string(p)).c_str(), XPRB_UI);
			O[p].setLB(0);

			for (int r = 0; r < NRES; ++r)
			{
				N[r][p] = prob->newVar(("N_" + to_string(r) + "_" + to_string(p)).c_str(), XPRB_UI);
				N[r][p].setLB(0);
				N[r][p].setUB(m[r][p]);
			}
		}

		// Create the timestep-based decision variables
		for (int t = 0; t < NTIMES; ++t)
		{
			for (int r = 0; r < NRES; ++r)
				n[r][t] = prob->newVar(("n_" + to_string(r) + "_" + to_string(t)).c_str(), XPRB_UI);

			for (int a = 0; a < NASSETS; ++a)
				for (int i = 0; i < NTASKS; ++i)
					s[a][i][t] = prob->newVar(("s_" + to_string(a) + "_" + to_string(i) + "_" + to_string(t)).c_str(), XPRB_BV);
		}
	}

	void genObjective(XPRBprob* prob)
	{
		outputPrinter.printer("Initialising objective", 1);

		XPRBctr Obj = prob->newCtr();
		for (int p = 0; p < NPERIODS; ++p)
		{
			double dis = pow(DIS, p);
			Obj.addTerm(O[p], v[p] * dis);

			for (int r = 0; r < NRES; ++r)
				Obj.addTerm(N[r][p], -C[r][p] * dis);
		}
		prob->setObj(Obj); // Set the objective function
	}

	void genSetConstraints(XPRBprob* prob, bool cut)
	{
		// Forces every task to start and end
		for (int a = 0; a < NASSETS; ++a)
		{
			for (int t = 1; t < NTIMES; ++t)
				for (int i = 0; i < NTASKS; ++i)
				{
					XPRBrelation rel = s[a][i][t] >= s[a][i][t - 1];

					if (cut)
						prob->newCut(rel);
					else
					{
						int indices[3] = { a, i, t };
						genCon(prob, rel, "Set", 3, indices);
					}
				}

			XPRBrelation rel = s[a][NTASKS - 1][sa[NTASKS - 1][NTIMES]] == 1;

			if (cut)
				prob->newCut(rel);
			else
			{
				int indices[3] = { a };
				genCon(prob, rel, "Fin", 3, indices);
			}
		}
	}

	void genPrecedenceConstraints(XPRBprob* prob, bool cut)
	{
		// Precedence constraints
		for (int a = 0; a < NASSETS; ++a)
			for (int x = 0; x < NIP; ++x)
				for (int t = 0; t < NTIMES; ++t)
				{
					int i, j;
					tie(i, j) = IP[x];

					XPRBrelation rel = s[a][i][sa[i][t]] >= s[a][j][t];
					if (sa[i][t] == -1)
					{
						s[a][j][t].setUB(0);
						continue;
					}

					if (cut)
						prob->newCut(rel);
					else
					{
						int indices[3] = { a, x, t };
						genCon(prob, rel, "Pre", 3, indices);
					}
				}
	}

	void genResourceConstraints(XPRBprob* prob, bool cut)
	{
		// Resource amount link
		for (int r = 0; r < NRES; ++r)
			for (int p = 0; p < NPERIODS; ++p)
				for (int t = p * TPP; t < (p + 1) * TPP; ++t)
				{
					XPRBrelation rel = N[r][p] >= 0;

					for (int i = 0; i < NTASKS; ++i)
					{
						if (rho[r][i] == 0)
							continue;

						for (int a = 0; a < NASSETS; a++)
						{
							rel.addTerm(s[a][i][t], -rho[r][i]);
							if (t > 0)
								if (sa[i][t] > -1)
									rel.addTerm(s[a][i][sa[i][t]], rho[r][i]);
								else
									continue;
						}
					}

					if (cut)
						prob->newCut(rel);
					else
					{
						int indices[3] = { r, p, t };
						genCon(prob, rel, "Nee", 3, indices);
					}
				}
	}

	void genOnlineConstraints(XPRBprob* prob, bool cut)
	{
		// Online turbines link
		O[0].setUB(0);

		for (int p = 1; p < NPERIODS; ++p)
		{
			XPRBrelation rel = O[p] == 0;

			for (int a = 0; a < NASSETS; a++)
				if (sa[NTASKS - 1][p * TPP] > -1)
					rel.addTerm(s[a][NTASKS - 1][sa[NTASKS - 1][p * TPP]], -1);
				else
					continue;

			if (cut)
				prob->newCut(rel);
			else
			{
				int indices[1] = { p };
				genCon(prob, rel, "Onl", 1, indices);
			}
		}
	}

public:
	void genOriProblem(XPRBprob* prob, int mode)
	{
		clock_t start = clock();

		genDecisionVariables(prob);
		genObjective(prob);

		outputPrinter.printer("Initialising Original constraints", 1);

		genSetConstraints(prob, mode % 2 == 1);
		genPrecedenceConstraints(prob, (mode >> 1) % 2 == 1);
		genResourceConstraints(prob, (mode >> 2) % 2 == 1);
		genOnlineConstraints(prob, (mode >> 3) % 2 == 1);

		double duration = ((double)clock() - start) / (double)CLOCKS_PER_SEC;
		outputPrinter.printer("Duration of initialisation: " + to_string(duration) + " seconds", 1);
	}

	void genFullProblem(XPRBprob* prob, int mode)
	{
		clock_t start = clock();

		outputPrinter.printer("Initialising Full constraints", 1);

		if (mode % 2 == 1)
			genSetConstraints(prob, false); 
		if ((mode >> 1) % 2 == 1)
			genPrecedenceConstraints(prob, false);
		if ((mode >> 2) % 2 == 1)
			genResourceConstraints(prob, false);
		if ((mode >> 3) % 2 == 1)
			genOnlineConstraints(prob, false);

		double duration = ((double)clock() - start) / (double)CLOCKS_PER_SEC;
		outputPrinter.printer("Duration of initialisation: " + to_string(duration) + " seconds", 1);
	}
};

class ProblemSolver
{
public:
	void solveProblem(XPRBprob* prob, string name)
	{
		outputPrinter.printer("Solving problem", 1);
		if (VERBOSITY == 0)
			prob->setMsgLevel(1);

		clock_t start = clock();

		prob->setSense(XPRB_MAXIM);
		prob->exportProb(XPRB_LP, name.c_str());
		prob->mipOptimize("");

		double duration = ((double)clock() - start) / (double)CLOCKS_PER_SEC;
		cout << "Solving duration: " << duration << " seconds" << endl;

		const char* MIPSTATUS[] = { "not loaded", "not optimized", "LP optimized", "unfinished (no solution)", "unfinished (solution found)", "infeasible", "optimal", "unbounded" };
		cout << "Problem status: " << MIPSTATUS[prob->getMIPStat()] << endl;
	}
};

// Program Objects
DataReader dataReader;
ProblemGen problemGen;
ProblemSolver problemSolver;

int main(int argc, char** argv)
{
	dataReader = DataReader();
	problemGen = ProblemGen();
	problemSolver = ProblemSolver();
	outputPrinter = OutputPrinter();

	XPRBprob probs[NMODES];		// Initialize new problems in BCL
	double durs[NMODES];
	bool opt = true;

	srand(SEED);

	dataReader.readData();

	for (int mode = 0; mode < NMODES; ++mode)
	{
#if NMODES == 1
		int realMode = 0;
#endif // NMODES == 1
#if NMODES > 1
		int realMode = mode;
#endif // NMODES > 1

		cout << "----------------------------------------------------------------------------------------" << endl;
		cout << "MODE: " << realMode << endl;

		string name = "Install" + to_string(realMode);
		probs[mode].setName(name.c_str());

		if (NAMES == 0)
			probs[mode].setDictionarySize(XPRB_DICT_NAMES, 0);

		clock_t start = clock();

		problemGen.genOriProblem(&probs[mode], realMode);
		problemSolver.solveProblem(&probs[mode], name);
#if CUTMODE == 0
		if (false)
		{
			problemGen.genFullProblem(&probs[mode], realMode);
			problemSolver.solveProblem(&probs[mode], name);
		}
#endif // CUTMODE == 0
#if CUTMODE == 1
		for (int i = 0; i < 5; ++i)
		{
			if (problemGen.addCtr(&probs[mode], realMode, i))
				problemSolver.solveProblem(&probs[mode], name);
		}
#endif // CUTMODE == 1
		outputPrinter.printProbOutput(&probs[mode], realMode);

#ifdef OPTIMAL
		opt &= round(probs[mode].getObjVal()) == OPTIMAL;
#endif // OPTIMAL

		double duration = ((double)clock() - start) / (double)CLOCKS_PER_SEC;
		cout << "FULL duration: " << duration << " seconds" << endl;
		durs[mode] = duration;

		probs[mode].reset();
	}

	outputPrinter.printModeOutput(durs, opt);

	return 0;
}
