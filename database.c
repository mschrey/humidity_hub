

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include </usr/include/mysql/mysql.h>   //install packages mysql-client and libmysqlclient-dev
//#include "relHumidityStructs.h"
#include "database.h"



// MySQL variables
char *server 	= "<IP_OF_MYSQL_SERVER>";
char *user 		= "<MYSQL_USERNAME>";
char *password	= "<MYSQL_PASSWORD>";
char *database 	= "<DATABASE_NAME>";



void getCurrentTime(char *outbuffer)
{
	time_t timer;
	struct tm* tm_info;
	time(&timer);
	tm_info = localtime(&timer);
	strftime(outbuffer, 26, "%Y:%m:%d %H:%M:%S", tm_info);
}


int buildQueryInsert(char *query, struct datagram *mydg) 
{
    char datestring[80];
    time_t seconds = time(NULL);
    struct tm *timeinfo = localtime(&seconds);
    strftime(datestring, 80, "%F %T", timeinfo);
    
	sprintf(query, "INSERT INTO `RawData`(\
	`datetime`,\
	`timestamp` ,\
	`id` ,\
	`humidity` ,\
	`temperature`,\
	`voltage`) VALUES (\
	'%d', \
	'%s', \
	'%d', \
	'%f', \
	'%f', \
	'%d');", 
    (int)seconds,
    datestring,
	mydg->idnr,
	mydg->relHumidity,
	mydg->temperature,
	mydg->battVoltage);
	return 0;
}


MYSQL *  mysql_connect()
{
	// Connect to MySQL server
	MYSQL *conn_data = mysql_init(NULL);
	if(!mysql_real_connect(conn_data, server, user, password, database, 0, NULL, 0))
	{
		fprintf(stderr, "%s\n", mysql_error(conn_data));
		//exit(1);
	}
    return conn_data;
}


void mysql_execute_insertQuery(MYSQL *conn_data, char *query)
{
	if(mysql_query(conn_data, query))
	{
		char *timebuf = malloc(30*sizeof(char));
		getCurrentTime(timebuf);
		fprintf(stderr, "[%s] Error: %s\nQuery: %s\n", timebuf, query, mysql_error(conn_data));
		free(timebuf);
	} 
	//int fieldcount = mysql_field_count(conn_data);
	//printf("Number of columns affected: %d\n", fieldcount);
	//printf("Error: %s\n", mysql_error(conn_data));
}


//usage
//    char *query = (char*)malloc(2048*sizeof(char));	
//    MYSQL *mysql_conn = mysql_connect();
//    buildQueryInsert(query, mydg);
//    mysql_execute_insertQuery(mysql_conn, query);
//

