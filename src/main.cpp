#include <stdio.h>

#include "rtimulib_examples.hpp"

void hello(const char* member)
{
	printf("hello %s!\n", member);
}

int main(void)
{
	hello("someone");

	run_rtimulib_example1();

	return 0;
}
