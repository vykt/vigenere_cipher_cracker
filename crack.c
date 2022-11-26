/*
 *  CUoL Vigenere Cracker 2022
 *         by vykt
 */

/*
 *	ASSUMPTION: matched word is shorter than key length, as 
 *	is the case with the ciphertext this is developed for.
 *
 *	Should this tool be used again where this does not hold 
 *	true, adding this functionality should prove trivial.
 */


//Lib C
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <ctype.h>
#include <string.h>

//Linux Specific
#include <unistd.h> //exit()
#include <errno.h>


#define KEY_LEN 5
#define KEY_OFFSET 4 //starting letter 5
#define TEXT_BUF_LEN 256
#define WORD_LEN 3
#define WORD_NUM 18
#define MATCH 90
#define ASCII_A 65

#define DONE 1
#define SUCCESS 1

#define FILE_OPEN_ERR 1
#define FILE_READ_ERR 2
#define FILE_WRITE_ERR 3
#define CRITICAL_ERR 99


void DEBUG_print_key(char * next_key) {

	char * format = "Attempting key: %s\n";
	char key_corrected[KEY_LEN] = {};

	//correcting key here
	for (int i = 0; i < KEY_LEN; i++) {
		key_corrected[i] = next_key[i] + 65; //Convert to ascii
	}

	printf(format, key_corrected);
}


void get_ciphertext(char * ciphertext_buffer, const char * ciphertext_file) {

	int ret, fd;
	ssize_t rd_wr;
	char * temp_buffer;

	fd = open(ciphertext_file, O_RDONLY);
	if (fd == -1) exit(FILE_OPEN_ERR);

	rd_wr = read(fd, ciphertext_buffer, TEXT_BUF_LEN);
	if (rd_wr < 0) exit(FILE_READ_ERR);

	close(fd);

	temp_buffer = ciphertext_buffer;
	while (*temp_buffer) {
		*temp_buffer = toupper((unsigned char) *temp_buffer);
		++temp_buffer;
	}
}


void get_word_list(char word_list[WORD_NUM][WORD_LEN], const char * word_list_file) {

	int ret, fd;
	ssize_t rd_wr;
	char * read_buffer;
	char * next_word;
	const char delim = ',';

	read_buffer = malloc((WORD_LEN+1)*WORD_NUM + 1);
	if (read_buffer == NULL) exit(CRITICAL_ERR);
	
	fd = open(word_list_file, O_RDONLY);
	if (fd == -1) exit(FILE_OPEN_ERR);

	rd_wr = read(fd, read_buffer, (WORD_LEN+1)*WORD_NUM + 1);
	if (rd_wr < 0) exit(FILE_READ_ERR);

	close(fd);

	strncpy(word_list[0], read_buffer, WORD_LEN);
	next_word = strtok(read_buffer, &delim);
	for (int i = 1; i < WORD_NUM; i++) {
		next_word = strtok(NULL, &delim);
		strncpy(word_list[i], next_word, WORD_LEN);
	}

	free(read_buffer);
}


int increment_key(char * next_key) {

	char * format = "Lowest 2 digits: %d,%d\n";
	next_key[KEY_LEN - 1] = next_key[KEY_LEN - 1] + 1;
	
	//For every possible key
	for (int i = KEY_LEN - 1; i >= 0; i--) {

		//If reached mod 26
		if (next_key[i] == 26 && i != 0) {
			next_key[i-1] = next_key[i-1] + 1;
			for (int j = i; j < KEY_LEN; j++) {
				next_key[j] = 0;
			}

			//Tell user about progress
			if (i == 2) {
				printf(format, next_key[0], next_key[1]);
			}

		//If exhausted all keys
		} else if (next_key[i] == 26 && i == 0) {
			return DONE;
		
		//All other cases
		} else {
			//DEBUG_print_key(next_key);
			return 0;
		}

	} //end for
}


