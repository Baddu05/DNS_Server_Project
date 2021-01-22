#include	"logger.h"
#include	"header.h"
#include	"message.h"
#include 	"authenticate.h"
#include	"resource.h"

int main()
{
	
	int sd = 0; // socket_descriptor
	char domain_name[HOST_SIZE] = "www.hellohi.com";
	struct sockaddr_in nameserver;

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

	Dns_Query(sd,&nameserver,domain_name,IP);
	return 0;
}
