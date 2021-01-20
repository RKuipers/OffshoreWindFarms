#include <iostream>
#include <ctime>
#include "DataGen.h"
#include "Weather.h"
#include "Model.h"
#include "YearModel.h"
#include "MonthModel.h"
#include "Mode.h"

#include "Solution.h"

//#define YEAR
//#define MONTH
//#define MIXED

using namespace std;

void runYear()
{
    Mode mode = Mode();
    DataGen dg = DataGen();
    ifstream datafile("Input Files/yearBasic.dat");
    YearData* data = dg.readYear(&datafile);
    YearModel* model = new YearModel(data, &mode);
    model->genProblem();
    YearSolution* sol = model->solve();
    sol->print();
}

void runMonth()
{
    Mode mode = Mode();
    DataGen dg = DataGen();
    ifstream datafile("Input Files/monthBasic.dat");
    MonthData* data = dg.readMonth(&datafile);
    MonthModel* model = new MonthModel(data, &mode);
    model->genProblem();
    MonthSolution* sol = model->solve();
    sol->print();
}

void runMixed()
{
    clock_t start = clock();

    Mode mode = Mode();
    DataGen dg = DataGen();
    ifstream datafile("Input Files/mixedBasic.dat");
    MixedData* data = dg.readMixed(&datafile);

    bool feasible = false;
    int infeasible = 0;
    while (!feasible)
    {
        feasible = true;
        infeasible = 0;

        cout << "-------------- YEAR --------------" << endl;
        YearModel* yearModel = new YearModel(data, &mode);
        yearModel->genProblem();
        YearSolution* yearSol = yearModel->solve();
        yearSol->print();

        vector<MonthData> months = dg.genMonths(data, yearSol);
        for (int m = 0; m < months.size(); ++m)
        {
            cout << endl;

            if (months[m].I == 0)
            {
                cout << "Month " << m << " has no tasks to schedule" << endl;
                continue;
            }
            else
                cout << "-------------- MONTH " << m << " --------------" << endl;

            MonthModel* monthModel = new MonthModel(&months[m], &mode);
            monthModel->genProblem();
            MonthSolution* sol = monthModel->solve();
            if (sol == nullptr)
            {
                vector<double> ep;
                vector<int> rho;
                monthModel->getRequirements(&ep, &rho);
                data->eps[0][m] = ep; // TODO: 0 is for scenario; needs to be fixed
                data->rho[0][m] = rho;
                feasible = false;
                infeasible++;
            }
            else
                sol->print();
        }
    }

    cout << "TOTAL duration: " << ((double)clock() - start) / (double)CLOCKS_PER_SEC << endl;
}

int main()
{
    int run = 0;
#if defined(YEAR)
    run = 1;
#elif defined(MONTH)
    run = 2;
#elif defined(MIXED)
    run = 3;
#else
    cout << "Which Model do you want to run?" << endl << "Year (1), Month (2), Mixed (3)" << endl;
    cin >> run;
#endif

    switch (run)
    {
    case 1:
        runYear();
        break;
    case 2:
        runMonth();
        break;
    case 3:
        runMixed();
        break;
    default:
        cout << "ERROR" << endl;
    }
}
