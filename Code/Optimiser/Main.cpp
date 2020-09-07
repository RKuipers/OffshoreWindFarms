#include <string>		// string, to_string
#include <iostream>		// cout
#include "Optimiser.h"
#include "Deterministic.h"
#include "Stochastic.h"
#include "Multilevel.h"

using namespace std;

#define TYPE 2		// Multilevel
//#define TYPE 1		// Stochastic
//#define TYPE 0		// Deterministic

int main(int argc, char** argv)
{
	srand(SEED);

	int type;

#ifdef TYPE
	type = TYPE;		
#endif // TYPE

#ifndef TYPE
	cout << "Which program do you want to run?" << endl;
	cout << "Type D for Deterministic, S for Stochastic, M for Multilevel" << endl;

	string inp;
	cin >> inp;
	while (inp[0] != 'D' && inp[0] != 'S' && inp[0] != 'M')
	{
		cout << "Invalid input, try again please" << endl;
		cin >> inp;
	}

	if (inp[0] == 'D')
		type = 0;
	else if (inp[0] == 'S')
		type = 1;
	else
		type = 2;
#endif // !TYPE

	if (type == 0)
	{
		cout << "Running Deterministic" << endl;
		Deter d = Deter();
		d.Run();
	}
	else if (type == 1)
	{
		cout << "Running Stochastic" << endl;
		Stoch s = Stoch();
		s.Run();
	}
	else if (type == 2)
	{
		cout << "Running Multilevel" << endl;
		MultiLevel m = MultiLevel();
		m.Run();
	}
}