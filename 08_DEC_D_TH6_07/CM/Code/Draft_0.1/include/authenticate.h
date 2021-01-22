
#ifndef _AUTHENTICATE_H
#define _AUTHENTICATE_H

typedef struct
{
	char user[10];
	char password[10];
	unsigned char response:1;
}login;

#endif
