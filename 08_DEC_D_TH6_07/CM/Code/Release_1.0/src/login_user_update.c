#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include "authenticate.h"

int main()
{
	FILE *fp;
	fp = fopen("../bin/login","a");
	login a;
	strcpy(a.user, "Deepesh");
	strcpy(a.password,"12345");
	a.response = 0;

	fwrite(&a,sizeof(login),1,fp);
	strcpy(a.user, "Divya");
	strcpy(a.password,"12345");
	a.response = 0;

	fwrite(&a,sizeof(login),1,fp);
	strcpy(a.user, "Shivani");
	strcpy(a.password,"12345");
	a.response = 0;

	fwrite(&a,sizeof(login),1,fp);
	strcpy(a.user, "Sourav");
	strcpy(a.password,"12345");
	a.response = 0;

	fwrite(&a,sizeof(login),1,fp);
	strcpy(a.user, "Sonia");
	strcpy(a.password,"12345");
	a.response = 0;

	fwrite(&a,sizeof(login),1,fp);
	return 0;
}
