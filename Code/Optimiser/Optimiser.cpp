#include "Optimiser.h"

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

void Optimiser::Run()
{
	bool opt = true;

	srand(SEED);

	readData();

	Mode mode = initMode();

	for (bool stop = false; !stop; stop = mode.Next())
	{
		int id = mode.GetID();

		string modeName = mode.GetCurrentModeName();
		XPRBprob* p = new XPRBprob();

		printer("----------------------------------------------------------------------------------------", VERBSOL);
		printer("MODE: " + to_string(id) + " (" + modeName + ")", VERBSOL);

		string name = "Life" + to_string(id);
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
			solveProblem(p, toTune, name, MAXPRETIME);
			toTune = false;
			genFullProblem(p, &mode);
		}

		solveProblem(p, toTune, name, MAXFULLTIME);

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
