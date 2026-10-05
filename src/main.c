#include <stdio.h>

void hello(const char* member)
{
	printf("hello %s!\n", member);
}

int main(void)
{
	hello("someone");
	return 0;
}
