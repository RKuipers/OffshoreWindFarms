#include <iostream>
#include "DataGen.h"
#include "Weather.h"
#include "Model.h"
#include "YearModel.h"
#include "MonthModel.h"
#include "Mode.h"

#include "Solution.h"

using namespace std;

void TestYearSolution()
{
    cout << "--------------------------STARTING TestYearSolution--------------------------------" << endl;

    YearSolution s("test", 42);
    s.setResult(101, 0.5);

    vector<int> y0m0 = { 0, 1, 2 };
    vector<int> y0m1 = { 3, 4, 5 };
    vector<int> y0m2 = { 6, 7, 8 };
    vector<int> y1m0 = { 9, 10, 11 };
    vector<int> y1m1 = { 12, 13, 14 };
    vector<int> y1m2 = { 15, 16, 17 };

    vector<vector<int>> y0 = { y0m0, y0m1, y0m2 };
    vector<vector<int>> y1 = { y1m0, y1m1, y1m2 };

    s.setVessels({ y0, y1 });

    vector<int> m0 = { 18, 19, 20 };
    vector<int> m1 = { 21, 22, 23 };

    s.setPlanned({ m0, m1 });

    vector<int> m0i0 = { 24, 25, 26 };
    vector<int> m1i0 = { 27, 28, 29 };
    vector<int> m2i0 = { 30, 31, 32 };

    vector<vector<int>> m0a = { m0i0 };
    vector<vector<int>> m1a = { m1i0 };
    vector<vector<int>> m2a = { m2i0 };

    s.setReactive({ m0a, m1a, m2a });

    s.print();
}

void TestMonthSolution()
{
    cout << "--------------------------STARTING TestMonthSolution--------------------------------" << endl;

    MonthSolution s("test", 42);
    s.setResult(102, 0.5);

    vector<double> starts = {0, 1, 0.5, 7.5};

    s.setStarts(starts);

    vector<int> v0i0 = { 1, 0, 0 };
    vector<int> v0i1 = { 0, 0, 0 };
    vector<int> v0i2 = { 0, 1, 0 };
    vector<int> v0i3 = { 0, 0, 1 };
    vector<int> v1i0 = { 0, 0, 0 };
    vector<int> v1i1 = { 0, 1, 0 };
    vector<int> v1i2 = { 1, 0, 0 };
    vector<int> v1i3 = { 0, 0, 0 };

    vector<vector<int>> v0 = { v0i0 , v0i1 , v0i2 , v0i3 };
    vector<vector<int>> v1 = { v1i0 , v1i1 , v1i2 , v1i3 };

    s.setOrders({ v0, v1 });

    s.print();
}

void TestGenYearSol()
{
    cout << "--------------------------STARTING TestGenYearSol--------------------------------" << endl;

    YearData data = YearData(2, 2, 1, 2, 1);

    YearModel m(&data);
    Solution* sol = m.solve();
    sol->print();
}

void TestGenMonthSol()
{
    cout << "--------------------------STARTING TestGenMonthSol--------------------------------" << endl;

    MonthData data = MonthData(1, 1, 2, 0, 2);
    data.V = 1;
    data.I = 2;
    data.J = 2;

    MonthModel m(&data);
    Solution* sol = m.solve();
    sol->print();
}

void TestReadYear()
{
    DataGen dg = DataGen();
    ifstream datafile("Input Files/yearformat.dat");
    YearData* data = dg.readYear(&datafile);
}

void TestReadMonth()
{
    DataGen dg = DataGen();
    ifstream datafile("Input Files/monthformat.dat");
    MonthData* data = dg.readMonth(&datafile);
}

void TestReadMixed()
{
    DataGen dg = DataGen();
    ifstream datafile("Input Files/mixedformat.dat");
    MixedData* data = dg.readMixed(&datafile);
}

int main()
{
    //TestYearSolution();
    //TestMonthSolution();
    //TestGenYearSol();
    //TestGenMonthSol();
    TestReadYear();
    TestReadMonth();
    TestReadMixed();
}
