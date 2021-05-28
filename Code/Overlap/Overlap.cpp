#include <iostream>
#include <ctime>
#include <filesystem>

#include "DataGen.h"
#include "Weather.h"
#include "Model.h"
#include "YearModel.h"
#include "MonthModel.h"
#include "Mode.h"
#include "Solution.h"

#define YEAR
//#define MONTH
//#define MIXED
#define DEFAULTPATH "yearDinwoodieInstall.dat"

using namespace std;

void runYear(string fullName)
{
    string name = fullName;
    name.replace(fullName.size() - 4, string::npos, "");
    Mode mode = Mode();
    DataGen dg = DataGen();
    ifstream datafile("Input Files/" + fullName);
    YearData* data = dg.readYear(&datafile);
    YearModel* model = new YearModel(data, &mode, name);
    model->genProblem();
    YearSolution* sol = model->solve();
    sol->print();
}

void runMultipleYears(string path)
{
    for (const auto& entry : filesystem::directory_iterator("Input Files/" + path))
    {
        string pathString = entry.path().string();
        runYear(pathString.substr(pathString.find("\\") + 1));
        // TODO: Add parameters to stop every run from printing everything and to make them write a combined CSV
    }
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
    ifstream datafile("Input Files/mixedDinwoodie.dat");
    MixedData* data = dg.readMixed(&datafile);

    int maxTime = 300;
    int infeasible = 1;
    while (infeasible > 0)
    {
        infeasible = 0;

        cout << "-------------- YEAR --------------" << endl;
        YearModel* yearModel = new YearModel(data, &mode);
        yearModel->genProblem();
        YearSolution* yearSol = yearModel->solve();
        yearSol->print();

        vector<MonthData> months = dg.genMonths2(data, yearSol);
        vector<MonthSolution*> monthSols = vector<MonthSolution*>(months.size(), nullptr);
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

            MonthModel* monthModel = new MonthModel(&months[m], &mode, "Month", m);
            //monthModel->genProblem();
            monthModel->genPartialProblem(0);
            monthSols[m] = monthModel->solve(maxTime);
            monthModel->genPartialProblem(1);
            monthSols[m] = monthModel->solve(maxTime);

            if (monthSols[m] == nullptr)
            {
                cout << "Month " << m << " deemed infeasible, running Feedback model" << endl;

                vector<double> ep;
                vector<int> rho;
                monthModel->getRequirements(&ep, &rho, data->Y);
                //data->eps[0][m] = ep; // TODO: 0 is for scenario; needs to be fixed
                //data->rho[0][m] = rho;
                infeasible++;
            }
            else
                monthSols[m]->print();
        }
        
        cout << endl;

        if (infeasible != 0)
        {
            cout << "Redoing year since " << infeasible << " months are infeasible" << endl;
            maxTime *= 2;
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

    string path = "";
    switch (run)
    {
    case 1:
#if defined(DEFAULTPATH)
        path = DEFAULTPATH;
#else
        cout << "Which input file(s) should be used?" << endl;
        cin >> path;
#endif // !DEFAULTPATH
        if (path.substr(path.size() - 4).compare(".dat") == 0)
            runYear(path);
        else
            runMultipleYears(path);
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
