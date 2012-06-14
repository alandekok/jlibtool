#include <stdlib.h>
#include <stdio.h>

extern int example_function(const char *s);


int main(int argc, char **argv)
{
	printf("We are ");

	if (argc == 1) {
		example_function(__FUNCTION__);
	} else {
		example_function(argv[1]);
	}

	printf("\n");
	return 0;
}
