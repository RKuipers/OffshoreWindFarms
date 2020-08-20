#include <string>		// string, to_string
#include <iostream>		// cout
#include "Deterministic.h"
#include "Stochastic.h"

using namespace std;

//#define TYPE 1		// Sto
//#define TYPE 0		// Opt

int main(int argc, char** argv)
{
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
		Deter::run();
	else if (type == 1)
		Stoc::run();
}