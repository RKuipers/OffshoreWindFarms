#include "Model.h"

Model::Model(InputData* data) : data(data) { }

Solution* Model::solve()
{
	p.mipOptimise();

	return genSolution(&p);
}

void Model::genProblem()
{
	// TODO: Add your implementation code here.
}


void Model::genDecVars()
{
	// TODO: Add your implementation code here.
}


void Model::genObj()
{
	// TODO: Add your implementation code here.
}
