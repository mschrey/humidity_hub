//taken from https://gist.github.com/browny/5211329
//multi-client forking taken from https://stackoverflow.com/questions/13669474/multiclient-server-using-fork

// Additional info for MYSQL stuff:
//
// this program is supposed to connect to a _remote_ MYSQL database. 
// The ubuntu packages 'mysql-client' and 'libmysqlclient-dev' must be installed.
// This has security implications:
//  * I checked with tcpdump and wireshark that the MYSQL password does not
//    get transferred unencrypted
//  * This program (aka MYSQL client) has a special MYSQL user 'humdata' 
//    assigned to it - together with special, reduced privileges. 
//   See e.g. https://stackoverflow.com/questions/1559955/host-xxx-xx-xxx-xxx-is-not-allowed-to-connect-to-this-mysql-server
//   Privileges are: 
//     - User may INSERT and SELECT only.
//     - User may execute max 300 queries per hour, max 300 updates per hour, max 300 connections per hour, max 5 user_connections 
//       (following the rationale: <= 1 update per sensor per minute; with 5 sensors this results in <= 300 updates per hour


//when executing ./humserver binary, do
// $ su humiditydata
// $ script /dev/null
// $ screen
// $ ./humserver
// as written in https://makandracards.com/makandra/2533-solve-screen-error-cannot-open-your-terminal-dev-pts-0-please-check


/* --- server.c --- */
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include "database.h"



int main(int argc, char *argv[])
{
	int listenfd = 0, connfd = 0, n=0, error=0, counter=0, pid, waitret1, waitret2;
	struct sockaddr_in serv_addr, client_addr;

	char sendBuff[1025];
    char recvBuff[1025];

	char cpyBuff[1025];
	char cpyBuff2[1025];
	char * key;
	char * value;
	char * pch;
	char * state1;
	char * state2;
	struct datagram myDatagram;
	char *query = (char*)malloc(2048*sizeof(char));
	char clientName[INET_ADDRSTRLEN];
    MYSQL *mysql_conn;
        
	/* creates an UN-named socket inside the kernel and returns
	 * an integer known as socket descriptor
	 * This function takes domain/family as its first argument.
	 * For Internet family of IPv4 addresses we use AF_INET
	 */
	listenfd = socket(AF_INET, SOCK_STREAM, 0);
	memset(&serv_addr, '0', sizeof(serv_addr));
	memset(sendBuff, '0', sizeof(sendBuff));

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_addr.sin_port = htons(8034);

	/* The call to the function "bind()" assigns the details specified
	 * in the structure 『serv_addr' to the socket created in the step above
	 */
	bind(listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr));

	/* The call to the function "listen()" with second argument as 10 specifies
	 * maximum number of client connections that server will queue for this listening
	 * socket.
	 */
	listen(listenfd, 10);

	while(1)
	{
		/* In the call to accept(), the server is put to sleep and when for an incoming
		 * client request, the three way TCP handshake* is complete, the function accept()
		 * wakes up and returns the socket descriptor representing the client socket.
		 */
		printf("PID=%d: waiting for new connection...\n", getpid());
		int client_len = sizeof(client_addr);
		connfd = accept(listenfd, (struct sockaddr*)&client_addr, (socklen_t*) &client_len);
		
		if ((pid = fork()) == -1) {  //child creation failed
		    printf("Error! Child creation failed!\n");
		    close(connfd);           
		    continue;
		} else if (pid > 0) {        //this is the parent process
		    close(connfd);           //drop connection handle, child owns this now
		    counter++;
		    printf("parent here, counter=%d\n", counter);
		    waitret1 = waitpid(-1, NULL, WNOHANG);   //try to reap any remaining <defunct> process stubs
		    waitret2 = waitpid(-1, NULL, WNOHANG);   //we do this twice, because we don't always execute this parent part
		    printf("Calling wait() returned %d and %d\n", waitret1, waitret2);   //as often as the child part
		    continue;
		} else if (pid == 0) {       //this is the child process
		    
		    inet_ntop(AF_INET, &client_addr.sin_addr.s_addr, clientName, sizeof(clientName));
		    printf("PID=%d: new connection accepted, IP: %s\n", getpid(), clientName);
		    /* As soon as server gets a request from client, it prepares the date and time and
		     * writes on the client socket through the descriptor returned by accept()
		     */
	    
		    //receive some data
		    n = read(connfd, recvBuff, sizeof(recvBuff)-1);
		    printf("received %d bytes!\n", n);
		    recvBuff[n] = '\0';
		    printf("datagram:'%s'\n", recvBuff);

		    strcpy(cpyBuff, recvBuff);
		    strcpy(cpyBuff2, recvBuff);
		    
		    //splitting into tokens		
		    pch = strtok_r(cpyBuff, ";", &state1);  //use strtok_r to enable nested calls 
		    while(pch != NULL) {
		        strcpy(cpyBuff2, pch);
		        key = strtok_r(cpyBuff2, "=", &state2);
		        if(key != NULL) {
		            value = strtok_r(NULL, "=", &state2);
                    //printf("%s -> %s\n", key, value);
                    if (strcmp(key, "system") == 0) {
                        if(strcmp(value, "remoteHumiditySensor") == 0) {
                            printf("correct datagram received!\n");
                        } else {
                            printf("illegal datagram received!\n");
                            error += 1;
                        }		                
                    } else if (strcmp(key, "idnr") == 0) {		      
       		            myDatagram.idnr = atoi(value);
        		    } else if (strcmp(key, "temperature") == 0) {
        	            myDatagram.temperature = atof(value);
                    } else if (strcmp(key, "humidity") == 0) {
                        myDatagram.relHumidity = atof(value);
                    } else if (strcmp(key, "voltage") == 0) {
                        myDatagram.battVoltage = atof(value);
                    } else {
        	            printf("illegal key found!\n");
        	            error += 1;
                    }
		            pch = strtok_r(NULL, ";", &state1);
	            }		
	        }
			    
            
            if(error == 0) {
                strcpy(sendBuff, "ACK ");
                printf("sending ACK\n");
                write(connfd, sendBuff, strlen(sendBuff));
                
           		printf("ID: %d\n", myDatagram.idnr);
        		printf("Temperature: %.2f°C\n", myDatagram.temperature);
        		printf("RelativeHumidity: %.2f%%\n", myDatagram.relHumidity);
        		
                //prepare insertion of data into MYSQL table
                mysql_conn = mysql_connect();
                printf("connected to mysql db\n");
                buildQueryInsert(query, &myDatagram);
                printf("sql query is: '%s'\n", query);
                mysql_execute_insertQuery(mysql_conn, query);        
            	mysql_close(mysql_conn);        		
            } else {
                strcpy(sendBuff, "NACK");
                printf("sending NACK\n");
           		write(connfd, sendBuff, strlen(sendBuff));
            }

            printf("\n");
		    close(connfd);
		    break; //exit child's while loop
		}
	}
	close(listenfd);	
}

