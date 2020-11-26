#include "YearModel.h"

YearData* YearModel::getData()
{
	return data->getYear();
}

YearSolution* YearModel::genSolution(XPRBprob* p, double duration)
{
	solution = new YearSolution(mode->getCurrentName(), mode->getCurrentId());
	solution->setResult(p->getObjVal(), duration);

	vector<vector<vector<int>>> N(getData()->Y, vector<vector<int>>(getData()->M, vector<int>()));

	for (int y = 0; y < getData()->Y; ++y)
		for (int m = 0; m < getData()->M; ++m)
			for (int sig = 0; sig < getData()->S; ++sig)
				N[y][m].push_back(round(this->N[y][m][sig].getSol()));

	solution->setVessels(N);

	vector<vector<int>> P(getData()->M, vector<int>());

	for (int m = 0; m < getData()->M; ++m)
		for (int i = 0; i < getData()->Ip; ++i)
			P[m].push_back(round(this->P[m][i].getSol()));

	solution->setPlanned(P);
	
	vector<vector<vector<int>>> R(getData()->M, vector<vector<int>>(getData()->Ir, vector<int>()));

	for (int m = 0; m < getData()->M; ++m)
		for (int i = 0; i < getData()->Ir; ++i)
			for (int sig = 0; sig < getData()->S; ++sig)
				R[m][i].push_back(round(this->R[m][i][sig].getSol()));

	solution->setReactive(R);

	return solution;
}

void YearModel::genProblem()
{
	genDecVars();
	genObj();

	genCapacityCon;
	genRepairCon();
	genMaxMaintCon();
	genMinMaintCon();
	genAvailableCon();
}

void YearModel::genDecVars()
{
	for (int m = 0; m < getData()->M; ++m)
	{
		for (int sig = 0; sig < getData()->S; ++sig)
		{
			for (int y = 0; y < getData()->Y; ++y)
				N[y][m][sig] = p.newVar(("N_" + to_string(y) + "_" + to_string(m) + to_string(sig)).c_str(), XPRB_UI);

			for (int ir = 0; ir < getData()->Ir; ++ir)
				R[m][ir][sig] = p.newVar(("R_" + to_string(m) + "_" + to_string(ir) + to_string(sig)).c_str(), XPRB_UI);
		}

		for (int ip = 0; ip < getData()->Ip; ++ip)
			P[m][ip] = p.newVar(("P_" + to_string(m) + "_" + to_string(ip)).c_str(), XPRB_UI);
	}
}

void YearModel::genObj()
{
	double sFac = 1.0 / getData()->S; 
	XPRBctr Obj = p.newCtr();

	for (int sig = 0; sig < getData()->S; ++sig)
		for (int m = 0; m < getData()->M; ++m)
		{
			for (int y = 0; y < getData()->Y; ++y)
				Obj.addTerm(N[y][m][sig], getData()->c[y][m] * sFac);

			for (int ip = 0; ip < getData()->Ip; ++ip)
				Obj.addTerm(P[m][ip], getData()->dP * getData()->eH[m] * sFac);

			for (int ir = 0; ir < getData()->Ir; ++ir)
				for (int m_ = 0; m_ < m; ++m_)
				{
					Obj.addTerm(R[m_][ir][sig], -1 * getData()->eH[m] * getData()->H[m] * sFac);
					Obj.add(getData()->f[m_][ir][sig] * getData()->eH[m] * getData()->H[m] * sFac);
				}
		}
}

void YearModel::genCapacityCon()
{

}

void YearModel::genRepairCon()
{
}

void YearModel::genMaxMaintCon()
{
}

void YearModel::genMinMaintCon()
{
}

void YearModel::genAvailableCon()
{
}

YearModel::YearModel(YearData* data) : Model(data)
{
	// TODO: Remove this test code
	vector<XPRBvar> N00;
	N00.push_back(p.newVar("n000", XPRB_UI));
	N00.push_back(p.newVar("n001", XPRB_UI));
	vector<XPRBvar> N01;
	N01.push_back(p.newVar("n010", XPRB_UI));
	N01.push_back(p.newVar("n011", XPRB_UI));
	N = vector<vector<vector<XPRBvar>>>(1, { N00, N01 });

	P.push_back({ p.newVar("p00", XPRB_UI, 0), p.newVar("p01", XPRB_UI, 0) } );
	P.push_back({ p.newVar("p10", XPRB_UI, 0), p.newVar("p11", XPRB_UI, 0) } );

	R.push_back({ { p.newVar("r000", XPRB_UI, 0), p.newVar("r001", XPRB_UI, 0) } });
	R.push_back({ { p.newVar("r100", XPRB_UI, 0), p.newVar("r101", XPRB_UI, 0) } });

	p.setObj(N[0][0][0] + N[0][0][1] + N[0][1][0] + N[0][1][1]);
	p.setSense(XPRB_MINIM);

	p.newCtr(2 * N[0][0][0] >= P[0][0] + 2 * P[0][1] + 3 * R[0][0][0]);
	p.newCtr(2 * N[0][0][1] >= P[0][0] + 2 * P[0][1] + 3 * R[0][0][1]);
	p.newCtr(2 * N[0][1][0] >= P[1][0] + 2 * P[1][1] + 3 * R[1][0][0]);
	p.newCtr(2 * N[0][1][1] >= P[1][0] + 2 * P[1][1] + 3 * R[1][0][1]);
	p.newCtr(P[0][0] + P[1][0] >= 4);
	p.newCtr(P[0][1] + P[1][1] >= 2);
	p.newCtr(R[0][0][0] >= 1);
	p.newCtr(R[1][0][1] >= 1);
}

YearSolution* YearModel::solve()
{
	// TODO: Remove this test code

	YearSolution* sol = new YearSolution("test", 0);

	sol->setVessels({ {vector<vector<int>>(24, {2})},{vector<vector<int>>(24, {1})} });
	sol->setPlanned(vector<vector<int>>(24, {5, 3, 1}));
	sol->setReactive(vector<vector<vector<int>>>(24, { {{8}},{{6}},{{4}} }));

	return sol;
}
