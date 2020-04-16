# Documentation

## Table of contents
* [Security design](./documentation.md#security-design)
* [Messages](./documentation.md#messages)
* [Administrative api](./documentation.md#administrative-api)
* [Build](./documentation.md#build)

## Security design

This document should briefly cover all needed levels of protection, meaning **what** needs to be protected, **whom** it needs to be protected agains and **how** it can be protected.


### potential attackers
1) server misusing users data or spoofing messages
2) server attacking users devices
3) users attacking other users ( e.g. sending malicious files)
4) users attacking server
5) attacker misuising the client to attack other services
6) MitM reading messages
7) MitM spoofing messages
8) other services attacking the client ( e. g. keyboard or OS )
9) DDoS attack on the server

### what we might need to protect
* users personal data ( e.g. identity, location etc. )
* users contacts
* users online status
* message contents
* time and recipient/sender of a given message
* access to users OS
* private keys and other secrets
* conversation history

### Levels of protection

#### Protect the server
Since this part really depends on what server we're gonna use, it's not really designable now. However it's needed to be said, that access to the server needs to be heavily protected. What comes to mind in the first place is closing all the ports (e.g. using firewall rules) except 22, 443, possibly 80.\
This protection is ment to be against all the attackers trying to take control over the server for whatever reason (having a server, stealing the data...).\
We should also limit the amount of data a user can send per unit of time to prevent DDoS from the
side of user. User registration should require some CAPTCHA measure.

#### Protect databases
There are couple of ways how to understand **database protection**. First thing that comes to mind is of course some access rights policy set on the server with databases. No need to mention password protection. Second view on this thing is really high-level and it's a protection against SQL injection. This is really more of a GUI type of a problem, but we want to have it mentioned somewhere, so why not here. The last but not least problem of database protection is backup. I would suggest creating a backup of all the tables (basically the whole db) every 12 hours. Doing it more often now seems unnecessary for now. I would also suggest having at least 2 backups (one 12 hours old, second 24 hours old). Sending the backup files through some encrypted channel to another physical device also looks like a good idea.\
This protection is ment to be against attackers trying to steal/damage the database (first and second view) and against data-loss (third view).

#### Protect accounts
First step of protecting accounts should be proper password hashing and not sending nor storing them in opened version. I suggest using SHA-256 as the hashing function.\
Second step in protecting accounts is to have some kind of session control (e.g. insipired by google account policy). I would suggest having a list of open connections to accounts on the server. Being active on one account from more than one device could be marked down as suspicious. Being active on one account from more than two devices could be considered being an attack. All devices should be logged off and new login with password will be required. In this case I would also think about forcing the user to type in some activation code which would be sent to him to the email used for registration. This will protect the account even in case that the password got stolen/guessed/whatever.\
As suggested last week, the server will periodically ask the client **"are you alive"**. I would force user logout in these two cases:
* client is not responding whatsoever (device is off, app is turned off)
* client is responding but the time of the last activity (sending a message, clicking through the app) is older then 30 minutes.
This protection is ment to be against attackers who are trying (or actually succeeded) to steal an account by guessing a password, stealing a device with the app etc.

