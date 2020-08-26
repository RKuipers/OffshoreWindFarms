#include "Deterministic.h"
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
#include "xprs.h"
#include "Mode.cpp"

using namespace std;
using namespace ::dashoptimization;

namespace D
{

// Program settings
#define SEED 42 * NTIMES
#define WEATHERTYPE 1
#define VERBOSITY 1		// The one to edit
#define VERBMODE 1
#define VERBSOL 2
#define VERBINIT 3
#define VERBPROG 4
#define VERBWEAT 5
#define NAMES 1
#define INPUTFOLDER "Input files/"
#define OUTPUTFOLDER "Output files/"
#define PROBOUTPUTEXT ".sol"
#define MODEOUTPUTEXT ".csv"
#define DATAEXT ".dat"

// Mode related settings
//#define LOCKMODE "SetFinFaiDowCuts FinAll TEST0" 
//#define LOCKDIM "SetCuts"		// Current best: SetFinFaiDowCuts, SetOrdResBroCuts
#define LOCKSET 1	// 1 Strong
#define LOCKORD 0	// 0 Strong
#define LOCKFIN	1	// 1 Strong
#define LOCKPRE 0	// 0 Strong
#define LOCKRES 1	// 1 Medium (test more)
#define LOCKACT 0	// 0 Strong
#define LOCKFAI 1	// 1 Strong
#define LOCKCOR	0	// 0 Medium-Strong
#define LOCKDOW	1	// 1 Medium (test more)
#define MODECUTS 9
#define MODEFIN 2
//#define MODETUNE 2
#define MODETEST 2
#define MAXPRETIME 28000
#define MAXFULLTIME 28000

// Model settings
#define PROBNAME "lifeSimple"
#define NPERIODS 7
#define TPP 4 // Timesteps per Period
#define NTIMES NPERIODS * TPP
#define NITASKS 2
#define NMPTASKS 1
#define NMCTASKS 3
#define NDTASKS 2
#define NMTASKS NMPTASKS + NMCTASKS
#define NTASKS NITASKS + NMTASKS + NDTASKS
#define NIP 2
#define NRES 2
#define NASSETS 2
#define DIS 1.0
#define OPTIMAL 280 // The optimal solution, if known

// Weather characteristics
int base = 105;
int variety = 51;
int bonus = -25;

// Model parameters
int OMEGA[NTASKS][NTIMES];
int v[NTIMES];
int C[NRES][NPERIODS];
int d[NTASKS];
int sa[NTASKS][NTIMES + 1];
int rho[NRES][NTASKS];
int m[NRES][NPERIODS];
int lambda[NASSETS][NTASKS];
tuple<int, int> IP[NIP];

// Model variables
XPRBvar N[NRES][NPERIODS];
XPRBvar o[NASSETS][NTIMES];
XPRBvar s[NASSETS][NTASKS][NTIMES];

// Initialiser (to update according to specific tests to be run)
Mode InitMode()
{
	Mode mode = Mode();

#ifdef MODECUTS
	string names[MODECUTS + 2] = { "No", "Set", "Ord", "Fin", "Pre", "Res", "Act", "Fai", "Cor", "Dow", "Cuts" };
	mode.AddCombDim(MODECUTS, names);
#endif // MODECUTS

#ifdef MODEFIN
	string names2[MODEFIN] = { "FinAll", "FinMin" };
	mode.AddDim(MODEFIN, names2);
#endif // MODEFIN

#ifdef MODETUNE
	mode.AddDim(MODETUNE, "Tune");
#endif // MODETUNE

#ifdef MODETEST
	mode.AddDim(MODETEST, "TEST");
#endif // MODETEST

#pragma region Locks
#ifdef LOCKMODE
	mode.LockMode(LOCKMODE);
#endif // LOCKMODE

#ifdef LOCKDIM
	mode.LockDim(LOCKDIM);
#endif // LOCKDIM

#ifdef LOCKSET
	mode.LockSetting("SetCuts", LOCKSET);
#endif // LOCKSET

#ifdef LOCKORD
	mode.LockSetting("OrdCuts", LOCKORD);
#endif // LOCKORD

#ifdef LOCKFIN
	mode.LockSetting("FinCuts", LOCKFIN);
#endif // LOCKFIN

#ifdef LOCKPRE
	mode.LockSetting("PreCuts", LOCKPRE);
#endif // LOCKPRE

#ifdef LOCKRES
	mode.LockSetting("ResCuts", LOCKRES);
#endif // LOCKRES

#ifdef LOCKACT
	mode.LockSetting("ActCuts", LOCKACT);
#endif // LOCKACT

#ifdef LOCKFAI
	mode.LockSetting("FaiCuts", LOCKFAI);
#endif // LOCKFAI

#ifdef LOCKCOR
	mode.LockSetting("CorCuts", LOCKCOR);
#endif // LOCKCOR

#ifdef LOCKDOW
	mode.LockSetting("DowCuts", LOCKDOW);
#endif // LOCKDOW
#pragma endregion 

	mode.Resize();
	mode.Reset();

	return mode;
}

class OutputPrinter
{
private:
	void printObj(ofstream* file, XPRBprob* prob)
	{
		printer("Total return: " + to_string(round(prob->getObjVal())), VERBSOL);
		*file << "Objective: " << prob->getObjVal() << endl;
	}

