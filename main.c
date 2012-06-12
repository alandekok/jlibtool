#include <stdlib.h>
#include <stdio.h>

extern int example_function(const char *s);


int main(int argc, char **argv)
{
	example_function(__FUNCTION__);
	printf("\n");
	return 0;
}
