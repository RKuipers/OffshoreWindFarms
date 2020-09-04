#include "Multilevel.h"

// ----------------------------Constructor function----------------------------

MultiLevel::MultiLevel()
{
	name = "Multilevel";

	V = 2;
	M = 12;

	C = vector<vector<double>>(V, vector<double>(M));
	eh = vector<double>(M);
	em = vector<double>(M);
	dpv = vector<int>(V);
	drv = vector<int>(V);
	f = vector<int>(M);
	l = vector<int>(V);
	P = vector<XPRBvar>(M);
	R = vector<XPRBvar>(M);
	N = vector<vector<XPRBvar>>(V, vector<XPRBvar>(M));
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

void MultiLevel::getData()
{
	l = { 720, 480 };
	A = 100;
	f = { 0, 1, 4, 2, 1, 6, 8, 2, 0, 4, 3, 2 };
	dp = 12;
	dpv = {4, 12};
	drv = {8, 16};
	eh = { 540, 540, 540, 540, 540, 540, 540, 540, 540, 540, 540, 540 };
	em = { 388800, 388800, 388800, 388800, 388800, 388800, 388800, 388800, 388800, 388800, 388800, 388800 };
	C = { { 500000, 500000, 500000, 500000, 500000, 500000, 500000, 500000, 500000, 500000, 500000, 500000 }, 
		{ 120000, 120000, 120000, 120000, 120000, 120000, 120000, 120000, 120000, 120000, 120000, 120000 } };
}

// ----------------------------Generator functions-----------------------------

void MultiLevel::genDecisionVariables(XPRBprob* prob)
{
	for (int m = 0; m < M; ++m)
	{
		P[m] = prob->newVar(("P_" + to_string(m)).c_str(), XPRB_UI);
		R[m] = prob->newVar(("R_" + to_string(m)).c_str(), XPRB_UI);
		P[m].setLB(0);
		R[m].setLB(0);

		for (int v = 0; v < V; ++v)
		{
			N[v][m] = prob->newVar(("N_" + to_string(v) + "_" + to_string(m)).c_str(), XPRB_UI);
			N[v][m].setLB(0);
		}
	}
}

void MultiLevel::genObjective(XPRBprob* prob)
{
	XPRBctr Obj = prob->newCtr();
	for (int m = 0; m < M; ++m)
	{
		for (int v = 0; v < V; ++v)
			Obj.addTerm(N[v][m], C[v][m]);

		Obj.addTerm(P[m], dp * eh[m]);

		for (int m_ = 0; m_ < m; ++m_)
		{
			Obj.addTerm(R[m_], -em[m]);
			Obj.add(f[m_] * em[m]);
		}
	}
	prob->setObj(Obj);
}

void MultiLevel::genCapacityConstraints(XPRBprob* prob)
{
	for (int m = 0; m < M; ++m)
		for (int v = 0; v < V; ++v)
		{
			prob->newCtr(("Cap_" + to_string(m) + "_" + to_string(v)).c_str(), l[v] * N[v][m] >= P[m] * dpv[v] + R[m] * drv[v]);
		}

}

void MultiLevel::genFailuresConstraints(XPRBprob* prob)
{
	int total = 0;

	for (int m = 0; m < M; ++m)
	{
		if (m > 0)
			total += f[m-1];

		XPRBrelation rel = total - R[0] >= 0;

		for (int m_ = 1; m_ <= m; ++m_)
			rel.addTerm(R[m_], -1);

		prob->newCtr(("Fai_" + to_string(m)).c_str(), rel);
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

void MultiLevel::printResources(ofstream* file)
{
	printer("Resources needed per month and vessel type: ", VERBSOL);
	for (int m = 0; m < M; ++m)
	{
		int val = round(N[0][m].getSol());
		printer(to_string(m) + ": " + to_string(val), VERBSOL, false);
		*file << "N_0_" << m << ": " << val << endl;;

		for (int v = 1; v < V; ++v)
		{
			val = round(N[v][m].getSol());
			printer(", " + to_string(val), VERBSOL, false);
			*file << "N_" << v << "_" << m << ": " << val << endl;;
		}
		printer("", VERBSOL);
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

	printer("Repair tasks per month: ", VERBSOL);
	for (int m = 0; m < M; ++m)
	{
		int val = round(R[m].getSol());
		printer(to_string(m) + ": " + to_string(val), VERBSOL);
		*file << "R_" << m << ": " << val << endl;;
	}
}

void MultiLevel::printProbOutput(XPRBprob* prob)
{
	/*if (prob->getProbStat() == 1)
		return;*/

	ofstream file;
	file.open(string() + OUTPUTFOLDER + name + PROBOUTPUTEXT);

	printObj(&file, prob);
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