	void printTurbines(ofstream* file)
	{
		vector<int> vals = vector<int>();

		printer("Online turbines per timestep: ", VERBSOL);
		for (int t = 0; t < NTIMES; ++t)
		{
			vals.push_back(0);
			for (int a = 0; a < NASSETS; ++a)
				vals[t] += round(o[a][t].getSol());

			int v = vals[t];
			if (t == 0 || v != vals[t-1])
				printer(to_string(t) + ": "+ to_string(v), VERBSOL);
			*file << "O_" << t << ": " << v << endl;
		}
	}

	void printResources(ofstream* file)
	{
		printer("Resources needed per period and type: ", VERBSOL);
		for (int p = 0; p < NPERIODS; ++p)
		{
			int v = round(N[0][p].getSol());
			printer(to_string(p) + ": " + to_string(v), VERBSOL, false);
			*file << "N_0_" << p << ": " << v << endl;;

			for (int r = 1; r < NRES; ++r)
			{
				v = round(N[r][p].getSol());
				printer(", " + to_string(v), VERBSOL, false);
				*file << "N_" << r << "_" << p << ": " << v << endl;;
			}
			printer("", VERBSOL);
		}
	}

	void printTasks(ofstream* file)
	{
		printer("Start and finish time per asset and task: ", VERBSOL);
		for (int a = 0; a < NASSETS; ++a)
		{
			printer("Asset: " + to_string(a), VERBSOL);
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

				if (start == -1)
				{
					printer(to_string(i) + ": Incomplete", VERBSOL);
					*file << "Asset " << a << " task " << i << ": Incomplete" << endl;
				}
				else
				{
					for (int t1 = start + d[i] - 1; t1 <= NTIMES; ++t1)
						if (sa[i][t1] >= start)
						{
							finish = t1 - 1;
							break;
						}

					printer(to_string(i) + ": " + to_string(start) + " " + to_string(finish), VERBSOL);
					*file << "Asset " << a << " task " << i << ": " << start << " " << finish << endl;
				}
			}
		}
	}

	string boolVec2Str(vector<bool> vec)
	{
		string res = "";
		for (int i = 0; i < vec.size(); ++i)
		{
			if (vec[i])
				res.append("1");
			else
				res.append("0");
			if (i < vec.size() - 1)
				res.append(";");
		}
		return res;
	}

public:
	void printProbOutput(XPRBprob* prob, Mode* m, int id)
	{
		if (prob->getProbStat() == 1)
			return;

		ofstream file;
		file.open(string() + OUTPUTFOLDER + PROBNAME + to_string(id) + PROBOUTPUTEXT);

		printObj(&file, prob);
		printTurbines(&file);
		printResources(&file);
		printTasks(&file);

		file.close();
	}

