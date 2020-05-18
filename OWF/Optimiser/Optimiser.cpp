//TODO: readData()

#include <tuple>		// tuple
#include <iostream>		// cout
#include <cmath>		// pow
#include <string>		// string, to_string
#include "xprb_cpp.h"

using namespace std;
using namespace ::dashoptimization;

#define DATAFILE "install.dat"
#define NPERIODS 5
#define TPP 4 // Timesteps per Period
#define NTIMES NPERIODS * TPP
#define NTASKS 4
#define NIP 2
#define NRES 2
#define NASSETS 2
#define DIS 1.0

int OMEGA[NTASKS][NTIMES] = { { 1, 1, 1, 1, 0, 0, 1, 1, 1, 1, 1, 1, 0, 1, 0, 1, 1, 0, 1, 1 }, { 1, 1, 1, 1, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1 }, { 1, 1, 1, 1, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1 }, { 1, 1, 1, 1, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1 } }; //TODO: Give right number, set up test case
int v[NPERIODS] = { 75, 8, 5, 10, 25 };
int C[NRES][NPERIODS] = { { 20, 20, 30, 20, 15 }, { 15, 15, 5, 10, 5} };
int d[NTASKS] = { 2, 4, 3, 1 };
int rho[NTASKS][NRES] = { {0, 1}, {3, 2}, {1, 0}, {1, 1} };
int m[NRES][NPERIODS] = { {5, 5, 5, 5, 5}, {5, 5, 5, 5, 5} };
tuple<int, int> IP[NIP] = { make_tuple(1, 2), make_tuple(2, 3) };

XPRBprob prob("Install1"); // Initialize a new problem in BCL

/*void readData(void)
{
	double value;
	int s;
	FILE* datafile;
	char name[100];
	//SHARES = p.newIndexSet("Shares", NSHARES); // Create the ‘SHARES’ index set
	// Read ‘RET’ data from file
	datafile = fopen(DATAFILE, "r");
	for (s = 0; s < NSHARES; s++)
	{
		XPRBreadlinecb(XPRB_FGETS, datafile, 200, "T g", name, &value);
		RET[SHARES += name] = value;
	}
	fclose(datafile);
	SHARES.print(); // Print out the set contents
}*/

