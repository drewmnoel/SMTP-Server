SMTP Server
==========
This project involves basic SMTP implementation on both the client and server levels.

Authors:
----------
* Richard Couillard
* Scott Fenwick
* Alexander Leary
* Daniel Mercado
* Drew Noel

Pre-programming Tasks
----------
* Structure and outline process

Server Tasks
----------
* Create new thread for each connection
* Log all details of transactions
* Create message buffer from client interaction
* Check to see if allowed to relay to destination
* Codes: 220, 221, 250, 251, 252, 354, 421, 447, 500
* Read from message buffer in dedicated thread
* Send to user if local user
* Send to next if not local user (acting as a client)
* Remove message from buffer
* File mutex between reading and writing

Client Tasks
----------
* Interface (prompt is fine)
* From, To, Server, Message
* Send data to SMTP server
* Check for error codes
* Communicate with simple and extended SMTPs
* Display entire interaction
* Optional debug log