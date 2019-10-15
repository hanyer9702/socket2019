#include <stdio.h>

int main()
{
	int retval = system("ls > list.txt");
	printf("%d\n", retval);
	return 0;
}
