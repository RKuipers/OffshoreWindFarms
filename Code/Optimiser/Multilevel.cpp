#include "Multilevel.h"

// ----------------------------Constructor function----------------------------

MultiLevel::MultiLevel()
{
	name = "Multilevel";

	Y = 2;
	M = 12;
	S = 2;

	C = vector<vector<double>>(Y, vector<double>(M));
	eh = vector<double>(M);
	em = vector<double>(M);
	dpy = vector<int>(Y);
	dry = vector<int>(Y);
	f = vector<vector<int>>(M, vector<int>(S));
	l = vector<int>(Y);

	P = vector<XPRBvar>(M);
	R = vector<vector<XPRBvar>>(M, vector<XPRBvar>(S));
	N = vector<vector<vector<XPRBvar>>>(Y, vector<vector<XPRBvar>>(M, vector<XPRBvar>(S)));
	gam = vector<XPRBvar>(S);
	ep = vector<vector<vector<XPRBvar>>>(Y, vector<vector<XPRBvar>>(M, vector<XPRBvar>(S)));

	V = 3;
	I = 5;
	J = I;
	T = 30;

	c = vector<double>(I);
	sd = vector<vector<double>>(Y, vector<double>(I));
	d = vector<vector<double>>(Y, vector<double>(I));
	rho = vector<vector<int>>(Y, vector<int>(I));
	Vy = vector<int>(Y);

	s = vector<XPRBvar>(I);
	a = vector<vector<vector<XPRBvar>>>(V, vector<vector<XPRBvar>>(I, vector<XPRBvar>(J)));

	minDur = vector<double>(I, -1);
}

void MultiLevel::Run(bool top)
{
	XPRBprob p(name.c_str());
	getData();

	if (top)
		genTopProblem(&p);
	else
		genLowProblem(&p);

	p.setSense(XPRB_MINIM);
	p.exportProb(XPRB_LP, (OUTPUTFOLDER + name).c_str());
	p.mipOptimize("");

	if (top)
		printTopProbOutput(&p);
	else
		printLowProbOutput(&p);
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
	dpy = {4, 12};
	dry = {8, 16};
	u = 100;
	eh = vector<double>(M, 540);
	em = vector<double>(M, 388800);
	C = { vector<double>(M, 500000), vector<double>(M, 120000) };
	LARGE = 10000;

	for (int s = 0; s < S; ++s)
		genScenario(s);

	c = vector<double>(I, 12960);
	sd = { vector<double>(I, 0), vector<double>(I, 1) };
	//sd = { vector<double>(I, 0), { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 } };
	d = { vector<double>(I, 2), vector<double>(I, 1) };
	//d = { { 1, 1, 1, 1, 2, 2, 2, 2, 3, 3, 1, 1, 1, 1, 2, 2, 2, 2, 3, 3, 1, 1, 1, 1, 2, 2, 2, 2, 3, 3 }, { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2 } };
	rho = vector<vector<int>>(Y, vector<int>(I, 1));
	Vy = { 2, 1 };
	Vyo = { 0, 2 };
}

// ----------------------------Generator functions-----------------------------

void MultiLevel::genTopDecisionVariables(XPRBprob* prob)
{
	for (int m = 0; m < M; ++m)
	{
		P[m] = prob->newVar(("P_" + to_string(m)).c_str(), XPRB_UI);
		P[m].setLB(0);

		for (int s = 0; s < S; ++s)
		{
			R[m][s] = prob->newVar(("R_" + to_string(m) + "_" + to_string(s)).c_str(), XPRB_UI);
			R[m][s].setLB(0);

			for (int y = 0; y < Y; ++y)
			{
				N[y][m][s] = prob->newVar(("N_" + to_string(y) + "_" + to_string(m) + to_string(s)).c_str(), XPRB_UI);
				N[y][m][s].setLB(0);

				ep[y][m][s] = prob->newVar(("e_" + to_string(y) + "_" + to_string(m) + to_string(s)).c_str(), XPRB_BV);
			}
		}
	}

	for (int s = 0; s < S; ++s)
	{
		gam[s] = prob->newVar(("y_" + to_string(s)).c_str(), XPRB_PL);
		gam[s].setLB(0);
	}
}

void MultiLevel::genTopObjective(XPRBprob* prob)
{
	XPRBctr Obj = prob->newCtr();
	double sfactor = 1.0 / (double)S;

	for (int s = 0; s < S; ++s)
	{
		for (int m = 0; m < M; ++m)
		{
			for (int y = 0; y < Y; ++y)
				Obj.addTerm(N[y][m][s], C[y][m] * sfactor);

			Obj.addTerm(P[m], dp * eh[m] * sfactor);

			for (int m_ = 0; m_ <= m; ++m_)
			{
				Obj.addTerm(R[m_][s], -em[m] * sfactor);
				Obj.add(f[m_][s] * em[m] * sfactor);
			}
		}

		Obj.addTerm(gam[s], -u);
	}
	prob->setObj(Obj);
}