void apply_key(char * ciphertext_buffer, char * potential_plaintext_buffer, 
	          char * next_key) {

	int count = 0;
	char * temp_buffer;

	strcpy(potential_plaintext_buffer, ciphertext_buffer);
	temp_buffer = potential_plaintext_buffer;
	
	while (*temp_buffer) {

		//First, check for ignored characters
		if (*temp_buffer == ' ' || *temp_buffer == '\n' || *temp_buffer == '.') {
			++temp_buffer;
			continue;
		}

		//Now shift characters
		*temp_buffer = *temp_buffer - next_key[count % KEY_LEN];
		if (*temp_buffer < ASCII_A) *temp_buffer = *temp_buffer + 26;

		//Prepare next cycle
		++count;
		++temp_buffer;
	}
}


void apply_key_on_match(char * ciphertext_buffer, char * match_buffer, 
	                    char * next_key) {

	strncpy(match_buffer, ciphertext_buffer+MATCH, WORD_LEN);

	//For every char of matched word
	for (int i = 0; i < WORD_LEN; i++) {
		match_buffer[i] = match_buffer[i] - next_key[(i + KEY_OFFSET) % KEY_LEN];
		if (match_buffer[i] < ASCII_A) match_buffer[i] = match_buffer[i] + 26;
	}
}


int match_words(char * match_buffer, char word_list[WORD_NUM][WORD_LEN]) {

	int ret;

	for (int i = 0; i < WORD_NUM; i++) {
		ret = strncmp(match_buffer, word_list[i], WORD_LEN);
		//If matching one of the words
		if (ret == 0) return SUCCESS;
	} //end for
	
	return 0;
}


void record_hit(char * next_key, char * potential_plaintext_buffer, int fd_out) {

	char * format = "\nKey: %s\n --- \n%s\n --- \n";
	char key_corrected[KEY_LEN] = {};

	//correcting key here
	for (int i = 0; i < KEY_LEN; i++) {
		key_corrected[i] = next_key[i] + 65; //Convert to ascii
	}

	dprintf(fd_out, format, key_corrected, potential_plaintext_buffer);
}


int main(int argc, char ** argv) {

	const char * input_file     = "ciphertext";
	const char * output_file    = "plaintexts.crack";
	const char * word_list_file = "words";

	char * ciphertext_buffer;
	char * potential_plaintext_buffer;
	char * match_buffer;
	char * output_buffer;

	char word_list[WORD_NUM][WORD_LEN];

	int ret, fd_out;

	char next_key[KEY_LEN] = {0, 0, 0, 0, 0}; //Values 0 to 25

	ciphertext_buffer = malloc(TEXT_BUF_LEN);
	potential_plaintext_buffer = malloc(TEXT_BUF_LEN);
	match_buffer = malloc(WORD_LEN + 1);
	output_buffer = malloc(TEXT_BUF_LEN + KEY_LEN + 32);

	//If malloc failed
	if (ciphertext_buffer == NULL || potential_plaintext_buffer == NULL
		|| match_buffer == NULL || output_buffer == NULL) {
		exit(CRITICAL_ERR);
	}
	
	fd_out = open(output_file, O_WRONLY | O_CREAT, 0644);
	if (fd_out == -1) exit(FILE_OPEN_ERR);

	//Get ciphertext & word list
	get_ciphertext(ciphertext_buffer, input_file);
	get_word_list(word_list, word_list_file);

	do {
		//Only decipher 3 letters of matched word
		apply_key_on_match(ciphertext_buffer, match_buffer, next_key);

		//See if the 3 letters make a word in english language
		if (match_words(match_buffer, word_list)) {
			
			apply_key(ciphertext_buffer, potential_plaintext_buffer, next_key);
			record_hit(next_key, potential_plaintext_buffer, fd_out);
		} // end if

	} while (!(increment_key(next_key)));

	close(fd_out);
	return 0;
}