	void printModeOutput(Mode* m, bool opt)
	{
		ofstream file;
		file.open(string() + OUTPUTFOLDER + PROBNAME + "Modes" + MODEOUTPUTEXT);

		printer("----------------------------------------------------------------------------------------", VERBMODE);

#ifdef OPTIMAL
		if (opt)
			printer("All solutions are optimal", VERBMODE);
		else
			printer("Not all solutions are optimal", VERBMODE);
#endif // OPTIMAL
		
		vector<string> modeNames = m->GetModeNames();
		m->Reset();

		for (int i = 0; i < m->GetNModes(); ++i)
		{
			double dur = m->GetDur(i);
			string setStr = boolVec2Str(m->GetSettingStatus());
			printer("MODE: " + to_string(i) + " (" + modeNames[i] + ") DUR: " + to_string(dur), VERBMODE);
			file << i << ";" << modeNames[i] << ";" << dur << ";" << setStr << endl;
			m->Next();
		}

		file.close();

#if NMODETYPES > 1
		file.open(string() + OUTPUTFOLDER + PROBNAME + "Settings" + MODEOUTPUTEXT);

		vector<string> settingNames = m->GetSettingNames();
		vector<double> setAvgs = m->GetSettingDurs();

		for (int i = 0; i < m->GetNSettings(); ++i)
		{
			printer("SETTING: " + settingNames[i] + " DUR: " + to_string(setAvgs[i]), VERBMODE);
			file << settingNames[i] << ";" << setAvgs[i] << endl;
		}

		file.close();
		file.open(string() + OUTPUTFOLDER + PROBNAME + "Submodes" + MODEOUTPUTEXT);

		vector<string> subModeNames = m->GetCombModeNames();
		vector<double> subModeAvgs = m->GetModeDurs(subModeNames);

		for (int i = 0; i < subModeNames.size(); ++i)
		{
			if (!isnan(subModeAvgs[i]))
				printer("SUBMODE: " + subModeNames[i] + " DUR: " + to_string(subModeAvgs[i]), VERBMODE);
			file << subModeNames[i] << ";" << subModeAvgs[i] << endl;
		}

		file.close();
#endif // NMODETYPES > 1
	}

