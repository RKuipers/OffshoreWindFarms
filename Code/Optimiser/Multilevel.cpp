#include "Multilevel.h"

MultiLevel::MultiLevel()
{
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

	p.setSense(XPRB_MAXIM);
	p.mipOptimize("");

	printProbOutput(&p);
}

void MultiLevel::getData()
{

}

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
}

void MultiLevel::genFailuresConstraints(XPRBprob* prob)
{
}

void MultiLevel::genPlannedConstraints(XPRBprob* prob)
{
}

void MultiLevel::genProblem(XPRBprob* prob)
{
	genDecisionVariables(prob);
	genObjective(prob);

	genCapacityConstraints(prob);
	genFailuresConstraints(prob);
	genPlannedConstraints(prob);
}
