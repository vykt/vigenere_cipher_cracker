# vigenere_cipher_cracker

### PLAGIARISM NOTICE: 

Created by a CUoL student, ID: 210059011


### ABOUT:

Single threaded vigenere cipher cracker written in C for Linux systems.


### FUNCTIONALITY:

In brief, the user must first:

-Identify key length.
-Identify n-gram from ciphertext to attack.
-Provide csv of common words, all same length as chosen n-gram.

When run, the cracker will:

-Decipher the chosen n-gram using every possible key.
-Attempt to match thse deciphered n-grams to every word in the provided csv dictionary.
-If a match is found, entire ciphertext is deciphered using that key. Both key and 
 potential plaintext are written to a log.

The user may then process the log to search for additional common words to identify the 
real key/plaintext pair.


### USING THE CRACKER:

This cracker is programmed for a very specific use case but is easily extensible. If 
modifying the code, refer to this guide for the macro functionality:

```
KEY-LEN      : Length of key.
KEY-OFFSET   : How far through the key the chosen n-gram begins.
TEXT-BUF-LEN : Buffer size to hold the ciphertext.
WORD-LEN     ; Length of chosen n-gram and words in dictionary.
WORD-NUM     : Number of words in dictionary.
MATCH        : Offset of chosen n-gram from beginning of file.
```

The default names for files to be present in the working directory of the 
executable are as follows:

```
words            : Dictionary of words in .csv format. (required)
ciphertext       : Ciphertext. (required)
plaintexts.crack : Output of key & potential plaintext pairs.
```

Refer to the Makefile for compilation details.
