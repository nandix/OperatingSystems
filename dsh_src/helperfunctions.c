/************************************************************************//**
 *  @file helperfunctions.c
 *
 *  @brief Helper function definitions for dsh.
 ***************************************************************************/

#include "helperfunctions.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


/***************************************************************************//**
 * @author Joe Lillo
 *
 * @par Description:
 * Gets a line of input from stdin and returns it.
 * Removes newlines and null terminates the string.
 *
 * @return char* - Input from stdin
 ******************************************************************************/
char * getInput ()
{
    char * input = NULL;
    size_t size = 0;

    // Get a line of input from stdin
    size_t len = getline(&input, &size, stdin);

    // Replace newline with null terminator.
    if('\n' == input[len-1])
    {
        input[len-1] = '\0';
    }

    return input;
}


/***************************************************************************//**
 * @author Joe Lillo
 *
 * @par Description:
 * Seperates the given input into seperate words.
 *
 * @param[in] input - Input string to tokenize.
 * @param[out] wordCount - Used to return the number of words found.
 *
 * @return char** - Array of individual arguments found in input string.
 ******************************************************************************/
char ** getArgs (char * input, int * wordCount)
{
    // Error checking
    if (NULL == input)
    {
        return NULL;
    }

    char * i = input;
    int spaces = 0;

    // Iterate through string counting spaces.
    // isWhitespace() ????
    while('\0' != *i)
    {
        if (' ' == *i)
        {
            spaces += 1;
        }
        i += 1;
    }

    i = input;
    char ** args = malloc(sizeof(char*) * (spaces+2));
    int wordNum = 0;

    args[spaces+1] = NULL;

    // Iterate through words in string -- store them in 2d character array.
    do
    {
        char * space = strchr(i,' ');

        if (NULL == space)
        {
            space = strchr(i,'\0');
        }

        args[wordNum] = malloc(space-i + sizeof(char));
        memcpy(args[wordNum],i,(space-i));
        args[wordNum][(space-i)] = '\0';

        i = space + 1;
        wordNum += 1;
    }while(wordNum <= spaces);

    *wordCount = wordNum;
    return args;
}

///***************************************************************************//**
// * @author Joe Lillo
// *
// * @par Description:
// * Seperates the given input into seperate words.
// *
// * @param[in] input - Input string to tokenize.
// * @param[out] wordCount - Used to return the number of words found.
// *
// * @return char** - Array of individual arguments found in input string.
// ******************************************************************************/
//char ** getArgs (char * input, int * wordCount)
//{
//    // Error checking
//    if (NULL == input)
//    {
//        return NULL;
//    }

//    char * inCopy = malloc(sizeof(char) * strlen(input) + 1);
//    strcpy(inCopy, input);


//    *wordCount = 0;
//    char* token = strtok(inCopy, " ");
//    while (token)
//    {
//        (*wordCount)++;
//        token = strtok(NULL, " ");
//    }

//    char ** args = malloc( sizeof(char*) * * (wordCount+1)  );
//    args[*wordCount] = 0;

//    int c = 0;

//    token = strtok(input, " ");
//    while (token)
//    {
//        args[c] = malloc(strlen(token) + 1);
//        strcpy(args[c], token);
//        token = strtok(NULL, " ");
//        c++;
//    }

//    free(inCopy);

//    return args;
//}

/***************************************************************************//**
 * @author Joe Lillo
 *
 * @par Description:
 * Converts a string to an integer. (ie) "101" -> 101.
 *
 * @param[in] str - String to convert to int.
 * @param[in] ok [out] - Acts as boolean to indicate success of function.
 *
 * @return Integer representation of string.
 ******************************************************************************/
int strToInt (char * str, int * ok)
{
    int num = 0;
    unsigned int i;

    // Check if the number is negative.
    int neg = (str[0] == '-');

    // Error checking.
    if(0 == strlen(str))
    {
        *ok = -1;
        return 0;
    }

    // Set starting position for loop.
    if (neg)
    {
        i = 1;
    }
    else
    {
        i = 0;
    }

    // Iterate through string. Determine the value of each character.
    // Add that value to num and multiply num by 10.
    for (; i < strlen(str); i++)
    {
        int temp = str[i] - '0';
        if (temp < 0 || temp > 9)
        {
            *ok = -1;
            return 0;
        }
        num *= 10;
        num += temp;
    }

    *ok = 0;

    if (neg)
    {
        num *= -1;
    }
    return num;
}


