#ifndef AUXFUNCS_H
#define AUXFUNCS_H

//All strings are null terminated
int strs_are_strictly_equal(char* str1, char* str2);

int
 min(int first,int second);

int max(int first,int second);

void swap(void* first,void* second,int sz_bytes);

void print_addr_aux(char* prompt,struct sockaddr_in* addr);


#endif
