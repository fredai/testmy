#include <stdio.h>
#include <stdlib.h>

main()
{
	char * endptr;
	float value = 0; 
	char * str="23.6";

	value = strtof ( str, NULL );
	printf("%f\n", value);
}

