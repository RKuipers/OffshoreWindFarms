#include "Deter.h"

Mode Deter::initMode()
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

void Deter::readData()
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
	readTasks(&datafile, 2, &limits);
	readTasks(&datafile, 3, &limits);

	// Read the resource info
	readResources(&datafile);

	// Read the energy value info
	readValues(&datafile);

	// Read the lambda info
	readLambdas(&datafile);

	// Read the task order info
	readPreqs(&datafile);

	// Generate the weather and StartAt values
	vector<int> waveheights = wg.generateWeather();
	printWeather(waveheights);
	wg.generateStartValues(d, limits);

	datafile.close();
}

void Deter::readTasks(ifstream* datafile, int taskType, vector<int>* limits)
{
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

	vector<vector<string>> lines = parseSection(datafile, name, true, ntasks);

	for (int i = 0; i < ntasks; ++i)
	{
		d[i] = stoi(lines[i][1]);
		limits->push_back(stoi(lines[i][2]));

		for (int r = 0; r < lines[i].size()-3; ++r)
			rho[r][i] = stoi(lines[i][r + 3]);
	}
}

void Deter::readValues(ifstream* datafile)
{
	vector<vector<string>> lines = parseSection(datafile, "VALUES", false);

	parsePeriodical(lines[0][0][0], lines[1], 0, &v, nPeriods);
}

void Deter::readLambdas(ifstream* datafile)
{
	vector<vector<string>> lines = parseSection(datafile, "LAMBDAS");
	vector<vector<int>> flipped(lines.size(), vector<int>(NASSETS));

	int offset = stoi(lines[0][0]);

	for (int i = 0; i < lines.size(); ++i)
		parsePeriodical(lines[i][1][0], lines[i], 2, &flipped[i], NASSETS);

	for (int x = 0; x < flipped.size(); ++x)
		for (int y = 0; y < flipped[x].size(); ++y)
			lambda[x][y] = flipped[y][x];
}

void Deter::readPreqs(ifstream* datafile)
{
	vector<vector<string>> lines = parseSection(datafile, "PREREQUISITES", false);

	for (int x = 0; x < lines.size(); ++x)
		IP.push_back(make_tuple(stoi(lines[x][0]), stoi(lines[x][1])));
}

void Deter::genDecisionVariables(XPRBprob* prob)
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

void Deter::genObjective(XPRBprob* prob)
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

void Deter::genPartialProblem(XPRBprob* prob, Mode* m)
{
	printer("Initialising Original constraints", VERBINIT);

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

void Deter::genFullProblem(XPRBprob* prob, Mode* m)
{
	clock_t start = clock();

	printer("Initialising Full constraints", VERBINIT);

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
	printer("Duration of initialisation: " + to_string(duration) + " seconds", VERBINIT);
}

void Deter::printProbOutput(XPRBprob* prob, Mode* m, int id)
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

void Deter::printModeOutput(Mode* m, bool opt)
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
