# humidity_server

This humidity server resides in the same local area network as the sensor nodes. It accepts incoming connections from the sensor nodes, receives the data via TCP and rudimentarily checks the received data for consistency. Afterwards it connects to a different remote server with a MySQL database. It then inserts the data into that database for permanent storage and analysis. 

As it is possible that several sensor nodes try to establish a connection at the same time, the humidity server forks with each incoming connection. Unfortunately, sometimes a connection is not properly closed by the sensor node, leaving stale child processes. This is somewhat taken care of with calling waitpid(). 
