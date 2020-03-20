## Administrative api
* token ... random number or random string of fixed length
* challenge ... random number
* hash() ... hashing function, SHA-256 should be sufficient
* asymmetric crypto ... curve25519 would be ideal, it is supported in mbedtls and can be used for a part of the signal protocol
We will _most likely_ use [protocol buffers](https://developers.google.com/protocol-buffers/) for
communication between client and server, as the C library for Signal protocol uses it. It's
aviliable in many languages, including [C](https://github.com/protobuf-c/protobuf-c)

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
