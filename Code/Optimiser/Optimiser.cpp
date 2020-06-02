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
#define NMODES 24
#define WEATHERTYPE 1
#define VERBOSITY 0
#define NAMES 1
#define DATAFILE "installWeek.dat"
#define OUTPUTFILE "install.sol"

// Model settings
#define NPERIODS 7
#define TPP 12 // Timesteps per Period
#define NTIMES NPERIODS * TPP
#define NTASKS 5
#define NIP 4
#define NRES 3
#define NASSETS 2
#define DIS 0.99

// Weather characteristics
int base = 105;
int variety = 51;
int bonus = -25;

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

	void printTasks(ofstream* file)
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
					int fv = round(f[a][i][t].getSol());

					*file << "s_" << a << "_" << i << "_" << t << ": " << sv << endl;
					*file << "f_" << a << "_" << i << "_" << t << ": " << fv << endl;

					if (sv == 1)
						start = t;
					if (fv == 1)
						finish = t;
				}
				cout << i << ": " << start << " " << finish << endl;
				*file << "Asset " << a << " task " << i << ": " << start << " " << finish << endl;
			}
		}
	}

public:
	void printOutput(XPRBprob* prob)
	{
		ofstream file;
		file.open(OUTPUTFILE);

		printObj(&file, prob);
		printTurbines(&file);
		printResources(&file);
		printTasks(&file);

		file.close();
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
			waveHeight[0] = 120;

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
				waveHeight[p * TPP] = 130;
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
				{
					s[a][i][t] = prob->newVar(("s_" + to_string(a) + "_" + to_string(i) + "_" + to_string(t)).c_str(), XPRB_BV);
					f[a][i][t] = prob->newVar(("f_" + to_string(a) + "_" + to_string(i) + "_" + to_string(t)).c_str(), XPRB_BV);
				}
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

	void genSetConstraints(XPRBprob* prob, int mode, bool ctrForce = false)
	{
		// Forces every task to start and end
		for (int a = 0; a < NASSETS; ++a)
			for (int i = 0; i < NTASKS; ++i)
			{
				if (mode % 6 == 0 && !ctrForce)
				{
					XPRBcut ctrs, ctrf;
					int indices[2] = { a, i };
					ctrs = prob->newCut(s[a][i][0] == 1);
					ctrf = prob->newCut(f[a][i][0] == 1);

					for (int t = 1; t < NTIMES; ++t)
					{
						ctrs.addTerm(s[a][i][t]);
						ctrf.addTerm(f[a][i][t]);
					}
				}
				else
				{
					XPRBctr ctrs, ctrf;
					int indices[2] = { a, i };
					ctrs = genCon(prob, s[a][i][0] == 1, "Sta", 2, indices);
					ctrf = genCon(prob, f[a][i][0] == 1, "Fin", 2, indices);

					for (int t = 1; t < NTIMES; ++t)
					{
						ctrs.addTerm(s[a][i][t]);
						ctrf.addTerm(f[a][i][t]);
					}
				}
			}
	}

	void genPrecedenceConstraints(XPRBprob* prob, int mode, bool ctrForce = false)
	{
		int m1 = mode / 6;

		// Precedence constraints
		for (int a = 0; a < NASSETS; ++a)
			for (int x = 0; x < NIP; ++x)
				for (int t = 0; t < NTIMES; ++t)
				{
					if (m1 % 2 == 0) // sum_t1=t0^t s_a_j_t1 <= sum_t1=t0^t f_a_i_t1
					{
						if (mode % 6 == 1 && !ctrForce)
						{
							XPRBcut cut;
							int i, j;
							tie(i, j) = IP[x];

							cut = prob->newCut(s[a][j][t] <= f[a][i][t]);

							for (int t1 = 0; t1 < t; ++t1)
							{
								cut.addTerm(s[a][j][t1]);
								cut.addTerm(f[a][i][t1], -1);
							}
						}
						else
						{
							XPRBctr ctr;
							int i, j;
							tie(i, j) = IP[x];

							int indices[3] = { a, x, t };
							ctr = genCon(prob, s[a][j][t] <= f[a][i][t], "Pre", 3, indices);

							for (int t1 = 0; t1 < t; ++t1)
							{
								ctr.addTerm(s[a][j][t1]);
								ctr.addTerm(f[a][i][t1], -1);
							}
						}
					}
					else if (m1 % 2 == 1) // s_a_j_t + f_a_t_t1 <= 1    \forall t1
					{
						for (int t1 = t; t1 < NTIMES; ++t1)
						{
							int i, j;
							tie(i, j) = IP[x];

							if (mode % 6 == 1 && !ctrForce)
								prob->newCut(s[a][j][t] + f[a][i][t1] <= 1);
							else
							{
								int indices[4] = { a, x, t, t1 };
								genCon(prob, s[a][j][t] + f[a][i][t1] <= 1, "Pre", 4, indices);
							}
						}
					}
				}
	}

	void genDurationConstraints(XPRBprob* prob, int mode, bool ctrForce = false)
	{
		int m1 = mode / 12;

		// Duration constraints
		for (int a = 0; a < NASSETS; ++a)
			for (int i = 0; i < NTASKS; ++i)
				for (int t1 = 0; t1 < NTIMES; ++t1)
				{
					if (m1 % 2 == 0) // s_a_i_t1 <= f_a_i_(t1+d_i+weatherdelay)
					{
						int worked = 0, reald = 0;
						while (worked < d[i])
						{
							if (OMEGA[i][t1 + reald] == 1)
								worked++;
							reald++;
						}

						int indices[3] = { a, i, t1 };
						if (mode % 6 == 2 && !ctrForce)
							prob->newCut(s[a][i][t1] <= f[a][i][t1 + reald - 1]);
						else
							genCon(prob, s[a][i][t1] <= f[a][i][t1 + reald - 1], "Dur", 3, indices);
					}
					else if (m1 % 2 == 1) // (s_a_i_t1 + f_a_i_t1 - 1) * d_i <= (s_a_i_t1 + f_a_i_t2)/2 * weather /forall t2
					{
						for (int t2 = 0; t2 < NTIMES; ++t2)
						{
							int weather = 0;
							for (int t3 = t1; t3 <= t2; ++t3)
								weather += OMEGA[i][t3];

							int indices[4] = { a, i, t1, t2 };
							if (mode % 6 == 2 && !ctrForce)
								prob->newCut((f[a][i][t2] + s[a][i][t1]) * 0.5 * weather >= d[i] * (s[a][i][t1] + f[a][i][t2] - 1));
							else
								genCon(prob, (f[a][i][t2] + s[a][i][t1]) * 0.5 * weather >= d[i] * (s[a][i][t1] + f[a][i][t2] - 1), "Dur", 4, indices);
						}
					}
				}


	}

	void genResourceConstraints(XPRBprob* prob, int mode, bool ctrForce = false)
	{
		// Resource amount link
		for (int r = 0; r < NRES; ++r)
			for (int p = 0; p < NPERIODS; ++p)
				for (int t = p * TPP; t < (p + 1) * TPP; ++t)
				{
					if (mode % 6 == 3 && !ctrForce)
					{
						XPRBcut cut;
						if (t == 0)
						{
							cut = prob->newCut(n[r][t] == 0);

							for (int i = 0; i < NTASKS; ++i)
							{
								if (rho[r][i] == 0)
									continue;

								for (int a = 0; a < NASSETS; a++)
									cut.addTerm(s[a][i][t], -rho[r][i]);
							}
						}
						else
						{
							cut = prob->newCut(n[r][t] - n[r][t - 1] == 0);

							for (int i = 0; i < NTASKS; ++i)
							{
								if (rho[r][i] == 0)
									continue;

								for (int a = 0; a < NASSETS; a++)
								{
									cut.addTerm(s[a][i][t], -rho[r][i]);
									cut.addTerm(f[a][i][t - 1], rho[r][i]);
								}
							}
						}
					}
					else
					{
						XPRBctr ctr;
						int indices[2] = { r, t };
						if (t == 0)
						{
							ctr = genCon(prob, n[r][t] == 0, "Nee", 2, indices);

							for (int i = 0; i < NTASKS; ++i)
							{
								if (rho[r][i] == 0)
									continue;

								for (int a = 0; a < NASSETS; a++)
									ctr.addTerm(s[a][i][t], -rho[r][i]);
							}
						}
						else
						{
							ctr = genCon(prob, n[r][t] - n[r][t - 1] == 0, "Nee", 2, indices);

							for (int i = 0; i < NTASKS; ++i)
							{
								if (rho[r][i] == 0)
									continue;

								for (int a = 0; a < NASSETS; a++)
								{
									ctr.addTerm(s[a][i][t], -rho[r][i]);
									ctr.addTerm(f[a][i][t - 1], rho[r][i]);
								}
							}
						}
					}

					if (mode % 6 == 3 && !ctrForce)
					{
						prob->newCut(N[r][p] >= n[r][t]);
					}
					else
					{
						int indices2[3] = { r, p, t };
						genCon(prob, N[r][p] >= n[r][t], "Hav", 3, indices2);
					}
				}
	}

	void genOnlineConstraints(XPRBprob* prob, int mode, bool ctrForce = false)
	{
		// Online turbines link
		for (int p = 0; p < NPERIODS; ++p)
		{
			if (mode % 6 == 4 && !ctrForce)
			{
				XPRBcut cut;
				cut = prob->newCut(O[p] == 0);

				for (int t = 0; t < p * TPP; ++t)
					for (int a = 0; a < NASSETS; a++)
						cut.addTerm(f[a][NTASKS - 1][t], -1);
			}
			else
			{
				XPRBctr ctr;
				int indices[1] = { p };
				ctr = genCon(prob, O[p] == 0, "Onl", 1, indices);

				for (int t = 0; t < p * TPP; ++t)
					for (int a = 0; a < NASSETS; a++)
						ctr.addTerm(f[a][NTASKS - 1][t], -1);
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

		genSetConstraints(prob, mode);
		genPrecedenceConstraints(prob, mode);
		genDurationConstraints(prob, mode);
		genResourceConstraints(prob, mode);
		genOnlineConstraints(prob, mode);

		double duration = ((double)clock() - start) / (double)CLOCKS_PER_SEC;
		outputPrinter.printer("Duration of initialisation: " + to_string(duration) + " seconds", 1);
	}

	void genFullProblem(XPRBprob* prob, int mode)
	{
		clock_t start = clock();

		outputPrinter.printer("Initialising Full constraints", 1);

		if (mode % 6 == 0)
			genSetConstraints(prob, mode, true);
		else if (mode % 6 == 1)
			genPrecedenceConstraints(prob, mode, true);
		else if (mode % 6 == 2)
			genDurationConstraints(prob, mode, true);
		else if (mode % 6 == 3)
			genResourceConstraints(prob, mode, true);
		else if (mode % 6 == 4)
			genOnlineConstraints(prob, mode, true);

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

	srand(SEED);

	dataReader.readData();

	for (int mode = 0; mode < NMODES; ++mode)
	{
		cout << "----------------------------------------------------------------------------------------" << endl;
		cout << "MODE: " << mode << endl;

		string name = "Install" + to_string(mode);
		probs[mode].setName(name.c_str());

		if (NAMES == 0)
			probs[mode].setDictionarySize(XPRB_DICT_NAMES, 0);

		clock_t start = clock();

		problemGen.genOriProblem(&probs[mode], mode);
		problemSolver.solveProblem(&probs[mode], name);
		problemGen.genFullProblem(&probs[mode], mode);
		problemSolver.solveProblem(&probs[mode], name);
		outputPrinter.printOutput(&probs[mode]);

		double duration = ((double)clock() - start) / (double)CLOCKS_PER_SEC;
		cout << "FULL duration: " << duration << " seconds" << endl;
		durs[mode] = duration;

		probs[mode].reset();
	}
	
	cout << "----------------------------------------------------------------------------------------" << endl;

	double tots[10];
	for (int i = 0; i < 10; ++i)
		tots[i] = 0.0;

	for (int i = 0; i < 2; ++i)
		for (int j = 0; j < 2; ++j)
			for (int k = 0; k < 6; ++k)
			{
				cout << "MODE: " << k << i << j << " (" << (12 * i + 6 * j + k) << ") DUR: " << durs[12 * i + 6 * j + k] << endl;

				tots[i+6] += durs[12 * i + 6 * j + k];
				tots[j+8] += durs[12 * i + 6 * j + k];
				tots[k] += durs[12 * i + 6 * j + k];
			}

	string names[10] = { "Cuts Sets", "Cuts Prec", "Cuts Dur", "Cuts Res", "Cuts Onl", "No Cuts", "Prec 1", "Prec 2", "Dur 1", "Dur 2" };

	for (int i = 0; i < 6; ++i)
		cout << "SETTING: " << names[i] << " DUR: " << tots[i] / (NMODES / 6) << endl;
	for (int i = 6; i < 10; ++i)
		cout << "SETTING: " << names[i] << " DUR: " << tots[i] / (NMODES / 2) << endl;

	return 0;
}
