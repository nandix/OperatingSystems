/************************************************************************//**
 *  @file helperfunctions.h
 *
 *  @brief Helper function prototypes for dsh.
 ***************************************************************************/

#ifndef HELPERFUNCTIONS_H
#define HELPERFUNCTIONS_H

char ** getArgs (char * input, int * wordCount);
char * getInput ();

// Converts a string to and integer.
int strToInt (char * str, int * ok);

#endif
