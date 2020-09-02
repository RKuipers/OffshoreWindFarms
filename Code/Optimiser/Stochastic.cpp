#include "Stochastic.h"

// ----------------------------Constructor function----------------------------

Stoch::Stoch() : Optimiser(NPERIODS, NRES, NPTASKS, NTIMES, NASSETS, PROBNAME, WeatherGenerator(BASE, VARIETY, NTIMES, TPP))
{
#ifdef OPTIMAL
	optimal = OPTIMAL;
#endif // OPTIMAL

	v = vector<vector<int>>(NTIMES, vector<int>(NSCENARIOS));
	lambda = vector<vector<vector<int>>>(NASSETS, vector<vector<int>>(NTASKS+1, vector<int>(NSCENARIOS)));

	o = vector<vector<vector<XPRBvar>>>(NASSETS, vector<vector<XPRBvar>>(NTIMES, vector<XPRBvar>(NSCENARIOS)));
	sp = vector<vector<vector<XPRBvar>>>(NASSETS, vector<vector<XPRBvar>>(NPTASKS, vector<XPRBvar>(NTIMES)));
	sc = vector<vector<vector<vector<XPRBvar>>>>(NASSETS, vector<vector<vector<XPRBvar>>>(NCTASKS, vector<vector<XPRBvar>>(NTIMES, vector<XPRBvar>(NSCENARIOS))));

	maxPTime = MAXPRETIME; 
	maxFTime = MAXFULLTIME;
}

Mode Stoch::initMode()
{
	Mode mode = Mode();

#ifdef MODECUTS
	string names[MODECUTS + 2] = { "No", "Set", "Fin", "Res", "Fai", "Cor", "Dow", "Cuts" };
	mode.AddCombDim(MODECUTS, names);
#endif // MODECUTS

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
	vector<vector<vector<int>>> parsed(lines.size(), vector<vector<int>>(NSCENARIOS, vector<int>(NASSETS)));
	vector<int> locs(lines.size(), 1);
	locs[0] = 0;

	for (int s = 0; s < NSCENARIOS; ++s)
	{
		locs[0] = parsePeriodical(lines[0][locs[0]][0], lines[0], locs[0] + 1, &parsed[lines.size() - 1][s], NASSETS);

		for (int i = 1; i < lines.size(); ++i)
			locs[i] = parsePeriodical(lines[i][locs[i]][0], lines[i], locs[i] + 1, &parsed[i - 1][s], NASSETS);
	}

	for (int i = 0; i < parsed.size(); ++i)
		for (int s = 0; s < parsed[0].size(); ++s)
			for (int a = 0; a < parsed[0][0].size(); ++a)
				lambda[a][i][s] = parsed[i][s][a];
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
			for (int i = 0; i < NPTASKS; ++i)
				sp[a][i][t] = prob->newVar(("sp_" + to_string(a) + "_" + to_string(i) + "_" + to_string(t)).c_str(), XPRB_BV);

			for (int s = 0; s < NSCENARIOS; ++s)
			{
				o[a][t][s] = prob->newVar(("o_" + to_string(a) + "_" + to_string(t) + "_" + to_string(s)).c_str(), XPRB_BV);

				for (int i = 0; i < NCTASKS; ++i)
					sc[a][i][t][s] = prob->newVar(("sc_" + to_string(a) + "_" + to_string(i) + "_" + to_string(t) + "_" + to_string(s)).c_str(), XPRB_BV);
			}
		}
}

void Stoch::genObjective(XPRBprob* prob)
{
	XPRBctr Obj = prob->newCtr();
	for (int s = 0; s < NSCENARIOS; ++s)
		for (int p = 0; p < NPERIODS; ++p)
		{
			double dis = pow(DIS, p) / NSCENARIOS;

			for (int t = p * TPP; t < (p + 1) * TPP; ++t)
				for (int a = 0; a < NASSETS; ++a)
					Obj.addTerm(o[a][t][s], v[t][s] * dis);

			for (int r = 0; r < NRES; ++r)
				Obj.addTerm(N[r][p], -C[r][p] * dis);
		}
	prob->setObj(Obj); // Set the objective function
}

