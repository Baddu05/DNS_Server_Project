
typedef struct
{
	char domain[256];
	unsigned short int type;
	unsigned short int _class;
	unsigned int ttl;
	unsigned short datalen;
	char ip[30];
}Resource;