int main(int argc, char** argv)
{
	int a, p, r, i, x, t, t1, t2, t3;

	XPRBexpr Obj;
	XPRBexpr TaskStart[NASSETS][NTASKS];
	XPRBexpr TaskFinish[NASSETS][NTASKS];
	XPRBexpr Prec[NASSETS][NIP][NTIMES][NTIMES];
	XPRBexpr Weat[NASSETS][NTASKS][NTIMES][NTIMES];
	XPRBexpr Res[NRES][NPERIODS][TPP];
	XPRBexpr Onl[NPERIODS];
	XPRBexpr Cos[NPERIODS];

	XPRBvar O[NPERIODS];
	XPRBvar N[NRES][NPERIODS];
	XPRBvar s[NASSETS][NTASKS][NTIMES];
	XPRBvar f[NASSETS][NTASKS][NTIMES];

	// Read data from file
	//readData();

	//---------------------------Decision Variables---------------------------

	// Create the period-based decision variables
	for (p = 0; p < NPERIODS; ++p)
	{
		O[p] = prob.newVar(("O_" + to_string(p)).c_str(), XPRB_UI);
		O[p].setLB(0);

		for (r = 0; r < NRES; ++r)
		{
			N[r][p] = prob.newVar(("N_" + to_string(r) + " " + to_string(p)).c_str(), XPRB_UI);
			N[r][p].setLB(0);
			N[r][p].setUB(m[r][p]);
		}
	}

	// Create the timestep-based decision variables
	for (a = 0; a < NASSETS; ++a)
		for (i = 0; i < NTASKS; ++i)
			for (t = 0; t < NTIMES; ++t)
			{
				s[a][i][t] = prob.newVar(("s_" + to_string(a) + "_" + to_string(i) + "_" + to_string(t)).c_str(), XPRB_BV);
				f[a][i][t] = prob.newVar(("f_" + to_string(a) + "_" + to_string(i) + "_" + to_string(t)).c_str(), XPRB_BV);
			}

	//--------------------------------Objective--------------------------------

	for (p = 0; p < NPERIODS; ++p)
	{
		for (r = 0; r < NRES; ++r)
			Cos[p] += N[r][p] * C[r][p];

		Obj += pow(DIS, p) * (O[p] * v[p] - Cos[p]);
	}
	prob.setObj(Obj); // Set the objective function

	//-------------------------------Constraints-------------------------------

	// Forces every task to start and end
	for (a = 0; a < NASSETS; ++a)
		for (i = 0; i < NTASKS; ++i)
		{
			for (t = 0; t < NTIMES; ++t)
			{
				TaskStart[a][i] += s[a][i][t];
				TaskFinish[a][i] += f[a][i][t];
			}
			prob.newCtr(("Sta_" + to_string(a) + "_" + to_string(i)).c_str(), TaskStart[a][i] == 1);
			prob.newCtr(("Fin_" + to_string(a) + "_" + to_string(i)).c_str(), TaskFinish[a][i] == 1);
		}

	// Precedence constraints
	for (a = 0; a < NASSETS; ++a)
		for (x = 0; x < NIP; ++x)
			for (t1 = 0; t1 < NTIMES; ++t1)
				for (t2 = t1; t2 < NTIMES; ++t2)
				{
					int j;
					tie(i, j) = IP[x];

					Prec[a][x][t1][t2] = s[a][j][t1] + f[a][i][t2];
					prob.newCtr(("Prec_" + to_string(a) + "_" + to_string(x) + "_" + to_string(t1) + "_" + to_string(t2)).c_str(), Prec[a][x][t1][t2] <= 1);
				}

	// Weather conditions
	for (a = 0; a < NASSETS; ++a)
		for (i = 0; i < NTASKS; ++i)
			for (t1 = 0; t1 < NTIMES; ++t1)
				for (t2 = 0; t2 < NTIMES; ++t2)
				{
					int weather = 0;
					for (t3 = t1; t3 <= t2; ++t3)
						weather += OMEGA[i][t3];

					Weat[a][i][t1][t2] += (f[a][i][t2] + s[a][i][t1]) * 0.5 * weather - d[i] * (s[a][i][t1] + f[a][i][t2] - 1);

					prob.newCtr(("Weat_" + to_string(a) + "_" + to_string(i) + "_" + to_string(t1) + "_" + to_string(t2)).c_str(), Weat[a][i][t1][t2] >= 0);
				}

	// Resource amount link
	for (r = 0; r < NRES; ++r)
		for (p = 0; p < NPERIODS; ++p)
			for (t = p * TPP; t < (p + 1) * TPP; ++t)
			{
				for (a = 0; a < NASSETS; a++)
					for (i = 0; i < NTASKS; ++i)
					{
						Res[r][p][t - p * TPP] += rho[i][r] * s[a][i][0];
						for (t1 = 1; t1 <= t; ++t1)
							Res[r][p][t - p * TPP] += rho[i][r] * (s[a][i][t1] - f[a][i][t1 - 1]);
					}

				prob.newCtr(("Res_" + to_string(r) + "_" + to_string(p) + "_" + to_string(t)).c_str(), Res[r][p][t - p * TPP] <= N[r][p]);
			}

	// Online turbines link
	for (p = 0; p < NPERIODS; ++p)
	{
		for (t = 0; t < p * TPP; ++t)
			for (a = 0; a < NASSETS; a++)
				Onl[p] += f[a][NTASKS - 1][t];

		prob.newCtr(("Onl_" + to_string(p)).c_str(), Onl[p] == O[p]);
	}

	//---------------------------------Solving---------------------------------
	//p.setMsgLevel(1);
	prob.setSense(XPRB_MAXIM);
	prob.exportProb(XPRB_LP, "Install1");
	prob.mipOptimize("");

	const char* MIPSTATUS[] = { "not loaded", "not optimized", "LP optimized", "unfinished (no solution)", "unfinished (solution found)", "infeasible", "optimal", "unbounded" };
	cout << "Problem status: " << MIPSTATUS[prob.getMIPStat()] << endl;

	//----------------------------Solution printing----------------------------
	cout << "Total return: " << prob.getObjVal() << endl;

	cout << "Online turbines per period: " << endl;
	for (p = 0; p < NPERIODS; ++p)
		cout << p << ": " << O[p].getSol() << endl;

	cout << "Resources needed per period and type: " << endl;
	for (p = 0; p < NPERIODS; ++p)
	{
		cout << p << ": " << N[0][p].getSol();

		for (r = 1; r < NRES; ++r)
			cout << ", " << N[r][p].getSol();
		cout << endl;
	}

	cout << "Start and finish time per asset and task: " << endl;
	for (a = 0; a < NASSETS; ++a)
	{
		cout << "Asset: " << a << endl;
		for (i = 0; i < NTASKS; ++i)
		{
			int start = -1;
			int finish = -1;
			for (t = 0; t < NTIMES; ++t)
			{
				if (s[a][i][t].getSol() == 1)
					start = t;
				if (f[a][i][t].getSol() == 1)
					finish = t;
			}
			cout << i << ": " << start << " " << finish << endl;
		}
	}

	return 0;
}
