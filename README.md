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

Server Tasks
----------
1. Talk to Scott DNS to register domain -- `Scott`
2. Create new thread for each connection -- `Drew`
3. Log all details of transactions -- `Dan`
4. Create message buffer from client interaction -- `Alex`
5. Check to see if allowed to relay to destination -- `Rich`
6. Codes: 220, 221, 250, 251, 252, 354, 421, 447, 500 (Used by multiple)
7. Read from message buffer in dedicated thread -- `Scott`
8. Send to user if local user, or correct relay, obtained via Scott DNS -- `Dan`
9. Remove message from buffer -- `Drew`

Client Tasks
----------
1. Interface (prompt is fine) -- `Alex`
2. From, To, Server, Message -- `Alex`
3. Send data to SMTP server -- `Dan`
4. Check for error codes -- `Rich`
5. Communicate with simple and extended SMTPs (Used by multiple)
6. Display entire interaction `Rich`
7. Optional debug log -- `Drew`

Links
----------
* [Original SMTP RFC](http://tools.ietf.org/html/rfc0821 "RFC0821")
* [Latest SMTP RFC](http://tools.ietf.org/html/rfc5321 "RFC5321")