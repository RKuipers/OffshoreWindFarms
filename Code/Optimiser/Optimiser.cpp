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
#define NMODETYPES 2
#define MODECUTS 4
#define MODETESTS 3
#define NMODES 16 * MODETESTS // 2^MODECUTS * MODETESTS    // Product of all mode types (2^x for combination modes)
#define WEATHERTYPE 1
#define VERBOSITY 0
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
XPRBvar s[NASSETS][NTASKS][NTIMES];

class Mode 
{
	class ModeDim
	{
	protected:
		int curr, max, type, settings;
		vector<string> modeNames, settingNames;

		vector<string> expandFromName(string name, int sets)
		{
			vector<string> res;

			for (int i = 0; i < max; ++i)
				res.push_back(name + to_string(i));

			return res;
		}

		vector<string> genModeNames(string names[], int sets)
		{
			vector<string> res;

			for (int i = 0; i < max; ++i)
			{
				string name = "";
				for (int j = 0; j < sets; ++j)
					if ((i >> j) % 2 == 1)
						name += names[j + 1];

				if (name == "")
					name = names[0];

				res.push_back(name + names[sets + 1]);
			}

			return res;
		}

		vector<string> genSettingNames(string names[], int sets)
		{
			vector<string> res;

			for (int i = 0; i < sets; ++i)
			{
				res.push_back("N" + names[i+1] + names[sets + 1]);
				res.push_back("Y" + names[i+1] + names[sets + 1]);
			}

			return res;
		}

	public:
		ModeDim(string name, int type = 0, int max = 2)
		{
			this->curr = 0;
			if (type == 0)
			{
				this->settings = max;
				this->max = max;
			}
			else if (type == 1)
			{
				this->settings = 2 * max;
				this->max = pow(2, max);
			}
			this->type = type;
			this->modeNames = expandFromName(name, max);
			this->settingNames = modeNames;
		}

		ModeDim(string names[], int type = 0, int max = 2)
		{
			this->curr = 0;
			this->type = type;
			if (type == 0)
			{
				this->settings = max;
				this->max = max;
				this->modeNames = vector<string>(names, names + max);
				this->settingNames = vector<string>(names, names + max);
			}
			else if (type == 1)
			{
				this->settings = 2 * max;
				this->max = pow(2, max);
				this->modeNames = genModeNames(names, max);
				this->settingNames = genSettingNames(names, max);
			}
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

		int getCurr(int dim = -1)
		{
			if (type == 0)
				return curr;
			else if (type == 1 && dim >= 0)
				return (curr >> dim) % 2;
		}

		int getCurr(string name)
		{
			if (type == 1)
				name = "Y" + name;

			for (int i = 0; i < settings; ++i)
				if (name.compare(settingNames[i]) == 0)
					return getCurrBySet(i);

			return -1;
		}

		bool getCurrBySet(int set)
		{
			if (type == 0)
				return curr == set;
			else if (type == 1)
				return getCurr(set / 2) == set % 2;
		}

		vector<bool> getSetStat()
		{
			vector<bool> res;

			for (int i = 0; i < settings; ++i)
				res.push_back(getCurrBySet(i));

			return res;
		}

		void setCurr(int val)
		{
			curr = val;
		}

		string getModeName(int mode = -1)
		{
			int id = mode;
			if (id == -1)
				id = curr;

			return modeNames[id];
		}

		string getSetName(int set = -1)
		{
			int id = set;
			if (id == -1)
				id = curr;

			return settingNames[id];
		}

		int getMax()
		{
			return this->max;
		}
		
		int getSettings()
		{
			return this->settings;
		}
	};

private:
	int nDims, nModes, nSettings, current;
	ModeDim* dims[NMODETYPES];
	vector<double> durs;

	// Updates the int members after a dimension is added
	void updateCounters()
	{
		int newModes = dims[nDims]->getMax();

		if (nModes == 0)
			nModes += newModes;
		else
			nModes *= newModes;
		nSettings += dims[nDims]->getSettings();
		nDims++;
	}

public:
	// Constructor
	Mode()
	{
		nDims = 0;
		nModes = 0;
		nSettings = 0;
		current = 0;
		durs = vector<double>();
	}

