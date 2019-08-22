/**
@file mygrep.c
@author Stephen Hildebrand (sfhildeb@gmail.com)

Overview:
The mygrep program is a simplified version of grep, a standard pattern matching
program. Like grep, it is a command-line utility for searching plain-text data
sets for lines that match a regular expression. After it's started, mygrep just
needs to read lines from its input until it reaches the end-of-file.
<p>
It prints to standard output any line that contains a match for the pattern,
and ignores lines that don't contain a match.
<p>
This program limits itself to matching against one input line at a
time to avoid having to consider patterns that could match the newline
character. Input lines could be arbitrarily long.
<p>
The mygrep.c component contains the main() function. It's responsible for
handling command-line arguments, parsing the regular expression and matching
it against lines from the input.
*/

/* Headers */
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pattern.h"


/* Constant Definitions */
// Each CLA below has 1 added to it to account for program at position 0.
#define ONE_ARG 2   /* Count of args for pattern input only + 1 */
#define TWO_ARGS 3  /* Count of args when an input file is passed */

/* Prototoypes */
//static void testCode();


/********************************************************************
*
*                        UTILITY FUNTIONS
*
********************************************************************/
/**
Return true if the given character is ordinary, if it should just
match occurrences of itself. This returns false for metacharacters
like '*' that control how patterns are matched.

@param c Character that should be evaluated as ordinary or special.
@return True if c is not special.
*/
static bool ordinary(char c)
{
  // See if c is on our list of special characters.
  if (strchr(".^$*?+|()[{", c))
    return false;
  return true;
}

/**
Print appropriate error message for invalid pattern, exit unsuccessfully.
*/
static void invalidPattern()
{
  fprintf(stderr, "Invalid pattern\n");
  exit(EXIT_FAILURE);
}


/********************************************************************
*
*                          PARSER FUNCTIONS
*
********************************************************************/
/*
* Note: The job of the parser is to build a tree of Pattern objects
* representing the regular expression parsed. The pattern component
* actually implements these objects, exposing just a constructor for
* each type of object.
********************************************************************/

/**
Parse regular expression syntax with the 1st-highest precedence level,
including individual ordinary symbols, start ^ and end $ anchors,
character classes [], and patterns surrounded by parentheses (pattern).

@param str The string being parsed.
@param pos A pass-by-reference value for the location in str being
           parsed, increased as characters from str are parsed.
@return a dynamically allocated representation of the pattern for the
        next portion of str.
*/
static Pattern *parseAtomicPattern(char *str, int *pos)
{
  if (ordinary(str[*pos]))
    return makeSymbolPattern(str[(*pos)++]);
  else if (str[*pos] == '.')
    return makeDotPattern(str[(*pos)++]);
  else if (str[*pos] == '^')
    return makeStartAnchorPattern(str[(*pos)++]);
  else if (str[*pos] == '$')
    return makeEndAnchorPattern(str[(*pos)++]);
  //else if (str[*pos] == ']')
  //  return;

  invalidPattern();
  return NULL; // Just to make the compiler happy.
}

/**
Parse regular expression syntax with the 2nd-highest precedence. A
pattern, p, optionally followed by one or more repetition syntax like
'*', '+' or '?'. If there's no repetition syntax, it just returns the
pattern object for p.

Uses parseAtomicExpression() to parse whatever pattern needs to
be repeated. Then, if a pattern like (abc)+ is found, the
parseAtomicPattern() will take care of parsing the (abc) part, and the
parseRepetition() will only need to worry about noticing the + afterward.

@param str The string being parsed.
@param pos A pass-by-reference value for the location in str being
           parsed,increased as characters from str are parsed.
@return a dynamically allocated representation of the pattern for the
        next portion of str.
*/
static Pattern *parseRepetition(char *str, int *pos)
{
  Pattern *p = parseAtomicPattern(str, pos);
  return p;
}

