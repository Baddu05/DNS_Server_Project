
#include	"logger.h"
#include	"header.h"
#include	"message.h"
#include 	"authenticate.h"
#include	"resource.h"

int main()
{
	
	int sd = 0; // socket_descriptor
	char *domain_name = NULL;
	struct sockaddr_in nameserver;
	int status = 0;
	/* SOCKET CREATION */
	
	sd = socket(AF_INET,SOCK_DGRAM,0);

	/*Error checking for socket creation*/
	if(0 > sd)
	{
		ERROR(ERROR_MAJOR,"socket creation failed\n");
		LOGGER(LOG_CRITICAL,"Exiting function:main\n");
		return FAILURE;
	}

	

	/* CONFIGURTION OF ADDRESS FIELDS */
	
	nameserver.sin_family = AF_INET;
	nameserver.sin_port = htons(PORT);
	nameserver.sin_addr.s_addr = inet_addr(SERVER);

	TRACE(BRIEF_TRACE,("socket created\n"));
	status = Login_server(sd,&nameserver);
	if(status == SUCCESS)
	{
			
		printf("login successfull\n");
	}
	else
	{
		printf("login unsuccessfull\n");
		LOGGER(LOG_CRITICAL,"Exiting function:main\n");
		return FAILURE;
	}/*end if*/

	return 0;
}
