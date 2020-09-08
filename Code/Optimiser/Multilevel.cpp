#include "Multilevel.h"

// ----------------------------Constructor function----------------------------

MultiLevel::MultiLevel()
{
	name = "Multilevel";

	V = 2;
	M = 12;
	S = 5;

	C = vector<vector<double>>(V, vector<double>(M));
	eh = vector<double>(M);
	em = vector<double>(M);
	dpv = vector<int>(V);
	drv = vector<int>(V);
	f = vector<vector<int>>(M, vector<int>(S));
	l = vector<int>(V);
	P = vector<XPRBvar>(M);
	R = vector<vector<XPRBvar>>(M, vector<XPRBvar>(S));
	N = vector<vector<vector<XPRBvar>>>(V, vector<vector<XPRBvar>>(M, vector<XPRBvar>(S)));
}

void MultiLevel::Run()
{
	XPRBprob p("MultiLevel");
	getData();

	genProblem(&p);

	p.setSense(XPRB_MINIM);
	p.exportProb(XPRB_LP, (OUTPUTFOLDER + name).c_str());
	p.mipOptimize("");

	printProbOutput(&p);
}

// ------------------------------Data functions--------------------------------

void MultiLevel::genScenario(int id, double expected)
{
	double exp = expected / M;
	int max = (int)floor(exp * 2);
	vector<int> maxs(M, max);

	if (expected * 2 > max * M) 
	{
		vector<unsigned int> indices(M);
		iota(indices.begin(), indices.end(), 0);
		random_shuffle(indices.begin(), indices.end());
		for (int x = 0; x < (expected * 2) - (max * M); ++x)
			maxs[indices[x]]++;
	}

	for (int m = 0; m < M; ++m)
		f[m][id] = rand() % (maxs[m] + 1);
}

void MultiLevel::getData()
{
	l = { 720, 480 };
	A = 100;
	dp = 12;
	dpv = {4, 12};
	drv = {8, 16};
	eh = { 540, 540, 540, 540, 540, 540, 540, 540, 540, 540, 540, 540 };
	em = { 388800, 388800, 388800, 388800, 388800, 388800, 388800, 388800, 388800, 388800, 388800, 388800 };
	C = { { 500000, 500000, 500000, 500000, 500000, 500000, 500000, 500000, 500000, 500000, 500000, 500000 }, 
		{ 120000, 120000, 120000, 120000, 120000, 120000, 120000, 120000, 120000, 120000, 120000, 120000 } };

	for (int s = 0; s < S; ++s)
		genScenario(s);
}

// ----------------------------Generator functions-----------------------------

void MultiLevel::genDecisionVariables(XPRBprob* prob)
{
	for (int m = 0; m < M; ++m)
	{
		P[m] = prob->newVar(("P_" + to_string(m)).c_str(), XPRB_UI);
		P[m].setLB(0);

		for (int s = 0; s < S; ++s)
		{
			R[m][s] = prob->newVar(("R_" + to_string(m) + to_string(s)).c_str(), XPRB_UI);
			R[m][s].setLB(0);

			for (int v = 0; v < V; ++v)
			{
				N[v][m][s] = prob->newVar(("N_" + to_string(v) + "_" + to_string(m) + to_string(s)).c_str(), XPRB_UI);
				N[v][m][s].setLB(0);
			}
		}
	}
}

void MultiLevel::genObjective(XPRBprob* prob)
{
	XPRBctr Obj = prob->newCtr();
	double sfactor = 1.0 / (double)S;

	for (int s = 0; s < S; ++s)
		for (int m = 0; m < M; ++m)
		{
			for (int v = 0; v < V; ++v)
				Obj.addTerm(N[v][m][s], C[v][m] * sfactor);

			Obj.addTerm(P[m], dp * eh[m] * sfactor);

			for (int m_ = 0; m_ <= m; ++m_)
			{
				Obj.addTerm(R[m_][s], -em[m] * sfactor);
				Obj.add(f[m_][s] * em[m] * sfactor);
			}
		}
	prob->setObj(Obj);
}

void MultiLevel::genCapacityConstraints(XPRBprob* prob)
{
	for (int s = 0; s < S; ++s)
		for (int m = 0; m < M; ++m)
			for (int v = 0; v < V; ++v)
				prob->newCtr(("Cap_" + to_string(m) + "_" + to_string(v)).c_str(), l[v] * N[v][m][s] >= P[m] * dpv[v] + R[m][s] * drv[v]);
}