	// Initialiser (to update according to specific tests to be run)
	static Mode Init()
	{
		Mode mode = Mode();

#ifdef MODECUTS
		string names[MODECUTS + 2] = { "No", "Set", "Pre", "Res", "Onl", "Cuts" };

		mode.AddCombDim(MODECUTS, names);
#endif // MODECUTS

#ifdef MODETESTS
		string names2[3] = {"Test1", "Test2", "Test3"};
		mode.AddDim(MODETESTS, names2);
#endif // MODETESTS

		mode.durs.resize(mode.nModes);

		return mode;
	}
	
	// Move on to the next state (returns True if end is reached)
	bool Next()
	{
		current++;
		int res = 0;
		for (int i = 0; i < nDims; ++i)
		{
			res = dims[i]->next();
			if (res != -1)
				break;
		}
		return res == -1;
	}

	// Sets everything back to the starting state
	void Reset()
	{
		for (int i = 0; i < NMODETYPES; ++i)
			dims[i]->setCurr(0);
	}

	// Sets the duration the current run took
	void SetCurrDur(double dur)
	{
		durs[current] = dur;
	}

	/* Functions to add Dimensions: */
#pragma region Add Dims
	// Adds a named regular dimension 
	void AddDim(int max, string name)
	{
		dims[nDims] = new ModeDim(name, 0, max);
		updateCounters();
	}

	// Adds a regular dimension with each option named
	void AddDim(int max, string* names)
	{
		dims[nDims] = new ModeDim(names, 0, max);
		updateCounters();
	}

	// Adds a named combination dimension
	void AddCombDim(int max, string name)
	{
		dims[nDims] = new ModeDim(name, 1, max);
		updateCounters();
	}

	// Adds a combination dimension with each option named
	void AddCombDim(int max, string* names)
	{
		dims[nDims] = new ModeDim(names, 1, max);
		updateCounters();
	}

#pragma endregion
	
	/* Functions to get states: */
#pragma region Getters
	// Get current mode for specific dimension
	int GetCurrent(int dim)
	{
		return dims[dim]->getCurr();
	}

	// Get current mode for specific combination-dimension
	int GetCurrentComb(int dim, int index)
	{
		return dims[dim]->getCurr(index);
	}

	// Get current value of a named setting
	int GetCurrentBySettingName(string name)
	{
		for (int i = 0; i < nDims; ++i)
		{
			int res = dims[i]->getCurr(name);
			if (res != -1)
				return res;
		}
		return -1;
	}

	// Gets the ID of the current state
	int GetID()
	{
		return current;
	}

	// Get (mode)name of current mode
	string GetCurrentModeName()
	{
		string name = "";
		for (int i = 0; i < nDims; ++i)
			name += " " + dims[i]->getModeName();
		return name.substr(1);
	}

	// Get (mode)name of current mode for specific dimension
	string GetCurrentModeName(int dim)
	{
		return dims[dim]->getModeName();
	}

	// Get (setting)name of current mode for specific dimension
	string GetCurrentSettingName(int dim)
	{
		return dims[dim]->getSetName();
	}

	// Get the number of different modes (i.e. adding a binary dimension DOUBLES number of modes)
	int GetNModes()
	{
		return nModes;
	}

	// Get the number of different settings (i.e. adding a binary dimension ADDS TWO to the number of settings)
	int GetNSettings()
	{
		return nSettings;
	}

	// Get the duration a given mode took
	double GetDur(int mode)
	{
		return durs[mode];
	}

	// Gets the list of names for each mode
	vector<string> GetModeNames()
	{
		Reset();

		vector<string> res;

		for (bool stop = false; !stop; stop = Next())
			res.push_back(GetCurrentModeName());

		return res;
	}

	// Gets the list of names for each setting
	vector<string> GetSettingNames()
	{
		vector<string> res = vector<string>();

		for (int i = 0; i < nDims; ++i)
			for (int j = 0; j < dims[i]->getSettings(); ++j)
				res.push_back(dims[i]->getSetName(j));

		return res;
	}

	// Gets a list of bools indexed by setting to show which setting is on and which is off, in the current status
	vector<bool> GetSettingStatus()
	{
		vector<bool> res = vector<bool>();

		for (int i = 0; i < nDims; ++i)
		{
			vector<bool> curr = dims[i]->getSetStat();
			res.insert(res.end(), curr.begin(), curr.end());
		}

		return res;
	}

