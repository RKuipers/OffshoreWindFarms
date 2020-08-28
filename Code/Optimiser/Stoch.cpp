#include "Stoch.h"

// ----------------------------Constructor function----------------------------

Stoch::Stoch() : Optimiser(NPERIODS, NRES, NTASKS, NTIMES, NASSETS, PROBNAME, WeatherGenerator(BASE, VARIETY, NTIMES, TPP))
{
#ifdef OPTIMAL
	optimal = OPTIMAL;
#endif // OPTIMAL

	v = vector<vector<int>>(NTIMES, vector<int>(NSCENARIOS));
	lambda = vector<vector<int>>(NASSETS, vector<int>(NTASKS+1));
}

Mode Stoch::initMode()
{
	Mode mode = Mode();

#ifdef MODECUTS
	string names[MODECUTS + 2] = { "No", "Set", "Fin", "Res", "Fai", "Cor", "Dow", "Cuts" };
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

#ifdef LOCKFIN
	mode.LockSetting("FinCuts", LOCKFIN);
#endif // LOCKFIN

#ifdef LOCKRES
	mode.LockSetting("ResCuts", LOCKRES);
#endif // LOCKRES

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

// -----------------------------Reading functions------------------------------

void Stoch::readTasks(ifstream* datafile, int taskType, vector<int>* limits)
{
	string name;
	int ntasks, start;
	switch (taskType)
	{
	case 0:
		name = "PTASKS";
		ntasks = NPTASKS;
		start = 0;
		break;
	case 1:
		name = "CTASKS";
		ntasks = NCTASKS;
		start = NPTASKS;
		break;
	default:
		name = "";
		ntasks = -1;
		start = -1;
		break;
	}

	vector<vector<string>> lines = parseSection(datafile, name, true, ntasks);

	for (int i = 0; i < ntasks; ++i)
	{
		d[i + start] = stoi(lines[i][1]);
		limits->push_back(stoi(lines[i][2]));

		for (int r = 0; r < lines[i].size() - 3; ++r)
			rho[r][i + start] = stoi(lines[i][r + 3]);
	}
}

void Stoch::readValues(ifstream* datafile)
{
	vector<vector<string>> lines = parseSection(datafile, "VALUES", false, NSCENARIOS);
	vector<vector<int>> flipped(lines.size(), vector<int>(NTIMES));

	for (int i = 0; i < lines.size(); ++i)
		parsePeriodical(lines[i][0][0], lines[i], 1, &flipped[i], NTIMES);

	for (int x = 0; x < flipped[0].size(); ++x)
		for (int y = 0; y < flipped.size(); ++y)
			v[x][y] = flipped[y][x];
}

void Stoch::readLambdas(ifstream* datafile)
{
	vector<vector<string>> lines = parseSection(datafile, "LAMBDAS");
	vector<vector<int>> flipped(lines.size(), vector<int>(NASSETS));

	parsePeriodical(lines[0][0][0], lines[0], 1, &flipped[lines.size() - 1], NASSETS);

	for (int i = 1; i < lines.size(); ++i)
		parsePeriodical(lines[i][1][0], lines[i], 2, &flipped[i-1], NASSETS);

	for (int x = 0; x < flipped[0].size(); ++x)
		for (int y = 0; y < flipped.size(); ++y)
			lambda[x][y] = flipped[y][x];
}

void Stoch::readData()
{
	// Read data from file
	printer("Reading Data", VERBMODE);

	string line;
	ifstream datafile(string() + INPUTFOLDER + PROBNAME + DATAEXT);
	if (!datafile.is_open())
	{
		cout << "Unable to open file" << endl;
		return;
	}

	vector<int> limits;

	// Read the task info
	readTasks(&datafile, 0, &limits);
	readTasks(&datafile, 1, &limits);

	// Read the resource info
	readResources(&datafile);

	// Read the energy value info
	readValues(&datafile);

	// Read the lambda info
	readLambdas(&datafile);

	// Generate the weather and StartAt values
	vector<int> waveheights = wg.generateWeather();
	printWeather(waveheights);
	sa = wg.generateStartValues(d, limits);

	datafile.close();
}

// ----------------------------Generator functions-----------------------------

void Stoch::genDecisionVariables(XPRBprob* prob)
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

void Stoch::genObjective(XPRBprob* prob)
{
	XPRBctr Obj = prob->newCtr();
	for (int s = 0; s < NSCENARIOS; ++s)
		for (int p = 0; p < NPERIODS; ++p)
		{
			double dis = pow(DIS, p);

			for (int t = p * TPP; t < (p + 1) * TPP; ++t)
				for (int a = 0; a < NASSETS; ++a)
					Obj.addTerm(o[a][t], v[t][s] * dis);

			for (int r = 0; r < NRES; ++r)
				Obj.addTerm(N[r][p], -C[r][p] * dis);
		}
	prob->setObj(Obj); // Set the objective function
}

void Stoch::genSetConstraints(XPRBprob* prob, bool cut)
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

void Stoch::genFinishConstraints(XPRBprob* prob, bool cut, bool finAll)
{
	// Forces every non-optional task to finish
	for (int a = 0; a < NASSETS; ++a)
		for (int i = 0; i < NTASKS; ++i)
		{
			XPRBrelation rel = s[a][i][sa[i][NTIMES]] == 1;

			int indices[2] = { a, i };
			genCon(prob, rel, "Fin", 2, indices, cut);
		}
}

void Stoch::genResourceConstraints(XPRBprob* prob, bool cut)
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

void Stoch::genFailureConstraints(XPRBprob* prob, bool cut)
{
	// Turbines can only be online if they're not broken
	for (int t = 0; t < NTIMES; ++t)
		for (int a = 0; a < NASSETS; ++a)
		{
			if (t < lambda[a][NTASKS]) // Active time from installation
				continue;

			XPRBrelation rel = o[a][t] <= 0;

			for (int i = 0; i < NTASKS; i++)
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

void Stoch::genCorrectiveConstraints(XPRBprob* prob, bool cut)
{
	double factor = 1 / ((double)NTASKS);

	// Turbines can only be correctively repaired if they are broken
	for (int t = 0; t < NTIMES; ++t)
		for (int a = 0; a < NASSETS; ++a)
			for (int i = NPTASKS; i < NTASKS; ++i)
			{
				if (t < lambda[a][NTASKS]) // Active time from installation
				{
					s[a][i][t].setUB(0);
					continue;
				}

				XPRBrelation rel = s[a][i][t] - s[a][i][t - 1] <= 1;

				for (int j = 0; j < NTASKS; j++)
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

void Stoch::genDowntimeConstraints(XPRBprob* prob, bool cut)
{
	// Turbines are offline while maintenance work is ongoing
	for (int t = 0; t < NTIMES; ++t)
		for (int a = 0; a < NASSETS; ++a)
			for (int i = 0; i < NTASKS; ++i)
			{
				XPRBrelation rel = o[a][t] <= 1 - s[a][i][t];
				if (sa[i][t] >= 0)
					rel.addTerm(s[a][i][sa[i][t]], -1);

				int indices[3] = { a, i, t };
				genCon(prob, rel, "Down", 3, indices, cut);
			}
}

void Stoch::genPartialProblem(XPRBprob* prob, Mode* m)
{
	printer("Initialising Original constraints", VERBINIT);

	genSetConstraints(prob, m->GetCurrentBySettingName("SetCuts") == 1);
	genFinishConstraints(prob, m->GetCurrentBySettingName("FinCuts") == 1, m->GetCurrentBySettingName("FinAll") == 1);
	genResourceConstraints(prob, m->GetCurrentBySettingName("ResCuts") == 1);
	genFailureConstraints(prob, m->GetCurrentBySettingName("FaiCuts") == 1);
	genCorrectiveConstraints(prob, m->GetCurrentBySettingName("CorCuts") == 1);
	genDowntimeConstraints(prob, m->GetCurrentBySettingName("DowCuts") == 1);
}

void Stoch::genFullProblem(XPRBprob* prob, Mode* m)
{
	clock_t start = clock();

	printer("Initialising Full constraints", VERBINIT);

	if (m->GetCurrentBySettingName("SetCuts") == 1)
		genSetConstraints(prob, false);
	if (m->GetCurrentBySettingName("FinCuts") == 1)
		genFinishConstraints(prob, false, m->GetCurrentBySettingName("FinAll") == 1);
	if (m->GetCurrentBySettingName("ResCuts") == 1)
		genResourceConstraints(prob, false);
	if (m->GetCurrentBySettingName("FaiCuts") == 1)
		genFailureConstraints(prob, false);
	if (m->GetCurrentBySettingName("CorCuts") == 1)
		genCorrectiveConstraints(prob, false);
	if (m->GetCurrentBySettingName("DowCuts") == 1)
		genDowntimeConstraints(prob, false);

	double duration = ((double)clock() - start) / (double)CLOCKS_PER_SEC;
	printer("Duration of initialisation: " + to_string(duration) + " seconds", VERBINIT);
}
