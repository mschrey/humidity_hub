#ifndef DATABASE
#define DATABASE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include </usr/include/mysql/mysql.h>   //install packages mysql-client and libmysqlclient-dev



struct datagram {
    float temperature;
    float relHumidity;
    int idnr;
    int battVoltage;
};



void getCurrentTime(char *outbuffer);

int buildQueryInsert(char *query, struct datagram *mydg);

MYSQL *  mysql_connect();

void mysql_execute_insertQuery(MYSQL *conn_data, char *query);

#endif