void MultiLevel::genCapacityConstraints(XPRBprob* prob)
{
	for (int s = 0; s < S; ++s)
		for (int m = 0; m < M; ++m)
			for (int y = 0; y < Y; ++y)
				prob->newCtr(("Cap_" + to_string(m) + "_" + to_string(y)).c_str(), l[y] * N[y][m][s] >= P[m] * dpy[y] + R[m][s] * dry[y] + gam[s] - ep[y][m][s] * LARGE);
}

void MultiLevel::genCharterConstraints(XPRBprob* prob)
{
	for (int s = 0; s < S; ++s)
		for (int m = 0; m < M; ++m)
			for (int y = 0; y < Y; ++y)
				prob->newCtr(("Cha_" + to_string(s) + "_" + to_string(m) + "_" + to_string(y)).c_str(), A >= A * ep[y][m][s] + P[m] + R[m][s]);
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

void MultiLevel::genTopProblem(XPRBprob* prob)
{
	genTopDecisionVariables(prob);
	genTopObjective(prob);

	genCapacityConstraints(prob);
	genCharterConstraints(prob);
	genFailuresConstraints(prob);
	genPlannedConstraints(prob);
}

void MultiLevel::genLowDecisionVariables(XPRBprob* prob)
{
	for (int i = 0; i < I; ++i)
	{
		s[i] = prob->newVar(("s_" + to_string(i)).c_str(), XPRB_PL);
		s[i].setLB(0);

		for (int y = 0; y < Y; ++y)
			if (sd[y][i] + d[y][i] >= minDur[i])
				minDur[i] = sd[y][i] + d[y][i];

		s[i].setUB(T - minDur[i]);

		for (int v = 0; v < V; ++v)
			for (int j = 0; j < J; ++j)
				a[v][i][j] = prob->newVar(("a_" + to_string(v) + "_" + to_string(i) + "_" + to_string(j)).c_str(), XPRB_BV);
	}
}

void MultiLevel::genLowObjective(XPRBprob* prob)
{
	XPRBctr Obj = prob->newCtr();

	for (int i = 0; i < I; ++i)
	{
		Obj.addTerm(s[i], c[i]);
		Obj.add(c[i] * minDur[i]);
	}
	prob->setObj(Obj);
}

void MultiLevel::genSetConstraints(XPRBprob* prob)
{
	for (int v = 0; v < V; ++v)
		for (int j = 0; j < J; ++j)
		{
			XPRBrelation rel = a[v][0][j] <= 1;

			for (int i = 1; i < I; ++i)
				rel.addTerm(a[v][i][j]);

			prob->newCtr(("Set_" + to_string(v) + "_" + to_string(j)).c_str(), rel);
		}
	
	for (int v = 0; v < V; ++v)
		for (int i = 0; i < I; ++i)
		{
			XPRBrelation rel = a[v][i][0] <= 1;

			for (int j = 1; j < J; ++j)
				rel.addTerm(a[v][i][j]);

			prob->newCtr(("SetAlt_" + to_string(v) + "_" + to_string(i)).c_str(), rel);
		}
}

void MultiLevel::genOrdConstraints(XPRBprob* prob)
{
	for (int v = 0; v < V; ++v)
		for (int j = 1; j < J; ++j)
		{
			XPRBrelation rel = a[v][0][j] <= a[v][0][j-1];

			for (int i = 1; i < I; ++i)
			{
				rel.addTerm(a[v][i][j]);
				rel.addTerm(a[v][i][j-1], -1);
			}

			prob->newCtr(("Ord_" + to_string(v) + "_" + to_string(j)).c_str(), rel);
		}
}

void MultiLevel::genResourceConstraints(XPRBprob* prob)
{
	for (int y = 0; y < Y; ++y)
		for (int i = 0; i < I; ++i)
		{
			if (rho[y][i] == 0)
				continue;

			XPRBrelation rel = a[Vyo[y]][i][0] >= rho[y][i];

			for (int v = 0; v < Vy[y]; ++v)
				for (int j = 0; j < J; ++j)
					if (v > 0 || j > 0)
						rel.addTerm(a[v + Vyo[y]][i][j]);

			prob->newCtr(("Res_" + to_string(y) + "_" + to_string(i)).c_str(), rel);
		}
}

void MultiLevel::genDurationConstraints(XPRBprob* prob) 
{
	int M = 2 * T;

	for (int y = 0; y < Y; ++y)
		for (int v = 0; v < Vy[y]; ++v)
			for (int i = 0; i < I; ++i)
				for (int i_ = 0; i_ < I; ++i_)
					if (i != i_)
						for (int j = 1; j < J; ++j)
							prob->newCtr(("Dur_" + to_string(y) + "_" + to_string(v) + "_" + to_string(i) + "_" + to_string(i_) + "_" + to_string(j)).c_str(), 
								M * (a[v + Vyo[y]][i][j] + a[v + Vyo[y]][i_][j-1]) + d[y][i_] * a[v + Vyo[y]][i_][j-1] - 2 * M <= s[i] + sd[y][i] - s[i_] - sd[y][i_]);
}

void MultiLevel::genLowProblem(XPRBprob* prob) 
{
	genLowDecisionVariables(prob);
	genLowObjective(prob);

	genSetConstraints(prob);
	genOrdConstraints(prob);
	genResourceConstraints(prob);
	genDurationConstraints(prob);
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
	for (int y = 0; y < Y; ++y)
	{
		printer("Resources needed per month and scenario for Vessel Type " + to_string(y) + ": ", VERBSOL);
		for (int m = 0; m < M; ++m)
		{
			int val = round(N[y][m][0].getSol());
			printer(to_string(m) + ": " + to_string(val), VERBSOL, false);
			*file << "N_" << y << "_" << m << "_0: " << val << endl;;

			for (int s = 1; s < S; ++s)
			{
				val = round(N[y][m][s].getSol());
				printer(", " + to_string(val), VERBSOL, false);
				*file << "N_" << y << "_" << m << "_" << s << ": " << val << endl;;
			}
			printer("", VERBSOL);
		}
	}

	for (int y = 0; y < Y; ++y)
	{
		printer("Epsilon values per month and scenario for Vessel Type " + to_string(y) + ": ", VERBSOL);
		for (int m = 0; m < M; ++m)
		{
			int val = round(ep[y][m][0].getSol());
			printer(to_string(m) + ": " + to_string(val), VERBSOL, false);
			*file << "e_" << y << "_" << m << "_0: " << val << endl;;

			for (int s = 1; s < S; ++s)
			{
				val = round(ep[y][m][s].getSol());
				printer(", " + to_string(val), VERBSOL, false);
				*file << "e_" << y << "_" << m << "_" << s << ": " << val << endl;;
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
	
	printer("Repair tasks per month and scenario: ", VERBSOL);
	for (int m = 0; m < M; ++m)
	{
		int val = round(R[m][0].getSol());
		printer(to_string(m) + ": " + to_string(val), VERBSOL, false);
		*file << "R_" << m << "_0: " << val << endl;

		for (int s = 1; s < S; ++s)
		{
			int val = round(R[m][s].getSol());
			printer(", " + to_string(val), VERBSOL, false);
			*file << "R_" << m << "_" << s <<": " << val << endl;
		}
		printer("", VERBSOL);
	}
}

void MultiLevel::printGamma(ofstream* file)
{
	printer("Gamma values: ", VERBSOL);
	for (int s = 0; s < S; ++s)
	{
		int val = round(gam[s].getSol());
		printer(to_string(s) + ": " + to_string(val), VERBSOL);
		*file << "Gam_" << s << ": " << val << endl;;
	}
}

void MultiLevel::printTopProbOutput(XPRBprob* prob)
{
	/*if (prob->getProbStat() == 1)
		return;*/

	ofstream file;
	file.open(string() + OUTPUTFOLDER + name + PROBOUTPUTEXT);

	printObj(&file, prob);
	printFailures(&file);
	printResources(&file);
	printTasks(&file);
	printGamma(&file);

	file.close();
}

void MultiLevel::printStarts(ofstream* file)
{
	printer("Start times per task: ", VERBSOL);
	for (int i = 0; i < I; ++i)
	{
		double val = s[i].getSol();
		printer(to_string(i) + ": " + to_string(val), VERBSOL);
		*file << "s_" << i << "_0: " << val << endl;;
	}
}

void MultiLevel::printTaskOrders(ofstream* file)
{
	vector<vector<string>> tasks(J, vector<string>(V, "-"));

	for (int v = 0; v < V; ++v)
		for (int i = 0; i < I; ++i)
			for (int j = 0; j < J; ++j)
			{
				if (round(a[v][i][j].getSol()) == 1)
					tasks[j][v] = to_string(i);

				*file << "a_" << v << "_" << i << "_" << j << ": " << round(a[v][i][j].getSol()) << endl;;
			}

	printer("Task orders per vessel: ", VERBSOL);
	for (int j = 0; j < J; ++j)
	{		
		printer(to_string(j) + ": " + tasks[j][0], VERBSOL, false);
		for (int v = 1; v < V; ++v)
			printer(", " + tasks[j][v], VERBSOL, false);
		printer("", VERBSOL);
	}
}

void MultiLevel::printLowProbOutput(XPRBprob* prob)
{
	/*if (prob->getProbStat() == 1)
		return;*/

	ofstream file;
	file.open(string() + OUTPUTFOLDER + name + PROBOUTPUTEXT);

	printObj(&file, prob);
	printStarts(&file);
	printTaskOrders(&file);

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
