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
#define NCUTMODES 1
#define NDECVAR 2
#define NMODES NCUTMODES * NDECVAR // Product of all mode types
#define NSETTINGS NCUTMODES + NDECVAR // Sum of all mode types
#define WEATHERTYPE 1
#define CUTMODE 0
#define VERBOSITY 1
#define NAMES 1
#define OUTPUTFILE "install.sol"

// Model settings
#define DATAFILE "installSimple.dat"
#define NPERIODS 3
#define TPP 4 // Timesteps per Period
#define NTIMES NPERIODS * TPP
#define NTASKS 4
#define NIP 3
#define NRES 2
#define NASSETS 2
#define DIS 1.0
#define OPTIMAL -270 // The optimal solution, if known

// Weather characteristics
int base = 105;
int variety = 51;
int bonus = -25;

// Model parameters
int OMEGA[NTASKS][NTIMES];
int v[NPERIODS];
int C[NRES][NPERIODS];
int d[NTASKS];
int rd[NTASKS][NTIMES];
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
	void printProbOutput(XPRBprob* prob)
	{
		ofstream file;
		file.open(OUTPUTFILE);

		printObj(&file, prob);
		printTurbines(&file);
		printResources(&file);
		printTasks(&file);

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

		for (int i = 0; i < NCUTMODES; ++i)
		{
			cout << "MODE: " << i << " DUR: " << durs[i] << endl;

			tots[i] += durs[i];
		}

		/*for (int i = 0; i < NCUTMODES; ++i)
			cout << "SETTING: " << names[i] << " DUR: " << tots[i] / (NMODES / NCUTMODES) << endl;*/
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

	void generateRealDurations()
	{
		for(int i = 0; i < NTASKS; ++i)
			for (int t1 = 0; t1 < NTIMES; ++t1)
			{
				int worked = 0;
				int t2;
				for (t2 = t1; worked < d[i] && t2 > 0; --t2)
					if (OMEGA[i][t2] == 1)
						worked++;

				if (worked == d[i])
					rd[i][t1] = t1 - t2; // TODO: This leads to positive numbers when OMEGA == 0; could turn those to -1 instead as well
				else
					rd[i][t1] = -1; // TODO: Check if this value works
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
		generateRealDurations();

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

	void genSetConstraints(XPRBprob* prob, bool cut)
	{
		// Forces every task to start and end
		for (int a = 0; a < NASSETS; ++a)
			for (int i = 0; i < NTASKS; ++i)
			{
				XPRBrelation rs = s[a][i][0] == 1;
				XPRBrelation rf = f[a][i][0] == 1;

				for (int t = 1; t < NTIMES; ++t)
				{
					rs.addTerm(s[a][i][t]);
					rf.addTerm(f[a][i][t]);
				}

				if (cut)
				{
					prob->newCut(rs);
					prob->newCut(rf);
				}
				else
				{
					int indices[2] = { a, i };
					genCon(prob, rs, "Sta", 2, indices);
					genCon(prob, rf, "Fin", 2, indices);
				}
			}
	}

	void genPrecedenceConstraints(XPRBprob* prob, bool cut)
	{
		// Precedence constraints
		for (int a = 0; a < NASSETS; ++a)
			for (int x = 0; x < NIP; ++x)
				for (int t1 = 0; t1 < NTIMES; ++t1)
					for (int t2 = t1; t2 < NTIMES; ++t2)
					{
						int i, j;
						tie(i, j) = IP[x];

						const XPRBrelation rel = s[a][j][t1] + f[a][i][t2] <= 1;

						if (cut)
							prob->newCut(rel);
						else
						{
							int indices[4] = { a, x, t1, t2 };
							genCon(prob, rel, "Pre", 4, indices);
						}
					}
	}

	void genDurationConstraints(XPRBprob* prob, bool cut)
	{
		// Duration constraints
		for (int a = 0; a < NASSETS; ++a)
			for (int i = 0; i < NTASKS; ++i)
				for (int t1 = 0; t1 < NTIMES; ++t1)
				{
					for (int t2 = 0; t2 < NTIMES; ++t2)
					{
						int weather = 0;
						for (int t3 = t1; t3 <= t2; ++t3)
							weather += OMEGA[i][t3];

						XPRBrelation rel = (f[a][i][t2] + s[a][i][t1]) * 0.5 * weather >= d[i] * (s[a][i][t1] + f[a][i][t2] - 1);

						int indices[4] = { a, i, t1, t2 };
						if (cut)
							prob->newCut(rel);
						else
							genCon(prob, rel, "Dur", 4, indices);
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
					XPRBrelation rel = n[r][t] == 0;

					for (int i = 0; i < NTASKS; ++i)
					{
						if (rho[r][i] == 0)
							continue;

						for (int a = 0; a < NASSETS; a++)
							rel.addTerm(s[a][i][t], -rho[r][i]);

						if (t > 0)
						{
							rel.addTerm(n[r][t - 1], -1);
							for (int a = 0; a < NASSETS; a++)
								rel.addTerm(f[a][i][t - 1], rho[r][i]);
						}
					}

					if (cut)
					{
						prob->newCut(rel);
						prob->newCut(N[r][p] >= n[r][t]);
					}
					else
					{
						int indices[2] = { r, t }; 
						genCon(prob, rel, "Nee", 2, indices);
						int indices2[3] = { r, p, t };
						genCon(prob, N[r][p] >= n[r][t], "Hav", 3, indices2);
					}
				}
	}

	void genOnlineConstraints(XPRBprob* prob, bool cut)
	{
		// Online turbines link
		for (int p = 0; p < NPERIODS; ++p)
		{
			XPRBrelation rel = O[p] == 0;

			for (int t = 0; t < p * TPP; ++t)
				for (int a = 0; a < NASSETS; a++)
					rel.addTerm(f[a][NTASKS - 1][t], -1);

			if (cut)
				prob->newCut(rel);
			else
			{
				int indices[1] = { p };
				genCon(prob, rel, "Onl", 1, indices);
			}
		}
	}

	void genCtrByID(XPRBprob* prob, int id)
	{
		switch (id)
		{
		case 0:
			genSetConstraints(prob, false);
			break;
		case 1:
			genPrecedenceConstraints(prob, false);
			break;
		case 2:
			genDurationConstraints(prob, false);
			break;
		case 3:
			genResourceConstraints(prob, false);
			break;
		case 4:
			genOnlineConstraints(prob, false);
			break;
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
		genDurationConstraints(prob, (mode >> 2) % 2 == 1);
		genResourceConstraints(prob, (mode >> 3) % 2 == 1);
		genOnlineConstraints(prob, (mode >> 4) % 2 == 1);

		double duration = ((double)clock() - start) / (double)CLOCKS_PER_SEC;
		outputPrinter.printer("Duration of initialisation: " + to_string(duration) + " seconds", 1);
	}

	bool addCtr(XPRBprob* prob, int mode, int id)
	{
		if ((mode >> id) % 2 == 1)
		{
			genCtrByID(prob, id);
			return true;
		}
		return false;
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
			genDurationConstraints(prob, false);
		if ((mode >> 3) % 2 == 1)
			genResourceConstraints(prob, false);
		if ((mode >> 4) % 2 == 1)
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
		int realMode = 16;
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
		problemGen.genFullProblem(&probs[mode], realMode);
		problemSolver.solveProblem(&probs[mode], name);
#endif // CUTMODE == 0
#if CUTMODE == 1
		for (int i = 0; i < 5; ++i)
		{
			if (problemGen.addCtr(&probs[mode], realMode, i))
				problemSolver.solveProblem(&probs[mode], name);
		}
#endif // CUTMODE == 1
		outputPrinter.printProbOutput(&probs[mode]);

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
