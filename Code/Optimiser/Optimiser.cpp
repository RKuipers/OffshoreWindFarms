#include "Optimiser.h"

// ----------------------------Constructor function----------------------------

Optimiser::Optimiser(int nPeriods, int nRes, int nTasks, int nTimes, int nAssets, string name, const WeatherGenerator& wg) : nPeriods(nPeriods), baseName(name), wg(wg), optimal(INT_MIN)
{
	C = vector<vector<int>>(nRes, vector<int>(nPeriods));
	d = vector<int>(nTasks);
	sa = vector<vector<int>>(nTasks, vector<int>(nTimes));
	rho = vector<vector<int>>(nRes, vector<int>(nTasks));
	m = vector<vector<int>>(nRes, vector<int>(nPeriods));
	N = vector<vector<XPRBvar>>(nRes, vector<XPRBvar>(nPeriods));
	o = vector<vector<XPRBvar>>(nAssets, vector<XPRBvar>(nTimes));
	sp = vector<vector<vector<XPRBvar>>>(nAssets, vector<vector<XPRBvar>>(nTasks, vector<XPRBvar>(nTimes)));
}

// -----------------------------Reading functions------------------------------

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

vector<vector<string>> Optimiser::parseSection(ifstream* datafile, string name, bool canCopy, int expectedAmount)
{
	string line;
	vector<string>* split = new vector<string>();
	int nVals, copies, first = -1;
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
			copies = 1;

		for (int i = 0; i < copies; ++i)
		{
			vector<string> toAdd(*split);
			if (first != -1)
				toAdd[0] = to_string(first + i);
			res.push_back(toAdd);
		}

		if (!datafile->eof())
			getline(*datafile, line);
		else
			line = "";
	}

	if (res.size() != nVals)
		cout << "Error: declared amount of " << name << " does not match the actual amount" << endl;

	return res;
}

// ----------------------------Generator functions-----------------------------

