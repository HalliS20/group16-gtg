#include <iostream>

#include "rtimulib_examples.hpp"
#include "ledmatrixdriver_examples.hpp"

using std::cout;
using std::flush;

void hello(const char* member)
{
	cout << "hello" << member << "!\n" << flush;
}

int main()
{
	hello("someone");

	run_ledmatrixdriver_example1();
	run_rtimulib_example1();
}
