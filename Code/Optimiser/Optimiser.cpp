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
#define NCUTMODES 16
#define NMODES NCUTMODES // Product of all mode types
#define NSETTINGS NCUTMODES // Sum of all mode types
#define WEATHERTYPE 1
#define VERBOSITY 1
#define NAMES 1
#define OUTPUTFILE "install"
#define OUTPUTEXT ".sol"

// Model settings
#define DATAFILE "installMonth.dat"
#define NPERIODS 30
#define TPP 12 // Timesteps per Period
#define NTIMES NPERIODS * TPP
#define NTASKS 5
#define NIP 4
#define NRES 3
#define NASSETS 5
#define DIS 0.999972465
#define OPTIMAL -3274199 // The optimal solution, if known

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

class DataReader
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

	int parsePeriodical(char type, vector<string> line, int start, vector<int>* res, int amount = NPERIODS)
	{
		// Switch based on 3 types: U (universal value), I (intervals), S (single values)

		res->clear();
		(*res) = vector<int>(amount);

		switch (type)
		{
		case 'U':
		{
			int val = stoi(line[start]);
			fill(res->begin(), res->begin() + amount, val);
			return start + 1;
		}
		case 'I':
		{
			int filled = 0;
			int loc = start;
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
		case 'S':
		{
			for (int i = 0; i < amount; ++i)
				(*res)[i] = stoi(line[i + start]);
			return start + amount;
		}
		default:
		{
			cout << "Error reading a Periodical" << endl;
			return 0;
		}
		}
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
		string line;
		vector<string>* split = new vector<string>();

		getline(*datafile, line);
		splitString(line, split);
		if ((*split)[0].compare("RESOURCES") != 0)
			cout << "Error reading RESOURCES" << endl;
		if (stoi((*split)[1]) != NRES)
			cout << "Error with declared RESOURCES amount" << endl;

		for (int r = 0; r < NRES; ++r)
		{
			getline(*datafile, line);
			splitString(line, split, '\t');

			int loc = 1;
			vector<int> vals = vector<int>(NPERIODS);
			char type = (*split)[loc][0];
			loc = parsePeriodical(type, *split, loc+1, &vals);
			copy(vals.begin(), vals.end(), C[r]);
			
			type = (*split)[loc][0];
			loc = parsePeriodical(type, *split, loc+1, &vals);
			copy(vals.begin(), vals.end(), m[r]);
		}

		getline(*datafile, line);
	}

	void readValues(ifstream* datafile)
	{
		string line;
		vector<string>* split = new vector<string>();

		getline(*datafile, line);
		if (line.compare("VALUES") != 0)
			cout << "Error reading VALUES" << endl;

		getline(*datafile, line);
		char type = line[0];

		getline(*datafile, line);
		splitString(line, split);
		vector<int> vals = vector<int>(NPERIODS);
		parsePeriodical(type, *split, 0, &vals);
		copy(vals.begin(), vals.end(), v);

		getline(*datafile, line);
	}

	void readPreqs(ifstream* datafile)
	{
		string line;
		vector<string>* split = new vector<string>();

		getline(*datafile, line);
		splitString(line, split);
		if ((*split)[0].compare("PREREQUISITES") != 0)
			cout << "Error reading PREREQUISITES" << endl;
		if (stoi((*split)[1]) != NIP)
			cout << "Error with declared PREREQUISITES amount" << endl;

		for (int i = 0; i < NIP; ++i)
		{
			getline(*datafile, line);
			splitString(line, split);
			IP[i] = make_tuple(stoi((*split)[0]), stoi((*split)[1]));
		}

		getline(*datafile, line);
	}

	void generateWeather()
	{
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

public:
	DataReader()
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

		// Read the resource info
		readResources(&datafile);

		// Read the energy value info
		readValues(&datafile);

		// Read the task order info
		readPreqs(&datafile);

		// Generate the weather and StartAt values
		generateWeather();
		generateStartAtValues();

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
			for (int a = 0; a < NASSETS; ++a)
				for (int i = 0; i < NTASKS; ++i)
					s[a][i][t] = prob->newVar(("s_" + to_string(a) + "_" + to_string(i) + "_" + to_string(t)).c_str(), XPRB_BV);
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
				int indices[1] = { a };
				genCon(prob, rel, "Fin", 1, indices);
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

					if (sa[i][t] == -1)
					{
						s[a][j][t].setUB(0);
						continue;
					}

					XPRBrelation rel = s[a][i][sa[i][t]] >= s[a][j][t];

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

		if (realMode != 0)
		{
			problemGen.genFullProblem(&probs[mode], realMode);
			problemSolver.solveProblem(&probs[mode], name);
		}

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
