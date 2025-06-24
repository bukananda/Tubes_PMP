#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "helpers.h"

//Helper function to trim and normalize string
void trim_and_normalize_whitespace(char *str){
    //Return directly if it's empty string
    if(str == NULL || *str == '\0') return;

    //Replace non-breaking spaces (0xA0 byte) with regular spaces (0x20 byte)
    for(char *p = str; *p != '\0'; ++p){
        if((unsigned char)*p == 0xA0){ //Check for non-breaking space byte
            *p = ' '; // Replace with regular space
        }
    }

    //Trim leading whitespaces
    char *start = str;
    while (*start != '\0' && isspace((unsigned char)*start)) {
        start++;
    }

    //Trim trailing whitespaces
    char *end = str + strlen(str) - 1;
    while (end >= start && isspace((unsigned char)*end)) {
        end--;
    }

    //Calculate new string length
    int len = (int)(end - start + 1);

    //Shift content to the beginning after removing leading spaces
    if (start > str) {
        memmove(str, start, len);
    }
    
    // Null-terminate the string, so it won't get error
    str[len] = '\0';
}

//Helper function to count for tokens in doc string
int count_tokens(const char* input_string){
    //Return 0 on empty input
    if(input_string == NULL || strlen(input_string) == 0){ return 0; }
    
    //Make a copy
    char temp_str[5100]; 
    strncpy(temp_str, input_string, sizeof(temp_str) - 1);
    temp_str[sizeof(temp_str) - 1] = '\0';

    char *token;
    char *context = NULL;
    int count = 0;

    token = strtok_s(temp_str, ",", &context);
    while(token != NULL){
        trim_and_normalize_whitespace(token); //Trim each token
        if(strlen(token) > 0){ //Only count if token is not empty after trimming
            count++;
        }
        token = strtok_s(NULL, ",", &context);
    }
    return count;
}