#include <iostream>

#include "rtimulib_examples.hpp"

using std::cout;
using std::flush;

void hello(const char* member)
{
	cout << "hello" << member << "!\n" << flush;
}

int main()
{
	hello("someone");

	run_rtimulib_example1();
}
