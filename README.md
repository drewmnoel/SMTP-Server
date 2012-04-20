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
1. Talk to Scott DNS to register domain
2. Create new thread for each connection
3. Log all details of transactions
4. Create message buffer from client interaction
5. Check to see if allowed to relay to destination
6. Codes: 220, 221, 250, 251, 252, 354, 421, 447, 500
7. Read from message buffer in dedicated thread
8. Send to user if local user, or correct relay, obtained via Scott DNS
9. Remove message from buffer

Client Tasks
----------
1. Interface (prompt is fine)
2. From, To, Server, Message
3. Send data to SMTP server
4. Check for error codes
5. Communicate with simple and extended SMTPs
6. Display entire interaction
7. Optional debug log

Links
----------
* [Original SMTP RFC](http://tools.ietf.org/html/rfc0821 "RFC0821")
* [Latest SMTP RFC](http://tools.ietf.org/html/rfc5321 "RFC5321")