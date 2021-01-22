/******************************************************************************
*  FILE NAME    : logger.c
*
*  DESCRIPTION	: Function definitions for logger
*
*		COPYRIGHTS ARICENT TECHNOLOGY HOLDINGS
*******************************************************************************/

/***************************HEADER FILES INCLUDED******************************/
#include "logger.h"

/********************************************************************
*      			Global variables
********************************************************************/
int g_error_level = ERROR_MAJOR;
    char err_file[FILE_SIZE];
    int err_no;
    int err_level = 4;
    long err_max = 10000L;


static char time_string[LOG_SIZE];

    int err_no;
    int log_level = LOG_LEVEL;
    long log_max = LOG_MAX;

/******************************************************************************
*
* 	FUNCTION NAME: logger
*
*	DESCRIPTION  : Global function for debugging information
*
* 	 RETURNS     : Returns void
*******************************************************************************/

void logger( int level, char *log_msg )
{
 strcpy(log_file,"logfile.txt");

 FILE *logfile;
 char buffer[BUFF_SIZE];
 int attempts = ZERO;


 if (log_level >= level) /* Don't do debug messages with a lower priority */
 {                         /* than the program's debugging level.           */

	get_now_time();
  	(void)snprintf(buffer, (size_t)LOG_SIZE,"\n%s|%d--> ", time_string, getpid());

  	for (attempts = 0; attempts < LOOP; attempts++)
  	{
   		if ( '\0' != log_file[0] )
   		{
			if ((logfile = fopen(log_file, "a")) != NULL)
    			{
     				fprintf(logfile, "%s %s\n", buffer, log_msg);
     				(void)fflush(logfile);
     				(void)fclose(logfile);

     				attempts = 5;
    			}
    			else
     				(void)usleep((__useconds_t)100000L); /* Wait, someone writing now */
   		}
   		else
    			/* failure, try to print it where someone can find it */
    			/* This too may fail..                                */

    			(void)fprintf(stderr, "%s\n", buffer);
  	}/*end of for*/
  	roll_over_log();
  }/*end of if*/

} /*end of function*/


/******************************************************************************
*
* FUNCTION NAME: get_now_time
*
* DESCRIPTION: Gets the current system time
*
* RETURNS: Returns string contains current time
*******************************************************************************/

void get_now_time( void )
{

        struct timeb tb;
        struct tm *tmp;


        (void)ftime(&tb);
        tmp = localtime(&tb.time);
	if( NULL != tmp )
        	(void)strftime(time_string, sizeof(time_string), "%h %d %H:%M:%S", tmp);
        (void)snprintf(&time_string[strlen(time_string)], (size_t)LOG_SIZE,".%03u", tb.millitm);


}/*end of function*/


/******************************************************************************
*
* FUNCTION NAME: roll_over_log
*
* DESCRIPTION: Checks for the log file
*
* RETURNS: Returns void
*******************************************************************************/

void roll_over_log( void )
{

        struct stat file_stat;
        char backup_file[LOG_SIZE];


        /* A 0 limit size means don't limit file */
        if ( 0 == log_max ) return;

        /* A NULL file name. */
        if ( '\0' == log_file[0] )     return;

        /* Oops. This is bad. No file. */
        if ( -1 == stat(log_file, &file_stat) )       return;

        /* File still smaller than limit */
        if (file_stat.st_size < log_max)      return;

        (void)snprintf(backup_file,(size_t)LOG_SIZE ,"%s.bak", log_file);

        /* remove the old backup if it exists */
        (void)unlink(backup_file);

        /* chmod ugo+rwx the new log file */

	(void)rename(log_file, backup_file);


}/*end of function*/


/********************************************************************
*
* FUNCTION NAME: ang_error
*
* DESCRIPTION: Prints the error message along with the error number
*
* RETURNS: Returns void
*********************************************************************/
void ang_error(int err_level,char *msg)
{

    strcpy(err_file,"errorfile.txt");
    FILE *errfile;
    char buffer[BUFF_SIZE];
    int attempts = 0;

    /* compare the err_level parameter against the global flag to see
       whether the input error level is suppressed or not. If not then
       the input error message is printed */

    if(err_level <= g_error_level)
    {
      	 get_now_time();
        
  	 (void)snprintf(buffer, (size_t)BUFF_SIZE,"\n%s|%d|--> ", time_string, getpid());
         printf("Error : (%s)\n", msg);
         for (attempts = 0; attempts < LOOP; attempts++)
  	 {
   		if ( '\0' != err_file[0] ) 
   		{
			if ((errfile = fopen(err_file, "a")) != NULL)
    			{
   				fprintf(errfile, "%s %s\n", buffer, msg);
     				(void)fflush(errfile);
     				(void)fclose(errfile);

     				attempts = 21;
    			}
    			else
     				(void)usleep((__useconds_t)100000L); /* Wait, someone writing now */
   		}
  		else
    			/* failure, try to print it where someone can find it */
    			/* This too may fail..                                */

  			(void)fprintf(stderr, "%s\n", buffer);
  	}/*end of for*/
  	roll_over_err();
  }/*end of if*/

}
    

/******************************************************************************
*
* FUNCTION NAME: roll_over_log
*
* DESCRIPTION: Checks for the log file
*
* RETURNS: Returns void
*******************************************************************************/

void roll_over_err( void )
{

        struct stat file_stat;
        char backup_file[FILE_SIZE];


        /* A 0 limit size means don't limit file */
        if ( 0 == err_max ) return;

        /* A NULL file name. */
        if ( '\0' ==  err_file[0] )     return;

        /* Oops. This is bad. No file. */
        if ( -1 == stat(err_file, &file_stat) )       return;

        /* File still smaller than limit */
        if (file_stat.st_size < err_max)      return;

        (void)snprintf(backup_file,(size_t)LOG_SIZE ,"%s.bak", err_file);

        /* remove the old backup if it exists */
        (void)unlink(backup_file);

        /* chmod ugo+rwx the new log file */

	(void)rename(err_file, backup_file);

}/*end of function*/



