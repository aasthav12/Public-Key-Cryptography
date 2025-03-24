# Assignment 5 - Public Key Cryptography

This program implements cryptography to generate public and private key pairs. It is implemented using the SS algorithm. The user is able to use these public and private key pairs to encrypt and decrypt files of their choosing.

## Building

```
$ make all
```
To build the keygen, encrypt, or decrypt 
```
$ make keygen
$ make encrypt
$ make decrypt
```

## Running

```
$ ./keygen [options]
$ ./encrypt [options]
$ ./decrypt [options]
```
## Examples

In order to run the program, you would want to use the command ./keygen [options], and then go about the following few ways:

1. type in ./encrypt and specify the options, and if stdin is enabled type in the message to be encrypted, hit enter and then cntrl+d. Then it will output the public key, which you will copy and when ./decrypt is run with or without options, it will allow for user input of the copied public key to be inputted, which you could paste, hit enter and cntrl+d. Then the decrypted message will either be sent to a specified outfile or stdout.
2. echo "[STDIN MESSAGE]" | ./encrypt | ./decrypt
3. ./encrypt -i [FILE NAME] | ./decrypt