#### Protect messages
Here mostly comes to play the actual Signal protocol. The authentication and sharing a secret is
  based on [Extended Triple Diffie-Hellman](https://www.signal.org/docs/specifications/x3dh/). This
  phase requires storing a bunch of data on the server, thus connection can be established qithout
  all parties online. the choice of X3DH parameters is not that important. We will have a bunch of
  private keys to store on each users device. The protocol doesn't specify how that should be done,
  so we will go with PBKDF2 for key derivation from users passphrases and AES256 to encrypt the private
  keys. The passphrase is a string, but we might also enable the use of fingerprint or other
  biometric.\
  The confidentiality and integrity of messages is ensured by [The Double Ratchet
  Algorithm](https://www.signal.org/docs/specifications/doubleratchet/). We should follow the
  recommended cryptographics algorithms specified in section 5.2 of the documentation. That is
  SHA512 and AES256.\
  For now, we do not intend to implement some of the more advanced features of the protocol like
  Sesame or header encryption.

#### Protect the user device
We need to protect users (and actually the server as well) against malware and similar annoying things. This is more of like a client part of protection but it's needed to be mentioned. Therefore I suggest not letting users to send any executable files, possibly archives (inspired by messenger) and maybe some other potentially dangerous stuff. Also we need to check filesizes, since sending a xy GB file through this app will lead to a disaster. The maximum allowed file size will probably depend on the used server technology.\
This protection is ment to be agains attackers who are trying to use the app for spreading malware or attacking devices of other users.

## Messages
The application uses [Protocol Buffers](https://developers.google.com/protocol-buffers) for serializing messages between client and server.
The structure of the message is defined in a .proto file, which can then be compiled into *.cpp* and *.hpp* files.
They contain the defined classes from .proto file with methods for creation, manipulation, serialization and deserialization of messages.
All messages have the same structure:

* Type of message
* All used attributes

The type is an enum with all possible types:
```
enum MsgType {
	PUBLISH_KEYS = 0;
	FETCH_KEYS_REQUEST = 1;
	FETCH_KEYS_RESPONSE = 2;
	INIT_MESSAGE = 3;
	MESSAGE = 4;
	...
}
```

Each type has needed values filled in the body of the message.

### Publish keys
```
required MsgType type = 1;
optional int32 senId = 2;
optional string senIdKey = 3;
optional string senPreKey = 4;
optional string senPreKeySig = 5;
repeated string senOTP = 6;
```
* type - type of the message
* senId - id of the sender of the message
* senIdKey - sender identity key
* senPreKey - sender signed prekey
* senPreKeySig -  sender prekey signature
* senOTP - list of one-time prekeys


### Fetch keys request
```
required MsgType type = 1;
optional int32 senId = 2;
optional string senIdKey = 3;
...
repeated string senOTP = 6
optional int32 reqId = 7;
```
* type - type of the message
* senId - id of the sender of the message
* senIdKey ... senOTP - unused
* reqId - id of the requested user


### Fetch keys response
```
required MsgType type = 1;
optional int32 senId = 2;
...
repeated string senOTP = 6
optional int32 reqId = 7;
optional string reqIdKey = 8;
optional string reqPreKey = 9;
optional string reqPreKeySig = 10;
optional string reqOTP = 11;
```
* type - type of the message
* senId ... senOTP - unused
* reqId - id of the requested user
* reqIdKey - requested identity key IKB
* reqPreKey - requested signed prekey
* reqPreKeySig - requested prekey signature
* reqOTP - requested one-time prekey (optional)


### Init message
```
required MsgType type = 1;
optional int32 senId = 2;
optional string senIdKey = 3;
optional string senPreKey = 4;
...
optional string reqOTP = 11;
optional string senEpKey = 12;
optional int32 OTPid = 13;
optional string CT = 14;
```
* type - type of the message
* senId - id of the sender of the message
* senIdKey - sender identity key
* senPreKey ... reqOTP - unused
* senEpKey - sender ephermal key
* OTPid - identifier of which OTP was used
* CT - ciphertext containing asociated data AD and encrypted using the shared secret

### Message
```
required MsgType type = 1;
optional int32 senId = 2;
optional string senIdKey = 3;
...
optional string CT = 14;
optional int32 messNum = 15;
optional int32 prevNum = 16;
optional string senRKey = 17;
optional int32 recId = 18;
optional string textContent = 19;
optional string timestamp = 20;
```
* type - type of the message
* senId - id of the sender of the message
* senIdKey ... CT - unused
* messNum - message number in the sending chain
* prevNum - length (number of message keys) in the previous sending chain
* senRKey - sender's current ratchet public key
* recId - id of the reciever of the message
* textContent - the actual content of the message, probably limited to some length. In future will probably be a struct, so we support different message types (like response to some message), attachements etc. see this for inspiration.
* timestamp - time of sending

### Using the .proto file
* compile the .proto file to source files
```
$ protoc -I=$SOURCE_DIR --cpp_out=$DESTINATION_DIR $SOURCE_DIR/some_protofile.proto
```
* rename the *some_protofile.pb.cc* to *some_protofile.pb.cpp* and *some_protofile.pb.h* to *some_protofile.pb.hpp*
```
$ mv some_protofile.pb.cc some_protofile.pb.cpp
$ mv some_protofile.pb.h some_protofile.pb.hpp
```

* create an instance of the message
```
Message msgInstance;
```

* set all needed parameters
```
msgInstance.set_senId(currentUser.id);
msgInstance.set_recId(reciever.id);
...
```

* serialize the message to string
```
std::string msgSerialized;
msgInstance.SerializeToString(&msgSerialized);
```

Now we have a string containing a serialized message that we can send to server which can deserialize it and read its parameters
```
Message msgIntance;
msgInstance.ParseFromString(&stringRecievedFromClient);
std::cout << msgInstance.senId();
```

The same way as the server can the client deserialize incoming messages as well.

### Sending the messages
Messages will be send to server via HTTP requests, encrypted.


### Storing the messages
Messages will be stored on the server in the table **CONVERSATION**.

## Administrative api
* token ... random number or random string of fixed length
* challenge ... random number
* hash() ... hashing function, SHA-512
* asymmetric crypto ... curve25519

### Registration
```
Server                                          Client
        <-------------------------------email-
        -token to email---------------------->  (if email is not taken)
        -challenge--------------------------->
        <-------------hash(challenge + token)-
        -one time pubkey--------------------->
        <---------------------------challenge-
        -encrypted challenge----------------->
        <-----------hash encypted with pubkey-
        -ack--------------------------------->
```

### Authentication
```
Server                                          Client
        <-------------------------------email-
        -challenge--------------------------->
        <---------hash(hash(pwd) + challenge)-
        -ack--------------------------------->
```

### Change password
* user is authenticated
```
Server                                          Client
	<-------------password change request-
        -one time pubkey--------------------->
        <---------------------------challenge-
        -encrypted challenge----------------->
        <-----------hash encypted with pubkey-
        -ack--------------------------------->
```

### Password reset
* same as registration

### Delete account
```
Server                                          Client
        <-------------------------------email-
        -token to email---------------------->
        -challenge--------------------------->
        <-hash(hash(pwd) + challenge + token)-
        -ack--------------------------------->
```
### adding contact
```
Server                                          Client
	<------------------account_identifier- client_1
        --ack--------------------------------> client_1
        --request----------------------------> client_2
        <-------------------------accept/deny- client_2
        -(if accept):ack---------------------> client_1
```

### Find user
```
Server                                          Client
        <-------------------------------email-
        -user id----------------------------->
```

## Build
Dependencies:

* [GNU Make](https://www.gnu.org/software/make/)
* [Google Protocol buffers](https://developers.google.com/protocol-buffers)
* [pkg-config](https://www.freedesktop.org/wiki/Software/pkg-config/)

### Compilation
* clone the repository:
```
$ git clone https://github.com/Moteele/Yolo-Crypto.git
```

* navigate to the base directory:
```
$ cd Yolo-Crypto
```

* build desired target with make:
```
$ make all		// builds the client and server program
$ make test		// builds and runs tests
$ make clean		// removes object files
$ make debug		// prints used CXXFLAGS and LDFLAGS
```