void MultiLevel::genFailuresConstraints(XPRBprob* prob)
{
	for (int s = 0; s < S; ++s)
	{
		int total = 0;
		for (int m = 0; m < M; ++m)
		{
			if (m > 0)
				total += f[m - 1][s];

			XPRBrelation rel = total - R[0][s] >= 0;

			for (int m_ = 1; m_ <= m; ++m_)
				rel.addTerm(R[m_][s], -1);

			prob->newCtr(("Fai_" + to_string(m)).c_str(), rel);
		}
	}
}

void MultiLevel::genPlannedConstraints(XPRBprob* prob)
{
	XPRBrelation rel = P[0] >= A;

	for (int m = 1; m < M; ++m)
		rel.addTerm(P[m]);

	prob->newCtr("Plan", rel);
}

void MultiLevel::genProblem(XPRBprob* prob)
{
	genDecisionVariables(prob);
	genObjective(prob);

	genCapacityConstraints(prob);
	genFailuresConstraints(prob);
	genPlannedConstraints(prob);
}

// ----------------------------Printing functions------------------------------

void MultiLevel::printObj(ofstream* file, XPRBprob* prob)
{
	printer("Total return: " + to_string(round(prob->getObjVal())), VERBSOL);
	*file << "Objective: " << prob->getObjVal() << endl;
}

void MultiLevel::printFailures(ofstream* file)
{
	vector<int> totals(S, 0);

	printer("Failures per month per scenario: ", VERBSOL);
	for (int m = 0; m < M; ++m)
	{
		int val = f[m][0];
		printer(to_string(m) + ": " + to_string(val), VERBSOL, false);
		*file << "f_" << m << "_0: " << val << endl;;
		totals[0] += val;

		for (int s = 1; s < S; ++s)
		{
			val = f[m][s];
			printer(", " + to_string(val), VERBSOL, false);
			*file << "f_" << m << "_" << s << ": " << val << endl;
			totals[s] += val;
		}
		printer("", VERBSOL);
	}

	printer("Totals: " + to_string(totals[0]), VERBSOL, false);
	for (int s = 1; s < S; ++s)
		printer(", " + to_string(totals[s]), VERBSOL, false);
	printer("", VERBSOL);
}

void MultiLevel::printResources(ofstream* file)
{
	for (int s = 0; s < S; ++s)
	{
		printer("Resources needed per month and vessel type in Scenario " + to_string(s) + ": ", VERBSOL);
		for (int m = 0; m < M; ++m)
		{
			int val = round(N[0][m][s].getSol());
			printer(to_string(m) + ": " + to_string(val), VERBSOL, false);
			*file << "N_0_" << m << ": " << val << endl;;

			for (int v = 1; v < V; ++v)
			{
				val = round(N[v][m][s].getSol());
				printer(", " + to_string(val), VERBSOL, false);
				*file << "N_" << v << "_" << m << ": " << val << endl;;
			}
			printer("", VERBSOL);
		}
	}
}

void MultiLevel::printTasks(ofstream* file)
{
	printer("Planned tasks per month: ", VERBSOL);
	for (int m = 0; m < M; ++m)
	{
		int val = round(P[m].getSol());
		printer(to_string(m) + ": " + to_string(val), VERBSOL);
		*file << "P_" << m << ": " << val << endl;;
	}

	for (int s = 0; s < S; ++s)
	{
		printer("Repair tasks per month in Scenario " + to_string(s) + ": ", VERBSOL);
		for (int m = 0; m < M; ++m)
		{
			int val = round(R[m][s].getSol());
			printer(to_string(m) + ": " + to_string(val), VERBSOL);
			*file << "R_" << m << ": " << val << endl;;
		}
	}
}

void MultiLevel::printProbOutput(XPRBprob* prob)
{
	/*if (prob->getProbStat() == 1)
		return;*/

	ofstream file;
	file.open(string() + OUTPUTFOLDER + name + PROBOUTPUTEXT);

	printObj(&file, prob);
	printFailures(&file);
	printResources(&file);
	printTasks(&file);

	file.close();
}

int MultiLevel::printer(string s, int verbosity, bool end, int maxVerb)
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
