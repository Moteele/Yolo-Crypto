# Security design

This document should briefly cover all needed levels of protection, meaning **what** needs to be protected, **whom** it needs to be protected agains and **how** it can be protected.


## Levels of protection

### Protect the server
* Since this part really depends on what server we're gonna use, it's not really designable now. However it's needed to be said, that access to the server needs to be heavily protected. What comes to mind in the first place is closing all the ports (e.g. using firewall rules) except 22, 443, possibly 80.
This protection is ment to be against all the attackers trying to take control over the server for whatever reason (having a server, stealing the data...).

### Protect databases
* There are couple of ways how to understand **database protection**. First thing that comes to mind is of course some access rights policy set on the server with databases. No need to mention password protection. Second view on this thing is really high-level and it's a protection against SQL injection. This is really more of a GUI type of a problem, but we want to have it mentioned somewhere, so why not here. The last but not least problem of database protection is backup. I would suggest creating a backup of all the tables (basically the whole db) every 12 hours. Doing it more often now seems unnecessary for now. I would also suggest having at least 2 backups (one 12 hours old, second 24 hours old). Sending the backup files through some encrypted channel to another physical device also looks like a good idea.
This protection is ment to be against attackers trying to steal/damage the database (first and second view) and against data-loss (third view).

### Protect accounts
* First step of protecting accounts should be proper password hashing and not sending nor storing them in opened version. I suggest using SHA-256 as the hashing function (now it conflicts with the CHAR(128) column for password hash in database design but I don't think that's such a big deal to refactor the database design). 
Second step in protecting accounts is to have some kind of session control (e.g. insipired by google account policy). I would suggest having a list of open connections to accounts on the server. Being active on one account from more than one device could be marked down as suspicious. Being active on one account from more than two devices could be considered being an attack. All devices should be logged off and new login with password will be required. In this case I would also think about forcing the user to type in some activation code which would be sent to him to the email used for registration. This will protect the account even in case that the password got stolen/guessed/whatever.
As suggested last week, the server will periodically ask the client **"are you alive"**. I would force user logout in these two cases:
    * client is not responding whatsoever (device is off, app is turned off)
    * client is responding but the time of the last activity (sending a message, clicking through the app) is older then 30 minutes.
This protection is ment to be against attackers who are trying (or actually succeeded) to steal an account by guessing a password, stealing a device with the app etc.

### Protect messages
* This means protecting the content of messages that are being sent to the server (most likely via http requests). For this I suggest using AES-256 cipher.
This protection is ment to be agains attackers trying to catch and read messages.

### Protect the user device
* We need to protect users (and actually the server as well) against malware and similar annoying things. This is more of like a client part of protection but it's needed to be mentioned. Therefore I suggest not letting users to send any executable files, possibly archives (inspired by messenger) and maybe some other potentially dangerous stuff. Also we need to check filesizes, since sending a xy GB file through this app will lead to a disaster. The maximum allowed file size will probably depend on the used server technology.
This protection is ment to be agains attackers who are trying to use the app for spreading malware or attacking devices of other users.
