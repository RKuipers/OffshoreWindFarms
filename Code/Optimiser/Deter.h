#pragma once

#include "Optimiser.h"

class Deter : public Optimiser
{
private:
	Mode initMode();

	void readData();

	void genDecisionVariables(XPRBprob* prob);
	void genObjective(XPRBprob* prob);
	void genPartialProblem(XPRBprob* prob, Mode* m); 
	void genFullProblem(XPRBprob* prob, Mode* m);

	void printProbOutput(XPRBprob* prob, Mode* m, int id);
	void printModeOutput(Mode* m, bool opt);
};
