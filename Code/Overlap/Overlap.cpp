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
#define MIXED

using namespace std;

void runYear()
{
    Mode mode = Mode();
    DataGen dg = DataGen();
    ifstream datafile("Input Files/yearScen.dat");
    YearData* data = dg.readYear(&datafile);
    YearModel* model = new YearModel(data, &mode);
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
    MonthSolution* sol = model->solve();
    sol->print();
}

void runMixed()
{
    clock_t start = clock();

    cout << "-------------- YEAR --------------" << endl;
    Mode mode = Mode();
    DataGen dg = DataGen();
    ifstream datafile("Input Files/mixedInfea.dat");
    MixedData* data = dg.readMixed(&datafile);
    YearModel* yearModel = new YearModel(data, &mode);
    YearSolution* yearSol = yearModel->solve();
    yearSol->print();

    vector<MonthData> months = dg.genMonths(data, yearSol);
    for (int m = 0; m < months.size(); ++m)
    {
        if (months[m].I == 0)
        {
            cout << "Month " << m << " has no tasks to schedule" << endl;
            continue;
        }
        else
            cout << "-------------- MONTH " << m << " --------------" << endl;

        MonthModel* monthModel = new MonthModel(&months[m], &mode);
        MonthSolution* sol = monthModel->solve();
        if (sol == nullptr)
        {
            vector<double> ep;
            vector<int> rho;
            monthModel->getRequirements(&ep, &rho);
            data->eps[0][m] = ep; // TODO: 0 is for scenario; needs to be fixed
            data->rho[0][m] = rho; 
        }
        else
            sol->print();
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