void Stoch::genSetConstraints(XPRBprob* prob, bool cut)
{
	// Once a task has started it stays started
	for (int a = 0; a < NASSETS; ++a)
		for (int t = 1; t < NTIMES; ++t)
		{
			for (int i = 0; i < NPTASKS; ++i)
			{
				XPRBrelation rel = sp[a][i][t] >= sp[a][i][t - 1];

				int indices[3] = { a, i, t };
				genCon(prob, rel, "SetP", 3, indices, cut);
			}

			for (int s = 0; s < NSCENARIOS; ++s)
				for (int i = 0; i < NCTASKS; ++i)
				{
					XPRBrelation rel = sc[a][i][t][s] >= sc[a][i][t - 1][s];

					int indices[4] = { a, i, t, s };
					genCon(prob, rel, "SetC", 3, indices, cut);
				}
		}
}

void Stoch::genFinishConstraints(XPRBprob* prob, bool cut)
{
	// Forces every non-optional task to finish
	for (int a = 0; a < NASSETS; ++a)
		for (int i = 0; i < NPTASKS; ++i)
		{
			XPRBrelation rel = sp[a][i][sa[i][NTIMES]] == 1;

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

					for (int a = 0; a < NASSETS; ++a)
						if (i < NPTASKS)
						{
							rel.addTerm(sp[a][i][t], -rho[r][i]);
							if (t > 0)
								if (sa[i][t] > -1)
									rel.addTerm(sp[a][i][sa[i][t]], rho[r][i]);
								else
									continue;
						}
						else
							for (int s = 0; s < NSCENARIOS; ++s)
							{
								int j = i - NPTASKS;

								rel.addTerm(sc[a][j][t][s], -rho[r][j]);
								if (t > 0)
									if (sa[j][t] > -1)
										rel.addTerm(sc[a][j][sa[j][t]][s], rho[r][j]);
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
	for (int s = 0; s < NSCENARIOS; ++s)
		for (int t = 0; t < NTIMES; ++t)
			for (int a = 0; a < NASSETS; ++a)
			{
				if (t < lambda[a][NTASKS][s]) // Active time from installation
					continue;

				XPRBrelation rel = o[a][t][s] <= 0;

				for (int i = 0; i < NPTASKS; i++)
				{
					if (sa[i][t] > -1)
						rel.addTerm(sp[a][i][sa[i][t]], -1);
					if (t - lambda[a][i][s] >= 0 && sa[i][t - lambda[a][i][s]] > -1)
						rel.addTerm(sp[a][i][sa[i][t - lambda[a][i][s]]]);
				}
				for (int j = 0; j < NCTASKS; j++)
				{
					int i = j + NPTASKS;

					if (sa[i][t] > -1)
						rel.addTerm(sc[a][i][sa[i][t]][s], -1);
					if (t - lambda[a][i][s] >= 0 && sa[i][t - lambda[a][i][s]] > -1)
						rel.addTerm(sc[a][i][sa[i][t - lambda[a][i][s]]][s]);
				}

				int indices[3] = { a, t, s};
				genCon(prob, rel, "Fai", 3, indices, cut);
			}
}

void Stoch::genCorrectiveConstraints(XPRBprob* prob, bool cut)
{
	double factor = 1 / ((double)NTASKS);

	// Turbines can only be correctively repaired if they are broken
	for (int s = 0; s < NSCENARIOS; ++s)
		for (int t = 0; t < NTIMES; ++t)
			for (int a = 0; a < NASSETS; ++a)
				for (int i = NPTASKS; i < NTASKS; ++i)
				{
					if (t < lambda[a][NTASKS][s]) // Active time from installation
					{
						sc[a][i][t][s].setUB(0);
						continue;
					}

					XPRBrelation rel = sc[a][i][t][s] - sc[a][i][t - 1][s] <= 1;

					for (int j = 0; j < NPTASKS; ++j)
					{
						if (sa[j][t] > -1)
							rel.addTerm(sp[a][j][sa[j][t]], factor);
						if (t - lambda[a][j][s] >= 0 && sa[j][t - lambda[a][j][s]] > -1)
							rel.addTerm(sp[a][j][sa[j][t - lambda[a][j][s]]], -factor);
					}

					for (int k = 0; k < NCTASKS; ++k)
					{
						int j = k + NPTASKS;

						if (sa[j][t] > -1)
							rel.addTerm(sc[a][j][sa[j][t]][s], factor);
						if (t - lambda[a][j][s] >= 0 && sa[j][t - lambda[a][j][s]] > -1)
							rel.addTerm(sc[a][j][sa[j][t - lambda[a][j][s]]][s], -factor);
					}

					int indices[4] = { a, i, t, s };
					genCon(prob, rel, "Cor", 4, indices, cut);
				}
}

void Stoch::genDowntimeConstraints(XPRBprob* prob, bool cut)
{
	// Turbines are offline while maintenance work is ongoing
	for (int s = 0; s < NSCENARIOS; ++s)
		for (int t = 0; t < NTIMES; ++t)
			for (int a = 0; a < NASSETS; ++a)
				for (int i = 0; i < NTASKS; ++i)
				{
					XPRBrelation rel = o[a][t][s] <= 1 - sp[a][i][t];
					if (sa[i][t] >= 0)
						rel.addTerm(sp[a][i][sa[i][t]], -1);

					if (i >= NPTASKS)
					{
						rel = o[a][t][s] <= 1 - sc[a][i][t][s];
						if (sa[i][t] >= 0)
							rel.addTerm(sc[a][i][sa[i][t]][s], -1);
					}

					int indices[4] = { a, i, t, s };
					genCon(prob, rel, "Down", 4, indices, cut);
				}
}

void Stoch::genPartialProblem(XPRBprob* prob, Mode* m)
{
	printer("Initialising Original constraints", VERBINIT);

	genSetConstraints(prob, m->GetCurrentBySettingName("SetCuts") == 1);
	genFinishConstraints(prob, m->GetCurrentBySettingName("FinCuts") == 1);
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
		genFinishConstraints(prob, false);
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

// ------------------------------Print functions-------------------------------

void Stoch::printTurbines(ofstream* file)
{
	vector<int> vals = vector<int>();

	for (int s = 0; s < NSCENARIOS; ++s)
	{
		printer("Online turbines per timestep in Scenario " + to_string(s) + ": ", VERBSOL);
		for (int t = 0; t < o[0].size(); ++t)
		{
			vals.push_back(0);
			for (int a = 0; a < o.size(); ++a)
				vals[t] += round(o[a][t][s].getSol());

			int v = vals[t];
			if (t == 0 || v != vals[t - 1])
				printer(to_string(t) + ": " + to_string(v), VERBSOL);
			*file << "O_" << t << ": " << v << endl;
		}
	}
}

void Stoch::printTasks(ofstream* file)
{
	for (int s = 0; s < NSCENARIOS; ++s)
	{
		printer("Start and finish time per asset and task in Scenario " + to_string(s) + ": ", VERBSOL);
		for (int a = 0; a < NASSETS; ++a)
		{
			printer("Asset: " + to_string(a), VERBSOL);
			for (int i = 0; i < NTASKS; ++i)
			{
				int start = -1;
				int finish = -1;

				if (i < NPTASKS)
					for (int t = 0; t < sp[a][i].size(); ++t)
					{
						int sv = round(sp[a][i][t].getSol());

						*file << "s_" << a << "_" << i << "_" << t << ": " << sv << endl;

						if (sv == 1 && start == -1)
							start = t;
					}
				else
					for (int t = 0; t < sc[a][i-NPTASKS][s].size(); ++t)
					{
						int sv = round(sc[a][i-NPTASKS][t][s].getSol());

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
}
