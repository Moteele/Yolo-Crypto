## Administrative api
* token ... random number or random string of fixed length
* challenge ... random number
* hash() ... hashing function, SHA-256 should be sufficient
* asymmetric crypto ... curve25519 would be ideal, it is supported in mbedtls and can be used for a part of the signal protocol

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
