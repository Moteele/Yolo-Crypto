# Documentation
This file describes the inner workings of the program

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
repeating string senOTP = 6;
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
repeating string senOTP = 6
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
repeating string senOTP = 6
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
