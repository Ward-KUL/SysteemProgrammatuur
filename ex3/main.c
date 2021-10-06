#include <stdio.h>
#include <string.h>
#include <ctype.h>

#define MaxNumbers 40

char first[MaxNumbers] ;
char last[MaxNumbers];
char name[MaxNumbers];
char str[MaxNumbers];
long int birthyear;


int main(){
printf("%s", "What is your first name?");
scanf("%s", first);
printf("%s", "This is the first name ");
printf("%s\n", first);
printf("%s", "What is your last name?");
scanf("%s", last);
printf("%s", "This is your last name ");
printf("%s\n", last);
int index = 0;
while(last[index] != '\0'){
	str[index] = toupper(last[index]);
	index ++;	
}
printf("All capitals: %s\n", str);

int stringsCompared =  strcmp(last,str);
printf("%s\n", "The difference between the string \n if this is 32 -> strings not the same if it is 0 strings are the same ");
printf("%d\n", stringsCompared);

strcpy(name,first);
strcat(name,last);
printf("%s\n",name);

printf("%s\n", "Plz give me year birthyear");
scanf("%ld",&birthyear);

snprintf(name,MaxNumbers*3,"%s %s %ld", first,last,birthyear);
printf("This would be the name: %s\n",name);

char first2[MaxNumbers];
char last2[MaxNumbers];
long int birthyear2;
sscanf(name,"%s %s %ld", first2, last2, &birthyear2);
printf("This would be the full name processed another way: %s %s %ld \n", first2, last2, birthyear2);

return 0;
}