	int printer(string s, int verbosity, bool end = true, int maxVerb = 999)
	{
		if (VERBOSITY >= verbosity && VERBOSITY < maxVerb)
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
		case 'U': // U x -> x used for all periods
		{
			int val = stoi(line[start]);
			fill(res->begin(), res->begin() + amount, val);
			return start + 1;
		}
		case 'I': // I s1 e1 v1 s2 e2 v2 ... sn en vn -> Periods sx (inclusive) through ex (exclusive) use vx, s1 through en should cover all periods
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
		case 'S': // p1 v1 p2 v2 ... pn vn -> Period px uses vx, every period should be mentioned
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

	void readTasks(ifstream* datafile, int taskType)
	{
		string line;
		vector<string>* split = new vector<string>();

		string name;
		int ntasks, start;
		switch (taskType)
		{
		case 0:
			name = "ITASKS";
			ntasks = NITASKS;
			start = 0;
			break;
		case 1:
			name = "MPTASKS";
			ntasks = NMPTASKS;
			start = NITASKS;
			break;
		case 2:
			name = "MCTASKS";
			ntasks = NMCTASKS;
			start = NITASKS + NMPTASKS;
			break; 
		case 3:
			name = "DTASKS";
			ntasks = NDTASKS;
			start = NITASKS + NMTASKS;
			break;
		default:
			name = "";
			ntasks = -1;
			start = -1;
			break;
		}

		getline(*datafile, line);
		splitString(line, split);
		if ((*split)[0].compare(name) != 0)
			cout << "Error reading " << name << endl;
		if (stoi((*split)[1]) != ntasks)
			cout << "Error with declared " << name << " amount" << endl;

		int copies = 1;

		for (int i = start; i < ntasks + start; i += copies)
		{
			getline(*datafile, line);
			splitString(line, split, '\t');

			if (count(line.begin(), line.end(), '\t') != 2 + NRES)
				cout << "Error with column count on " << name << " line " << i << endl;

			if ((*split)[0].find(" ") != string::npos)
			{
				vector<string>* dups = new vector<string>();
				splitString((*split)[0], dups, ' ');
				copies = stoi((*dups)[1]) - stoi((*dups)[0]);
			}
			else
				copies = 1;

			for (int x = 0; x < copies; ++x)
			{
				d[i + x] = stoi((*split)[1]);
				limits[i + x] = stoi((*split)[2]);

				for (int r = 0; r < NRES; ++r)
					rho[r][i + x] = stoi((*split)[r + 3]);
			}
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

	void readSimpleArray(ifstream* datafile, string name, int arrSize, int arr[])
	{
		string line;
		vector<string>* split = new vector<string>();

		getline(*datafile, line);
		if (line.compare(name) != 0)
			cout << "Error reading " << name << endl;

		getline(*datafile, line);
		char type = line[0];

		getline(*datafile, line);
		splitString(line, split);
		vector<int> vals = vector<int>(arrSize);
		parsePeriodical(type, *split, 0, &vals, arrSize);
		copy(vals.begin(), vals.end(), arr);

		getline(*datafile, line);
	}

	void readLambdas(ifstream* datafile)
	{
		string line;
		vector<string>* split = new vector<string>();

		getline(*datafile, line);
		splitString(line, split);
		if ((*split)[0].compare("LAMBDAS") != 0)
			cout << "Error reading LAMBDAS" << endl;
		if (stoi((*split)[1]) != NMTASKS + 1)
			cout << "Error with declared LAMBDAS amount" << endl;

		int copies = 1;

		for (int i = 0; i < NTASKS; i += copies)
		{
			if (i < NITASKS - 1 || i >= NITASKS + NMTASKS)
			{
				for (int a = 0; a < NASSETS; ++a)
					lambda[a][i] = 0;
				copies = 1;
				continue;
			}

			getline(*datafile, line);
			splitString(line, split, '\t');

			if ((*split)[0].find(" ") != string::npos)
			{
				vector<string>* dups = new vector<string>();
				splitString((*split)[0], dups, ' ');
				copies = stoi((*dups)[1]) - stoi((*dups)[0]);
			}
			else
				copies = 1;

			vector<int> vals = vector<int>(NASSETS);
			parsePeriodical((*split)[1][0], *split, 2, &vals, NASSETS);

			for (int a = 0; a < NASSETS; ++a)
				for (int j = i; j < i + copies; ++j)
					lambda[a][j] = vals[a];
		}

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
		outputPrinter.printer("Wave heights per timestep:", VERBWEAT);

		int waveHeight[NTIMES];
		if (WEATHERTYPE == 0)
		{
			waveHeight[0] = base;

			outputPrinter.printer("0: " + to_string(waveHeight[0]), VERBWEAT);
			for (int t = 1; t < NTIMES; ++t)
			{
				bonus += (base - waveHeight[t - 1]) / 40;

				waveHeight[t] = max(0, waveHeight[t - 1] + bonus + (rand() % variety));
				outputPrinter.printer(to_string(t) + ": " + to_string(waveHeight[t]), VERBWEAT);
			}
		}
		else if (WEATHERTYPE == 1)
		{
			for (int p = 0; p < NPERIODS; ++p)
			{
				waveHeight[p * TPP] = base;
				outputPrinter.printer(to_string(p * TPP) + ": " + to_string(waveHeight[p * TPP]), VERBWEAT);
				for (int t = (p * TPP) + 1; t < (p + 1) * TPP; ++t)
				{
					waveHeight[t] = max(0, waveHeight[t - 1] + bonus + (rand() % variety));
					outputPrinter.printer(to_string(t) + ": " + to_string(waveHeight[t]), VERBWEAT);
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
		outputPrinter.printer("Reading Data", VERBMODE);

		string line;
		ifstream datafile(string() + INPUTFOLDER + PROBNAME + DATAEXT);
		if (!datafile.is_open())
		{
			cout << "Unable to open file" << endl;
			return;
		}

		// Read the task info
		readTasks(&datafile, 0);
		readTasks(&datafile, 1);
		readTasks(&datafile, 2);
		readTasks(&datafile, 3);

		// Read the resource info
		readResources(&datafile);

		// Read the energy value info
		readSimpleArray(&datafile, "VALUES", NTIMES, v);

		// Read the lambda info
		readLambdas(&datafile);

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
	void genCon(XPRBprob* prob, const XPRBrelation& ac, string base, int nInd, int* indices, bool cut)
	{
		if (cut)
			prob->newCut(ac);
		else if (NAMES == 0)
			prob->newCtr(ac);
		else
		{
			string name = base;

			for (int i = 0; i < nInd; ++i)
				name += "_" + to_string(indices[i]);

			prob->newCtr(name.c_str(), ac);
		}
	}

	void genDecisionVariables(XPRBprob* prob)
	{
		// Create the period-based decision variables
		for (int p = 0; p < NPERIODS; ++p)
			for (int r = 0; r < NRES; ++r)
			{
				N[r][p] = prob->newVar(("N_" + to_string(r) + "_" + to_string(p)).c_str(), XPRB_UI);
				N[r][p].setLB(0);
				N[r][p].setUB(m[r][p]);
			}

		// Create the timestep-based decision variables
		for (int t = 0; t < NTIMES; ++t)
			for (int a = 0; a < NASSETS; ++a)
			{
				o[a][t] = prob->newVar(("o_" + to_string(a) + "_" + to_string(t)).c_str(), XPRB_BV); 

				for (int i = 0; i < NTASKS; ++i)
					s[a][i][t] = prob->newVar(("s_" + to_string(a) + "_" + to_string(i) + "_" + to_string(t)).c_str(), XPRB_BV);
			}
	}

	void genObjective(XPRBprob* prob)
	{
		XPRBctr Obj = prob->newCtr();
		for (int p = 0; p < NPERIODS; ++p)
		{
			double dis = pow(DIS, p);

			for (int t = p * TPP; t < (p + 1) * TPP; ++t)
				for (int a = 0; a < NASSETS; ++a)
					Obj.addTerm(o[a][t], v[t] * dis);

			for (int r = 0; r < NRES; ++r)
				Obj.addTerm(N[r][p], -C[r][p] * dis);
		}
		prob->setObj(Obj); // Set the objective function
	}

	void genSetConstraints(XPRBprob* prob, bool cut)
	{
		// Once a task has started it stays started
		for (int a = 0; a < NASSETS; ++a)
			for (int i = 0; i < NTASKS; ++i)
				for (int t = 1; t < NTIMES; ++t)
				{
					XPRBrelation rel = s[a][i][t] >= s[a][i][t - 1];

					int indices[3] = { a, i, t };
					genCon(prob, rel, "Set", 3, indices, cut);
				}
	}

	void genOrderConstraints(XPRBprob* prob, bool cut)
	{
		// Forces every non-decomission task inactive after decomission starts
		for (int a = 0; a < NASSETS; ++a)
			for (int i = 0; i < NITASKS + NMTASKS; ++i)
				for (int t = 0; t < NTIMES; ++t)
				{
					if (sa[i][t] < 0)
						continue;

					XPRBrelation rel = s[a][i][sa[i][t]] - s[a][i][t] >= s[a][NITASKS + NMTASKS][t] - 1;

					int indices[3] = { a, i, t };
					genCon(prob, rel, "Ord", 3, indices, cut);
				}
	}

	void genFinishConstraints(XPRBprob* prob, bool cut, bool finAll)
	{
		// Forces every non-optional task to finish
		for (int a = 0; a < NASSETS; ++a)
			for (int i = 0; i < NTASKS; ++i)
			{
				if ((i >= NITASKS + NMPTASKS && i < NITASKS + NMTASKS) || (!finAll && (i < NITASKS - 1 || (i >= NITASKS + NMTASKS && i != NTASKS - 1))))
					continue;
				
				XPRBrelation rel = s[a][i][sa[i][NTIMES]] == 1;

				int indices[2] = { a, i };
				genCon(prob, rel, "Fin", 2, indices, cut);
			}
	}

	void genPrecedenceConstraints(XPRBprob* prob, bool cut)
	{
		// Precedence constraints
		for (int x = 0; x < NIP + NMTASKS + 1; ++x)
		{
			int i, j;
			if (x < NIP) // Normal precedence
				tie(i, j) = IP[x];
			else if (x < NIP + NMPTASKS) // Perform preventive maintenance tasks in order
			{
				i = x - NIP + NITASKS - 1;
				j = x - NIP + NITASKS; 
			}
			else if (x < NIP + NMTASKS) // Finish installation before corrective maintenance can take place
			{
				i = NITASKS - 1;
				j = x - NIP + NITASKS;
			}
			else
			{
				if (NITASKS <= 0 || NDTASKS <= 0)
					continue;

				i = NITASKS - 1;
				j = NITASKS + NMTASKS;
			}

			for (int a = 0; a < NASSETS; ++a)
				for (int t = 0; t < NTIMES; ++t)
				{
					if (sa[i][t] == -1)
					{
						s[a][j][t].setUB(0);
						continue;
					}

					XPRBrelation rel = s[a][i][sa[i][t]] >= s[a][j][t];

					int indices[3] = { a, x, t };
					genCon(prob, rel, "Pre", 3, indices, cut);
				}
		}
	}

	void genResourceConstraints(XPRBprob* prob, bool cut)
	{
		// Resource amount link to starting times
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

					int indices[3] = { r, p, t };
					genCon(prob, rel, "Nee", 3, indices, cut);
				}
	}

	void genActiveConstraints(XPRBprob* prob, bool cut)
	{
		// Ensures turbines are only active after installation finishes and before decomission begins
		for (int t = 0; t < NTIMES; ++t)
			for (int a = 0; a < NASSETS; ++a)
			{
				if (sa[NITASKS - 1][t] < 0)
				{
					o[a][t].setUB(0);
					continue;
				}

				XPRBrelation rel = o[a][t] <= s[a][NITASKS - 1][sa[NITASKS - 1][t]] - s[a][NITASKS + NMTASKS][t];

				int indices[2] = { a, t };
				genCon(prob, rel, "Act", 2, indices, cut);
			}
	}

	void genFailureConstraints(XPRBprob* prob, bool cut)
	{
		// Turbines can only be online if they're not broken
		for (int t = 0; t < NTIMES; ++t)
			for (int a = 0; a < NASSETS; ++a)
			{
				XPRBrelation rel = o[a][t] <= 0;

				for (int i = NITASKS - 1; i < NITASKS + NMTASKS; i++)
				{
					if (sa[i][t] > -1)
						rel.addTerm(s[a][i][sa[i][t]], -1);
					if (t - lambda[a][i] >= 0 && sa[i][t - lambda[a][i]] > -1)
						rel.addTerm(s[a][i][sa[i][t - lambda[a][i]]]);
				}

				int indices[2] = { a, t };
				genCon(prob, rel, "Fai", 2, indices, cut);
			}
	}

	void genCorrectiveConstraints(XPRBprob* prob, bool cut)
	{
		double factor = 1 / ((double)NTASKS);

		// Turbines can only be correctively repaired if they are broken
		for (int t = 0; t < NTIMES; ++t)
			for (int a = 0; a < NASSETS; ++a)
				for (int i = NITASKS + NMPTASKS; i < NITASKS + NMTASKS; ++i)
				{
					if (t == 0)
					{
						s[a][i][t].setUB(0);
						continue;
					}

					XPRBrelation rel = s[a][i][t] - s[a][i][t-1] <= 1;

					for (int j = NITASKS - 1; j < NITASKS + NMTASKS; j++)
					{
						if (sa[j][t] > -1)
							rel.addTerm(s[a][j][sa[j][t]], factor);
						if (t - lambda[a][j] >= 0 && sa[j][t - lambda[a][j]] > -1)
							rel.addTerm(s[a][j][sa[j][t - lambda[a][j]]], -factor);
					}

					int indices[3] = { a, i, t };
					genCon(prob, rel, "Cor", 3, indices, cut);
				}
	}

	void genDowntimeConstraints(XPRBprob* prob, bool cut)
	{
		// Turbines are offline while maintenance work is ongoing
		for (int t = 0; t < NTIMES; ++t)
			for (int a = 0; a < NASSETS; ++a)
				for (int i = NITASKS; i < NITASKS + NMTASKS; ++i)
				{
					XPRBrelation rel = o[a][t] <= 1 - s[a][i][t];
					if (sa[i][t] >= 0)
						rel.addTerm(s[a][i][sa[i][t]], -1);

					int indices[3] = { a, i, t };
					genCon(prob, rel, "Down", 3, indices, cut);
				}
	}

public:
	void genBasicProblem(XPRBprob* prob, Mode* m)
	{
		outputPrinter.printer("Initialising variables and objective", VERBINIT);

		genDecisionVariables(prob);
		genObjective(prob);
	}

	void genOriProblem(XPRBprob* prob, Mode* m)
	{
		outputPrinter.printer("Initialising Original constraints", VERBINIT);

		genSetConstraints(prob, m->GetCurrentBySettingName("SetCuts") == 1);
		genOrderConstraints(prob, m->GetCurrentBySettingName("OrdCuts") == 1);
		genFinishConstraints(prob, m->GetCurrentBySettingName("FinCuts") == 1, m->GetCurrentBySettingName("FinAll") == 1);
		genPrecedenceConstraints(prob, m->GetCurrentBySettingName("PreCuts") == 1);
		genResourceConstraints(prob, m->GetCurrentBySettingName("ResCuts") == 1);
		genActiveConstraints(prob, m->GetCurrentBySettingName("ActCuts") == 1);
		genFailureConstraints(prob, m->GetCurrentBySettingName("FaiCuts") == 1);
		genCorrectiveConstraints(prob, m->GetCurrentBySettingName("CorCuts") == 1);
		genDowntimeConstraints(prob, m->GetCurrentBySettingName("DowCuts") == 1);
	}

	void genFullProblem(XPRBprob* prob, Mode* m)
	{
		clock_t start = clock();

		outputPrinter.printer("Initialising Full constraints", VERBINIT);

		if (m->GetCurrentBySettingName("SetCuts") == 1)
			genSetConstraints(prob, false); 
		if (m->GetCurrentBySettingName("OrdCuts") == 1)
			genOrderConstraints(prob, false);
		if (m->GetCurrentBySettingName("FinCuts") == 1)
			genFinishConstraints(prob, false, m->GetCurrentBySettingName("FinAll") == 1);
		if (m->GetCurrentBySettingName("PreCuts") == 1)
			genPrecedenceConstraints(prob, false);
		if (m->GetCurrentBySettingName("ResCuts") == 1)
			genResourceConstraints(prob, false);
		if (m->GetCurrentBySettingName("ActCuts") == 1)
			genActiveConstraints(prob, false);
		if (m->GetCurrentBySettingName("FaiCuts") == 1)
			genFailureConstraints(prob, false);
		if (m->GetCurrentBySettingName("CorCuts") == 1)
			genCorrectiveConstraints(prob, false);
		if (m->GetCurrentBySettingName("DowCuts") == 1)
			genDowntimeConstraints(prob, false);

		double duration = ((double)clock() - start) / (double)CLOCKS_PER_SEC;
		outputPrinter.printer("Duration of initialisation: " + to_string(duration) + " seconds", VERBINIT);
	}
};

class ProblemSolver
{
public:
	void solveProblem(XPRBprob* prob, bool tune, string name, int maxTime = 0)
	{
		outputPrinter.printer("Solving problem", VERBINIT);
		if (VERBOSITY < VERBPROG)
			prob->setMsgLevel(1);

		clock_t start = clock();

		if (maxTime != 0 || tune)
		{
			XPRBloadmat(prob->getCRef());
			XPRSprob opt_prob = XPRBgetXPRSprob(prob->getCRef());
			if (maxTime != 0)
				XPRSsetintcontrol(opt_prob, XPRS_MAXTIME, maxTime);
			if (tune)
				XPRStune(opt_prob, "g");
		}

		prob->setSense(XPRB_MAXIM);
		prob->exportProb(XPRB_LP, (OUTPUTFOLDER + name).c_str());

		prob->mipOptimize("");

		double duration = ((double)clock() - start) / (double)CLOCKS_PER_SEC;
		outputPrinter.printer("Solving duration: " + to_string(duration) + " seconds", VERBSOL);

		const char* MIPSTATUS[] = { "not loaded", "not optimized", "LP optimized", "unfinished (no solution)", "unfinished (solution found)", "infeasible", "optimal", "unbounded" };
		outputPrinter.printer(string() + "Problem status: " + MIPSTATUS[prob->getMIPStat()], VERBSOL);
	}
};

// Program Objects
DataReader dataReader;
ProblemGen problemGen;
ProblemSolver problemSolver;

void run()
{
	dataReader = DataReader();
	problemGen = ProblemGen();
	problemSolver = ProblemSolver();
	outputPrinter = OutputPrinter();

	bool opt = true;

	srand(SEED);

	dataReader.readData();

	Mode mode = InitMode();

	for (bool stop = false; !stop; stop = mode.Next())
	{
		int id = mode.GetID();

		string modeName = mode.GetCurrentModeName();
		XPRBprob* p = new XPRBprob();

		outputPrinter.printer("----------------------------------------------------------------------------------------", VERBSOL);
		outputPrinter.printer("MODE: " + to_string(id) + " (" + modeName + ")", VERBSOL);

		string name = "Life" + to_string(id);
		p->setName(name.c_str());

		if (NAMES == 0)
			p->setDictionarySize(XPRB_DICT_NAMES, 0);

		clock_t start = clock();

		problemGen.genBasicProblem(p, &mode);
		problemGen.genOriProblem(p, &mode);

		double duration = ((double)clock() - start) / (double)CLOCKS_PER_SEC;
		outputPrinter.printer("Duration of initialisation: " + to_string(duration) + " seconds", VERBINIT);

		bool toTune = mode.GetCurrentBySettingName("Tune1") == 1;

		if (mode.GetCurrentModeName(0).compare("NoCuts") != 0)
		{
			problemSolver.solveProblem(p, toTune, name, MAXPRETIME);
			toTune = false;
			problemGen.genFullProblem(p, &mode);
		}

		problemSolver.solveProblem(p, toTune, name, MAXFULLTIME);

		outputPrinter.printProbOutput(p, &mode, id);

#ifdef OPTIMAL
		opt &= round(p->getObjVal()) == OPTIMAL;
#endif // OPTIMAL

		duration = ((double)clock() - start) / (double)CLOCKS_PER_SEC;
		outputPrinter.printer("FULL duration: " + to_string(duration) + " seconds", VERBSOL);

		outputPrinter.printer("Mode: " + to_string(id) + ", duration: " + to_string(duration) + " seconds, Solution: " + to_string(round(p->getObjVal())), VERBMODE, true, VERBSOL);
		mode.SetCurrDur(duration);
	}

#ifndef LOCKMODE
	outputPrinter.printModeOutput(&mode, opt);
#endif // !LOCKMODE
}

}
