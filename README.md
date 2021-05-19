# humidity_server

This humidity server resides in the same local area network as the sensor nodes. It accepts incoming connections from the sensor nodes, receives the data via TCP and rudimentarily checks the received data for consistency. Afterwards it connects to a different remote server with a MySQL database. It then inserts the data into that database for permanent storage and analysis. 

As it is possible that several sensor nodes try to establish a connection at the same time, the humidity server forks with each incoming connection. Unfortunately, sometimes a connection is not properly closed by the sensor node, leaving stale child processes. This is somewhat taken care of with calling waitpid(). 

## Build and Install ## 
Before building, you need to adjust the variables server, user, password and database in database.c, lines 14ff. These credentials refer to the MySQL server directly, meaning that the specified user and password need to exist within MySQL (not as a linux system user). Once that is done and the packages mysql-client and libmysqlclient-dev are installed, you should be able to simply call make to build the humidity server. 
