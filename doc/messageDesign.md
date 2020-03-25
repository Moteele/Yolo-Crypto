# Message design

This document should contain all information needed for implementing messages.

## Message format

### Protocol buffers

As suggested last week we will use Protocol buffers as messages format. Protocol buffers is a method of serializing data (e.g. similar to JSON) developed by Google. It offers a library which can take a simple definition file (.proto) and create a full C++ class out of it with all needed methods (such as parsing to ofstream and creating an object back from ifstream).

More about what is Protocol buffers here:
https://developers.google.com/protocol-buffers/docs/overview

### Our .proto file

Here is a suggestion how our *.proto* file can look like. If we come to an agreement we can start implementing it (which should be really easy using the provided google library).

<a href="./message.proto">message.proto</a>

* **senId**: id of the sender of the message
* **recId**: id of the reciever of the message
* **textContent**: the actual content of the message, string, probably limited to some length
* **kdfKeys**: array of needed keys for the double-ratchet algorith used by signal protocol (should be an array of 512b long strings)

### Using the .proto file

After we agree on how the *.proto* file should look like, we can compile it with the Google library, which will create *.cpp* and *.hpp* files of our Message C++ class. This class we will import in our source code and use it as follows (just an example):

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

According to <a href="../design_ideas/dbStructure.md"> dbStructure.md </a> messages will be stored on server in the table **CONVERSATION**.
After the last call (friday 20. 3. 2020) I assume that the specific design of the database is not important right now.
TODO:
