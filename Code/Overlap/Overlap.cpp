#include <iostream>
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
    cout << "-------------- YEAR --------------" << endl;
    Mode mode = Mode();
    DataGen dg = DataGen();
    ifstream datafile("Input Files/mixedBasic.dat");
    MixedData* data = dg.readMixed(&datafile);
    YearModel* yearModel = new YearModel(data, &mode);
    yearModel->genProblem();
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
        monthModel->genProblem();
        MonthSolution* sol = monthModel->solve();
        sol->print();
    }
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
