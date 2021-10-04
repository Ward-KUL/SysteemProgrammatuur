#include <stdio.h>
int main()
{
	printf("%s\n", "The size of char");
    printf("%lu\n",  sizeof(char));
	printf("%s\n", "Size of int");
    printf("%lu\n", sizeof(int));
	printf("%s\n", "Size of float");
    printf("%lu\n", sizeof(float));
	printf("%s\n", "Size of double");
    printf("%lu\n",sizeof(double));
	printf("%s\n","Size of void");
	printf("%lu\n", sizeof(void));
	printf("%s\n","Size of pointer");
	printf("%lu\n",sizeof(NULL));
	
	//Size of long and short qualifiers
 
	printf("%s\n", "Short int");
	printf("%lu\n", sizeof(short int));
	printf("%s\n", "Long int");
	printf("%lu\n", sizeof(long int));
	
	//Check if char is signed or unsigned
	if(sizeof(char) == sizeof(signed char)){
		printf("%s\n", "The char is signed");
	}
	else{
	printf("%s\n", "The char is usigned");
	}
	

   return 0;


}