/**
Parse regular expression syntax with the 3rd-highest precedence.
One pattern, p, (optionally) followed by additional patterns
(concatenation).  If there are no additional patterns, it just
returns the pattern object for p.

@param str The string being parsed.
@param pos A pass-by-reference value for the location in str being
           parsed,increased as characters from str are parsed.
@return a dynamically allocated representation of the pattern for the
        next portion of str.
*/
static Pattern *parseConcatenation(char *str, int *pos)
{
  // Parse the first pattern.
  Pattern *p1 = parseRepetition(str, pos);
  // While there are additional patterns, parse them.
  while (str[*pos] && str[*pos] != '|' && str[*pos] != ')') {
    Pattern *p2 = parseRepetition(str, pos);
    // And build a concatenation pattern to match the sequence.
    p1 = makeConcatenationPattern(p1, p2);
  }

  return p1;
}

/**
Parse regular expression syntax with the lowest precedence (4th). One
pattern, p, (optionally) followed by additional patterns separated by
| (alternation). If there are no additional patterns, just returns the
pattern object for p.

@param str The string being parsed.
@param pos A pass-by-reference value for the location in str being
           parsed,increased as characters from str are parsed.
@return a dynamically allocated representation of the pattern for the
        next portion of str.
*/
static Pattern *parseAlternation(char *str, int *pos)
{
  Pattern *p1 = parseConcatenation(str, pos);
  while (str[*pos] && str[*pos] == '|') {
    (*pos)++;
    Pattern *p2 = parseConcatenation(str, pos);
    p1 = makeAlternationPattern(p1, p2);
  }
  return p1;
}


/********************************************************************
*
*                           MAIN METHOD
*
********************************************************************/
/**
The main method for the mygrep program. It can be run with either
one command-line argument or with two. If only one command-line
argument is given, it will read and match lines from standard input.

@param argc The count of command line arguments.
@param argv The command line arguments array.
@return The programs successful or unsuccessful exit status.
*/
int main(int argc, char *argv[])
{
  FILE *input = NULL;       /* Input file (if not standard in) */
  Pattern *pat = NULL;      /* Pattern object to search for */
  char *str = NULL;         /* Next line read from input */

  // If one argument, read and match lines from standard input.
  // If two, then read and use the input file instead.
  if (argc == ONE_ARG) {          // Standard input.
    input = stdin;
  } else if (argc == TWO_ARGS) {  // File input.
    input = fopen(argv[2], "r");
    if (input == NULL) {          // Failed input.
      fprintf(stderr, "Can't open input file: %s\n", argv[2]);
      exit(EXIT_FAILURE);
    }
  } else { // Invalid number of args, exit with status of EXIT_FAILURE.
      fprintf(stderr, "usage: mygrep <pattern> [input-file.txt]\n");
      exit(EXIT_FAILURE);
      }

  // Parse the pattern into a Pattern object.
  int pos = 0;
  pat = parseAlternation(argv[1], &pos);

  // Try matching each line, str, of the input text to the pattern.
  size_t size = 100;
  str = (char *)malloc(size + 1);
  while (getline(&str, &size, input) > 0) {

    int len = strlen(str);
    // Instantiate before[] to necessary length and fill with true.
    bool *before = (bool *)malloc((len + 1) * sizeof(bool));
    for (int i = 0; i < len; i++) {
      before[i] = true;
    }
    // Instantiate space for after[] based on before[].
    bool after[sizeof(before) / sizeof(before[0])];

    // Test code
    //printf("Before matching: ");
    //reportMarks(str, before);

    // Perform the pattern match function to match the line str
    pat->match(pat, strlen(str), str, before, after);

    // Test code
    //printf("After matching: ");
    //reportMarks(str, after);

    // Print out any successful matches.
    if (str != NULL && isMatch(str, after)) {
      printf("%s", str);
    }

  }

  pat->destroy(pat);

  return(EXIT_SUCCESS);
}
