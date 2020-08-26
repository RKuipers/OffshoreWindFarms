#include <string>		// string, to_string
#include <iostream>		// cout
#include "Optimiser.h"
#include "Deter.h"
#include "Stochastic.h"

using namespace std;

//#define TYPE 1		// Stochastic
#define TYPE 0		// Deterministic

int main(int argc, char** argv)
{
	srand(SEED);

	int type;

#ifdef TYPE
	type = TYPE;		
#endif // TYPE

#ifndef TYPE
	cout << "Which program do you want to run?" << endl;
	cout << "Type D for Deterministic, S for Stochastic" << endl;

	string inp;
	cin >> inp;
	while (inp[0] != 'D' && inp[0] != 'S')
	{
		cout << "Invalid input, try again please" << endl;
		cin >> inp;
	}

	if (inp[0] == 'D')
		type = 0;
	else
		type = 1;
#endif // !TYPE

	if (type == 0)
	{
		cout << "Running Deterministic" << endl;
		Deter d = Deter();
		d.Run(MAXPRETIME, MAXFULLTIME);
	}
	else if (type == 1)
	{
		cout << "Running Stochastic" << endl;
		S::run();
	}
}