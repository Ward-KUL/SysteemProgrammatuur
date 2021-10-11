 #include <stdio.h>
#include <stdlib.h>

void swap_pointers(int* a, int* b)
{
printf("%s\n", "Swap pointers called");

int* temp = a;
a = b;
b = temp;

printf("Waarde's waar dees naar terug wijst: *p = %p en *q = %p\n",a,b);
printf("Result after swapping pointers,p = %p en q = %p",a,b);
return;
}

int main(void){
int a = 1;
int b = 2;
// for testing we use pointers to integers
int *p = &a;
int *q = &b;
printf("address of p = %p and q = %p\n", &a, &b);
// prints p = &a and q = &b
swap_pointers( p , q );
printf("address of p = %p and q = %p\n", &a, &b);
return 0;
}