void Optimiser::genBasicProblem(XPRBprob* prob, Mode* m)
{
	printer("Initialising variables and objective", VERBINIT);

	genDecisionVariables(prob);
	genObjective(prob);
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

// ------------------------------Print functions-------------------------------

void Optimiser::printWeather(vector<int> waveheights)
{
	printer("Wave heights per timestep:", VERBWEAT);
	for (int t = 0; t < waveheights.size(); ++t)
		printer(to_string(t) + ": " + to_string(waveheights[t]), VERBWEAT);
}

void Optimiser::printObj(ofstream* file, XPRBprob* prob)
{
	printer("Total return: " + to_string(round(prob->getObjVal())), VERBSOL);
	*file << "Objective: " << prob->getObjVal() << endl;
}

void Optimiser::printTurbines(ofstream* file)
{
	vector<int> vals = vector<int>();

	printer("Online turbines per timestep: ", VERBSOL);
	for (int t = 0; t < o[0].size(); ++t)
	{
		vals.push_back(0);
		for (int a = 0; a < o.size(); ++a)
			vals[t] += round(o[a][t].getSol());

		int v = vals[t];
		if (t == 0 || v != vals[t - 1])
			printer(to_string(t) + ": " + to_string(v), VERBSOL);
		*file << "O_" << t << ": " << v << endl;
	}
}

void Optimiser::printResources(ofstream* file)
{
	printer("Resources needed per period and type: ", VERBSOL);
	for (int p = 0; p < N[0].size(); ++p)
	{
		int v = round(N[0][p].getSol());
		printer(to_string(p) + ": " + to_string(v), VERBSOL, false);
		*file << "N_0_" << p << ": " << v << endl;;

		for (int r = 1; r < N.size(); ++r)
		{
			v = round(N[r][p].getSol());
			printer(", " + to_string(v), VERBSOL, false);
			*file << "N_" << r << "_" << p << ": " << v << endl;;
		}
		printer("", VERBSOL);
	}
}

void Optimiser::printTasks(ofstream* file)
{
	printer("Start and finish time per asset and task: ", VERBSOL);
	for (int a = 0; a < s.size(); ++a)
	{
		printer("Asset: " + to_string(a), VERBSOL);
		for (int i = 0; i < s[a].size(); ++i)
		{
			int start = -1;
			int finish = -1;

			for (int t = 0; t < s[a][i].size(); ++t)
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
				for (int t1 = start + d[i] - 1; t1 <= sa[i].size(); ++t1)
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

void Optimiser::printProbOutput(XPRBprob* prob, Mode* m, int id)
{
	if (prob->getProbStat() == 1)
		return;

	ofstream file;
	file.open(string() + OUTPUTFOLDER + baseName + to_string(id) + PROBOUTPUTEXT);

	printObj(&file, prob);
	printTurbines(&file);
	printResources(&file);
	printTasks(&file);

	file.close();
}

void Optimiser::printModeOutput(Mode* m)
{
	ofstream file;
	file.open(string() + OUTPUTFOLDER + baseName + "Modes" + MODEOUTPUTEXT);

	printer("----------------------------------------------------------------------------------------", VERBMODE);

	sort(sols.begin(), sols.end());
	if (get<0>(sols[0]) == get<0>(sols[sols.size() - 1]))
	{
		int val = get<0>(sols[0]);
		if (val == optimal || optimal == INT_MIN)
			printer("All solutions are optimal with a value of: " + to_string(val), VERBMODE);
		else
			printer("All solutions gave the value " + to_string(val) + " but the expected value was " + to_string(optimal), VERBMODE);
	}
	else
	{
		printer("Not all solutions are optimal", VERBMODE);
		printer("Worst solution is: " + to_string(get<0>(sols[0])) + " for mode " + get<1>(sols[0]), VERBMODE);
		printer("Best solution is: " + to_string(get<0>(sols[sols.size() - 1])) + " for mode " + get<1>(sols[sols.size() - 1]), VERBMODE);
	}

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

	if (m->GetNDims() <= 1)
		return;

	file.open(string() + OUTPUTFOLDER + baseName + "Settings" + MODEOUTPUTEXT);

	vector<string> settingNames = m->GetSettingNames();
	vector<double> setAvgs = m->GetSettingDurs();

	for (int i = 0; i < m->GetNSettings(); ++i)
	{
		printer("SETTING: " + settingNames[i] + " DUR: " + to_string(setAvgs[i]), VERBMODE);
		file << settingNames[i] << ";" << setAvgs[i] << endl;
	}

	file.close();
	file.open(string() + OUTPUTFOLDER + baseName + "Submodes" + MODEOUTPUTEXT);

	vector<string> subModeNames = m->GetCombModeNames();
	vector<double> subModeAvgs = m->GetModeDurs(subModeNames);

	for (int i = 0; i < subModeNames.size(); ++i)
	{
		if (!isnan(subModeAvgs[i]))
			printer("SUBMODE: " + subModeNames[i] + " DUR: " + to_string(subModeAvgs[i]), VERBMODE);
		file << subModeNames[i] << ";" << subModeAvgs[i] << endl;
	}

	file.close();

}

int Optimiser::printer(string s, int verbosity, bool end, int maxVerb)
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

string Optimiser::boolVec2Str(vector<bool> vec)
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

// -------------------------------Solve function-------------------------------

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

// --------------------------------Run function--------------------------------

void Optimiser::Run()
{
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

		clock_t start = clock();

		genBasicProblem(p, &mode);
		genPartialProblem(p, &mode);

		double duration = ((double)clock() - start) / (double)CLOCKS_PER_SEC;
		printer("Duration of initialisation: " + to_string(duration) + " seconds", VERBINIT);

		//bool toTune = mode.GetCurrentBySettingName("Tune1") == 1;
		bool toTune = true;

		if (mode.GetCurrentModeName(0).compare("NoCuts") != 0)
		{
			solveProblem(p, toTune, name, maxPTime);
			toTune = false;
			genFullProblem(p, &mode);
		}

		solveProblem(p, toTune, name, maxFTime);

		printProbOutput(p, &mode, id);

		duration = ((double)clock() - start) / (double)CLOCKS_PER_SEC;
		printer("FULL duration: " + to_string(duration) + " seconds", VERBSOL);

		printer("Mode: " + to_string(id) + ", duration: " + to_string(duration) + " seconds, Solution: " + to_string(round(p->getObjVal())), VERBMODE, true, VERBSOL);
		mode.SetCurrDur(duration);

		sols.push_back(make_tuple(round(p->getObjVal()), modeName));
	}

#ifndef LOCKMODE
	printModeOutput(&mode);
#endif // !LOCKMODE
}