
/**********************************************************************************************************************************************
*                       FILE NAME       :  dns_server.c
*
*                       DESCRIPTION     :  server handling  multiple client  
*
*       DATE            NAME                    REFERENCE       REASON
*    		                    				 Project
*
*       copyright @2015,Aricent Technologies(Holdings) Ltd.,
**********************************************************************************************************************************************/

/**********************************************************************************************************************************************
*
*                                HEADER FILES
*
***********************************************************************************************************************************************/
#include "header_server.h"
#include "logger.h"


/**********************************************************************************************************************************************
*
*                                GLOBAL VARIABLES
*
**********************************************************************************************************************************************/

int count = 10; //number of clients served concurrently


/***********************************************************************************************************************************************
*
* 	Function name: main
*
* 	Description: creates socket and recieves message from client and  serves client
*
*	Return: SUCCESS OR FAILURE
*
***********************************************************************************************************************************************/


int main(
	 int C, 	/* argument count*/ 
	 char *V[] 	/* comman line arguments*/
	 )
{
	LOGGER(LOG_CRITICAL,"Start of function:main, for server\n");
	
	int	sd	= 0,
		recvd	= 0,
		th_ret	= 0;

	char	buf[MAX];

	struct sockaddr_storage client_addr;
	struct sockaddr_in server_addr;

	socklen_t structlength = 0;
		
	Thread_client *client = NULL;

	pthread_t thread;


	/* Create socket */
	if ((sd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) 
	{
		ERROR(ERROR_CRITICAL, "Socket could not be created...\n");
		LOGGER(LOG_CRITICAL,"Exiting function: main, for server\n");
		exit(FAILURE);
	}
	/* Prepare the sockaddr_in structure */

	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = INADDR_ANY;
	server_addr.sin_port = htons(PORT1);

	/* bind the socket to the port number */
	if (bind(sd, (struct sockaddr *) &server_addr, sizeof(server_addr)) == -1) 
	{
		ERROR(ERROR_CRITICAL, "Error in binding...\n");
		close(sd);
		LOGGER(LOG_CRITICAL, "Exiting funtion: main, for server\n");

		exit(FAILURE);
	}

	structlength = sizeof(struct sockaddr_storage);
	
	/* Serving concurrently 10 clients */

	while(0 < count)
	{
		signal( SIGINT, my_handler );

		printf("In server now..  \n ");	
		printf("To exit from Server Press CTRL+C \n ");
		
		LOGGER(LOG_MINOR,"Server wait for client\n");
		
		/* Recieve message from client */
		
		recvd = recvfrom(sd, (char *)buf, sizeof(buf), 0,(struct sockaddr *) &client_addr, &structlength);	
		if(0 > recvd)
		{	
			ERROR(ERROR_CRITICAL, "Error in recieving message from client...\n");
			close(sd);
			LOGGER(LOG_CRITICAL, "Exiting funtion: main, for server\n");
			exit(FAILURE);
		}
		
		if (0 == strcmp(buf,"login"))
		{
			if (SUCCESS == Authenticate(sd,&client_addr))
			{
				TRACE(DETAILED_TRACE,("login successful\n"));
				memset(buf,0,MAX);
				recvd = recvfrom(sd, (char *)buf, sizeof(buf), 0,(struct sockaddr *)&client_addr, &structlength);
				if(0 > recvd)
				{	
					ERROR(ERROR_CRITICAL, "Error in recieving message from client...\n");
					close(sd);
					LOGGER(LOG_CRITICAL, "Exiting funtion: main, for server\n");
					exit(FAILURE);
				}
				

				/* Create structure Thread_client */

				client = (Thread_client *)calloc(1 ,sizeof(Thread_client));
				if(NULL == client)
				{
					ERROR(ERROR_CRITICAL, "Memory Allocation failure...\n");
					close(sd);
					LOGGER(LOG_CRITICAL, "Exiting function: main, for server\n");
					exit(FAILURE);
				}
				
				/* Assigning Thread_client structure */

				client->sd = sd;
				client->cliaddr = (struct sockaddr_storage)client_addr;
				memcpy((char *)client->buffer,(char *)buf,sizeof(buf));
					
				/* create new thread to serve client */

				th_ret = pthread_create(&thread,NULL,(void *)Update_Record,(void *)client);
				if(0 > th_ret)
				{
					ERROR(ERROR_CRITICAL, "Could not create thread...\n");
					close(sd);
					LOGGER(LOG_CRITICAL, "Exiting function: main, for server\n");
					exit(FAILURE);

				}	
			}
		}
		else
		{
			/* Create structure Thread_client */

			client = (Thread_client *)calloc(1 ,sizeof(Thread_client));
			if(NULL == client)
			{
				ERROR(ERROR_CRITICAL, "Memory Allocation failure...\n");
				close(sd);
				LOGGER(LOG_CRITICAL, "Exiting function: main, for server\n");
				exit(FAILURE);
			}
			
			/* Assigning Thread_client structure */

			client->sd = sd;
			client->cliaddr = (struct sockaddr_storage)client_addr;
			memcpy((char *)client->buffer,(char *)buf,sizeof(buf));
			
			
			/* create new thread to serve client */

			th_ret = pthread_create(&thread,NULL,(void *)serve_client,(void *)client);
			if(0 > th_ret)
			{
				ERROR(ERROR_CRITICAL, "Could not create thread...\n");
				close(sd);
				LOGGER(LOG_CRITICAL, "Exiting function: main, for server\n");
				exit(FAILURE);
			}
		}
	}//end of while

	LOGGER(LOG_CRITICAL,"Exiting funtion: main.for server\n");
	pthread_exit(NULL);	
	//return 0;
} // main

/*******************************END OF MAIN***********************************/








