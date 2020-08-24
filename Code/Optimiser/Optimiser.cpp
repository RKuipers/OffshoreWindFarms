#include "Optimiser.h"

void Optimiser::printWeather(vector<int> waveheights)
{
	printer("Wave heights per timestep:", VERBWEAT); 
	for(int t = 0; t < waveheights.size(); ++t)
		printer(to_string(t) + ": " + to_string(waveheights[t]), VERBWEAT);
}

int Optimiser::printer(string s, int verbosity, bool end = true, int maxVerb = 999)
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

void Optimiser::genBasicProblem(XPRBprob* prob, Mode* m)
{
	printer("Initialising variables and objective", VERBINIT);

	genDecisionVariables(prob);
	genObjective(prob);
}

void Optimiser::solveProblem(XPRBprob* prob, bool tune, string name, int maxTime = 0)
{
	printer("Solving problem", VERBINIT);
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
	printer("Solving duration: " + to_string(duration) + " seconds", VERBSOL);

	const char* MIPSTATUS[] = { "not loaded", "not optimized", "LP optimized", "unfinished (no solution)", "unfinished (solution found)", "infeasible", "optimal", "unbounded" };
	printer(string() + "Problem status: " + MIPSTATUS[prob->getMIPStat()], VERBSOL);
}

void Optimiser::Run(string baseName, int maxPTime = 0, int maxFTime = 0)
{
	bool opt = true;

	readData();

	Mode mode = initMode();

	for (bool stop = false; !stop; stop = mode.Next())
	{
		int id = mode.GetID();

		string modeName = mode.GetCurrentModeName();
		XPRBprob* p = new XPRBprob();

		printer("----------------------------------------------------------------------------------------", VERBSOL);
		printer("MODE: " + to_string(id) + " (" + modeName + ")", VERBSOL);

		string name = baseName + to_string(id);
		p->setName(name.c_str());

		if (NAMES == 0)
			p->setDictionarySize(XPRB_DICT_NAMES, 0);

		clock_t start = clock();

		genBasicProblem(p, &mode);
		genPartialProblem(p, &mode);

		double duration = ((double)clock() - start) / (double)CLOCKS_PER_SEC;
		printer("Duration of initialisation: " + to_string(duration) + " seconds", VERBINIT);

		bool toTune = mode.GetCurrentBySettingName("Tune1") == 1;

		if (mode.GetCurrentModeName(0).compare("NoCuts") != 0)
		{
			solveProblem(p, toTune, name, maxPTime);
			toTune = false;
			genFullProblem(p, &mode);
		}

		solveProblem(p, toTune, name, maxFTime);

		printProbOutput(p, &mode, id);

#ifdef OPTIMAL
		opt &= round(p->getObjVal()) == OPTIMAL;
#endif // OPTIMAL

		duration = ((double)clock() - start) / (double)CLOCKS_PER_SEC;
		printer("FULL duration: " + to_string(duration) + " seconds", VERBSOL);

		printer("Mode: " + to_string(id) + ", duration: " + to_string(duration) + " seconds, Solution: " + to_string(round(p->getObjVal())), VERBMODE, true, VERBSOL);
		mode.SetCurrDur(duration);
	}

#ifndef LOCKMODE
	printModeOutput(&mode, opt);
#endif // !LOCKMODE
}

void Optimiser::readResources(ifstream* datafile)
{
	vector<vector<string>> lines = parseSection(datafile, "RESOURCES");

	for (int r = 0; r < lines.size(); ++r)
	{
		int loc = 1;
		char type = lines[r][loc][0];
		loc = parsePeriodical(type, lines[r], loc + 1, &C[r], nPeriods);

		type = lines[r][loc][0];
		loc = parsePeriodical(type, lines[r], loc + 1, &m[r], nPeriods);
	}
}

void Optimiser::splitString(string s, vector<string>* res, char sep)
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

int Optimiser::parsePeriodical(char type, vector<string> line, int start, vector<int>* res, int amount)
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

vector<vector<string>> Optimiser::parseSection(ifstream* datafile, string name, bool canCopy = true, int expectedAmount = -1)
{
	string line;
	vector<string>* split = new vector<string>();
	int nVals, copies, first;
	vector<vector<string>> res;

	getline(*datafile, line);
	splitString(line, split);
	if ((*split)[0].compare(name) != 0)
		cout << "Error: Expected section " << name << " but did not find it" << endl;
	nVals = stoi((*split)[1]);

	if (expectedAmount != -1 && expectedAmount != nVals)
		cout << "Error: declared amount of " << name << " does not match the expected amount" << endl;

	getline(*datafile, line);
	while (line.compare("") != 0)
	{
		splitString(line, split, '\t');

		if (canCopy && (*split)[0].find(" ") != string::npos)
		{
			vector<string>* dups = new vector<string>();
			splitString((*split)[0], dups, ' ');
			first = stoi((*dups)[0]);
			copies = stoi((*dups)[1]) - first;
		}
		else
		{
			if (canCopy)
				first = stoi((*split)[0]);
			copies = 1;
		}

		for (int i = 0; i < copies; ++i)
		{
			vector<string> toAdd(split->size());
			copy(split->begin(), split->end(), toAdd);
			if (canCopy)
				toAdd[0] = first;
			res.push_back(toAdd);
		}
	}

	return res;
}

void Optimiser::genCon(XPRBprob* prob, const XPRBrelation& ac, string base, int nInd, int* indices, bool cut)
{
	if (cut)
		prob->newCut(ac);
	else
	{
		string name = base;

		for (int i = 0; i < nInd; ++i)
			name += "_" + to_string(indices[i]);

		prob->newCtr(name.c_str(), ac);
	}
}