	// Get a list of average durations per settings (to be run after all runs are completed)
	vector<double> GetSettingDurs()
	{
		Reset();

		vector<double> res(nSettings, 0.0); 
		vector<int> counts(nSettings, 0);

		bool stop = false;

		for (int i = 0; i < nModes && !stop; ++i)
		{
			vector<bool> curs = GetSettingStatus();

			for (int j = 0; j < nSettings; ++j)
				if (curs[j])
				{
					res[j] += durs[i];
					counts[j] += 1;
				}

			stop = Next();
		}

		for (int i = 0; i < nSettings; ++i)
			res[i] = res[i] / counts[i];

		return res;
	}

	// Add other getters as needed
#pragma endregion
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
	void printProbOutput(XPRBprob* prob, int id)
	{
		if (prob->getProbStat() == 1)
			return;

		ofstream file;
		file.open(OUTPUTFILE + to_string(id) + OUTPUTEXT);

		printObj(&file, prob);
		printTurbines(&file);
		printResources(&file);
		printTasks(&file);

		file.close();
	}

	void printModeOutput(Mode* m, bool opt)
	{
		cout << "----------------------------------------------------------------------------------------" << endl;

#ifdef OPTIMAL
		if (opt)
			cout << "All solutions are optimal" << endl;
		else
			cout << "Not all solutions are optimal" << endl;
#endif // OPTIMAL
		
		vector<string> modeNames = m->GetModeNames();

		for (int i = 0; i < m->GetNModes(); ++i)
			cout << "MODE: " << i << " (" << modeNames[i] << ") DUR: " << m->GetDur(i) << endl;

#if NMODETYPES > 1
		vector<double> setAvgs = m->GetSettingDurs();
		vector<string> settingNames = m->GetSettingNames();

		for (int i = 0; i < m->GetNSettings(); ++i)
			cout << "SETTING: " << settingNames[i] << " DUR: " << setAvgs[i] << endl;
#endif // NMODETYPES > 1
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
	void genOriProblem(XPRBprob* prob, Mode* m)
	{
		clock_t start = clock();

		genDecisionVariables(prob);
		genObjective(prob);

		outputPrinter.printer("Initialising Original constraints", 1);

		genSetConstraints(prob, m->GetCurrentBySettingName("SetCuts") == 1);
		genPrecedenceConstraints(prob, m->GetCurrentBySettingName("PreCuts") == 1);
		genResourceConstraints(prob, m->GetCurrentBySettingName("ResCuts") == 1);
		genOnlineConstraints(prob, m->GetCurrentBySettingName("OnlCuts") == 1);

		double duration = ((double)clock() - start) / (double)CLOCKS_PER_SEC;
		outputPrinter.printer("Duration of initialisation: " + to_string(duration) + " seconds", 1);
	}

	void genFullProblem(XPRBprob* prob, Mode* m)
	{
		clock_t start = clock();

		outputPrinter.printer("Initialising Full constraints", 1);

		if (m->GetCurrentBySettingName("SetCuts") == 1)
			genSetConstraints(prob, false); 
		if (m->GetCurrentBySettingName("PreCuts") == 1)
			genPrecedenceConstraints(prob, false);
		if (m->GetCurrentBySettingName("ResCuts") == 1)
			genResourceConstraints(prob, false);
		if (m->GetCurrentBySettingName("OnlCuts") == 1)
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
	bool opt = true;

	srand(SEED);

	dataReader.readData();

	Mode m = Mode::Init();

	for (bool stop = false; !stop; stop = m.Next())
	{
		int id = m.GetID();
		string modeName = m.GetCurrentModeName();
		XPRBprob* p = &probs[id];

		cout << "----------------------------------------------------------------------------------------" << endl;
		cout << "MODE: " << id << " (" << modeName << ")" << endl;

		string name = "Install" + to_string(id);
		p->setName(name.c_str());

		if (NAMES == 0)
			p->setDictionarySize(XPRB_DICT_NAMES, 0);

		clock_t start = clock();

		problemGen.genOriProblem(p, &m);
		problemSolver.solveProblem(p, name);

		if (m.GetCurrentModeName(0).compare("NoCuts") != 0)
		{
			problemGen.genFullProblem(p, &m);
			problemSolver.solveProblem(p, name);
		}

		outputPrinter.printProbOutput(p, id);

#ifdef OPTIMAL
		opt &= round(p->getObjVal()) == OPTIMAL;
#endif // OPTIMAL

		double duration = ((double)clock() - start) / (double)CLOCKS_PER_SEC;
		cout << "FULL duration: " << duration << " seconds" << endl;
		m.SetCurrDur(duration);
	}

	outputPrinter.printModeOutput(&m, opt);

	return 0;
}
