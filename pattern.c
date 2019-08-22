/**
@file pattern.c
@author Stephen Hildebrand (sfhildeb@gmail.com)

The pattern.c component provides an abstract interface for different types
of patterns, along with concrete implementations for all matching individual
symbols (SymbolPattern) and for matching concatenated patterns (BinaryPattern).
*/

/* Headers */
#include "pattern.h"
#include <stdlib.h>
#include <stdio.h>


/*******************************************************************************
*
*                          PATTERN UTILITY FUNCTIONS
*
********************************************************************************/

void reportMarks(const char *str, const bool *marks)
{
  int i = 0;
  while (str[i]) {
    printf("%c%c", marks[i] ? '*' : ' ', str[i]);
    i++;
  }
  printf("%c\n", marks[i] ? '*' : ' ');
}

bool isMatch(const char *str, const bool *marks)
{
  for (int i = 0; str[i]; i++)
    if (marks[i])
      return true;
  return false;
}

/**
A simple function that can be used to free the memory for any
pattern that doesn't allocate any additional memory other than the
struct used to represent it (e.g., if it doesn't contain any
sub-patterns).

@param pat The pattern to free memory for.
*/
static void destroySimplePattern(Pattern *pat)
{
  // The concrete type of pat doesn't matter. free() can still
  // free its memory, given a pointer to the start of it.
  free(pat);
}



/********************************************************************
*
*                    SYMBOL PATTERN DEFINITION
*
********************************************************************/
/**
Type of pattern used to represent a single, ordinary symbol, such
as 'a' or '5'.
*/
typedef struct {
  void(*match)(Pattern *pat, int len, const char *str,
    const bool *before, bool *after);

  void(*destroy)(Pattern *pat);

  char sym;           /* Symbol that the pattern is supposed to match */

} SymbolPattern; // Object construction.

/**
Method used to match a SymbolPattern.
*/
static void matchSymbolPattern(Pattern *pat, int len, const char *str,
  const bool *before, bool *after)
{
  // Cast down to the struct type pat really points to.
  SymbolPattern *this = (SymbolPattern *)pat;

  // Mark first position in after as false.
  after[0] = false;

  // So move any match in before[] forward by one and set it in after[].
  for (int i = 0; i < len; i++)
    // Set next position in after to true if previous position in before
    // was true and if the symbol in str matches this symbol.
    after[i + 1] = (before[i] && str[i] == this->sym);
}

Pattern *makeSymbolPattern(char sym)
{
  // Make an instance of SymbolPattern, and fill in its state.
  SymbolPattern *this = (SymbolPattern *)malloc(sizeof(SymbolPattern));
  this->sym = sym;

  this->match = matchSymbolPattern;
  this->destroy = destroySimplePattern;

  return (Pattern *) this;
}
/*********************** End SYMBOL Pattern ***********************/


/********************** Begin DOT Pattern *************************
Method used to match a DotPattern.
*/
static void matchDotPattern(Pattern *pat, int len, const char *str,
  const bool *before, bool *after)
{

  // Mark first position in after as false.
  after[0] = false;

  // So move any match in before[] forward by one and set it in after[].
  for (int i = 0; i < len; i++)
    // Set next position in after to true if previous position in before
    // was true and if the symbol in str matches this symbol.
    after[i + 1] = (before[i] && str[i]);
}

Pattern *makeDotPattern(char sym)
{
  // Make an instance of SymbolPattern, and fill in its state.
  SymbolPattern *this = (SymbolPattern *)malloc(sizeof(SymbolPattern));
  this->sym = sym;

  this->match = matchDotPattern;
  this->destroy = destroySimplePattern;

  return (Pattern *) this;
}
/************************ End DOT Pattern ************************/


/****************** Begin START ANCHOR Pattern ********************
Method used to match a StartAnchorPattern.
*/
static void matchStartAnchorPattern(Pattern *pat, int len, const char *str,
  const bool *before, bool *after)
{

  // Mark first position in after as true for start anchor.
  after[0] = true;

}

Pattern *makeStartAnchorPattern(char sym)
{
  // Make an instance of SymbolPattern, and fill in its state.
  SymbolPattern *this = (SymbolPattern *)malloc(sizeof(SymbolPattern));
  this->sym = sym;

  this->match = matchStartAnchorPattern;
  this->destroy = destroySimplePattern;

  return (Pattern *) this;
}
/******************** End START ANCHOR Pattern ********************/


/****************** Begin END ANCHOR Pattern ********************
Method used to match a StartAnchorPattern.
*/
static void matchEndAnchorPattern(Pattern *pat, int len, const char *str,
  const bool *before, bool *after)
{
  // Mark first position in after[] as false.
  after[0] = false;
  after[len] = true;

  if (before[len - 1] == false)
    for (int i = 0; i < len - 1; i++)
      after[i] = false;

}

Pattern *makeEndAnchorPattern(char sym)
{
  // Make an instance of SymbolPattern, and fill in its state.
  SymbolPattern *this = (SymbolPattern *)malloc(sizeof(SymbolPattern));
  this->sym = sym;

  this->match = matchEndAnchorPattern;
  this->destroy = destroySimplePattern;

  return (Pattern *) this;
}
/******************** End END ANCHOR Pattern ********************/





/********************************************************************
*
*                    BINARY PATTERN DEFINITION
*
********************************************************************/
/**
Representation for a type of pattern that just contains two
sub-patterns (e.g., concatenation).
*/
typedef struct {
  void(*match)(Pattern *pat, int len, const char *str,
    const bool *before, bool *after);

  void(*destroy)(Pattern *pat);

  Pattern *p1, *p2;         /* Pointer to one of two sub-patterns */
} BinaryPattern;

// BinaryPattern destroy function implementation
static void destroyBinaryPattern(Pattern *pat)
{
  // Cast down to the struct type pat really points to.
  BinaryPattern *this = (BinaryPattern *)pat;

  // Free the two subpatterns, then free the struct.
  this->p1->destroy(this->p1);
  this->p2->destroy(this->p2);
  free(this);
}



/***************** Begin CONCATENATION Pattern ********************
Match function for a BinaryPattern used to handle concatenation
and compute a new set of marked locations.
*/
static void matchConcatenationPattern(Pattern *pat, int len, const char *str,
  const bool *before, bool *after)
{

  // Cast down to the struct type pat really points to.
  BinaryPattern *this = (BinaryPattern *)pat;

  // Temporary storage for the marks after matching the first sub-pattern.
  bool midMarks[len + 1];

  // Match each of the sub-patterns in order.
  this->p1->match(this->p1, len, str, before, midMarks);
  this->p2->match(this->p2, len, str, midMarks, after);
}

Pattern *makeConcatenationPattern(Pattern *p1, Pattern *p2)
{
  // Make an instance of BinaryPattern and fill in its fields.
  BinaryPattern *this = (BinaryPattern *)malloc(sizeof(BinaryPattern));
  this->p1 = p1;
  this->p2 = p2;

  this->match = matchConcatenationPattern;
  this->destroy = destroyBinaryPattern;

  return (Pattern *) this;
}
/************** End CONCATENATION Pattern Definition ***************/


/****************** Begin ALTERNATION Pattern *********************
Match function for a BinaryPattern used to handle alternation
and compute a new set of marked locations.
*/
static void matchAlternationPattern(Pattern *pat, int len, const char *str,
  const bool *before, bool *after)
{

  // Cast down to the struct type pat really points to.
  BinaryPattern *this = (BinaryPattern *)pat;

  // Temporary storage for the marks after matching each subpattern.
  //bool afterMarks[len + 1];

  // Match each of the sub-patterns without one affecting the others marks.
  this->p1->match(this->p1, len, str, before, after);
  if (!isMatch(str, after))
    this->p2->match(this->p2, len, str, before, after);

}

Pattern *makeAlternationPattern(Pattern *p1, Pattern *p2)
{
  // Make an instance of BinaryPattern and fill in its fields.
  BinaryPattern *this = (BinaryPattern *)malloc(sizeof(BinaryPattern));
  this->p1 = p1;
  this->p2 = p2;

  this->match = matchAlternationPattern;
  this->destroy = destroyBinaryPattern;

  return (Pattern *) this;
}
/************** End ALTERNATION Pattern Definition ***************/



/********************************************************************
*                                                                   *
*                  REPETITION PATTERN DEFINITION                    *
*                                                                   *
********************************************************************/
/**
Representation for a type of pattern that contains a repetition of
another sub-pattern.
*/
typedef struct {
  void(*match)(Pattern *pat, int len, const char *str,
    const bool *before, bool *after);

  void(*destroy)(Pattern *pat);

  Pattern *p;       /* Pointer to subpattern for this repetition */
} RepitPattern;

// Destroy function used for RepitPattern.
static void destroyRepitPattern(Pattern *p)
{
  // Cast down to the struct type pat really points to.
  RepitPattern *this = (RepitPattern *)p;

  // Free the subpattern, then free the struct.
  this->p->destroy(this->p);
  free(this);
}



/********************** Begin STAR Pattern ************************
Match function for a RepitPattern used to handle zero or more
repetitions of a subpattern and compute a new set of marked locations.
*/
static void matchStarPattern(Pattern *pat, int len, const char *str,
  const bool *before, bool *after)
{

  // Cast down to the struct type pat really points to.
  RepitPattern *this = (RepitPattern *)pat;

  // From the before array, compute all locations in the string, str,
  // that could be reached after going on to match subpattern, pat.


}

Pattern *makeStarPattern(Pattern *p)
{
  // Make an instance of RepitPattern and fill in its fields.
  RepitPattern *this = (RepitPattern *)malloc(sizeof(RepitPattern));
  this->p = p;

  this->match = matchStarPattern;
  this->destroy = destroyRepitPattern;

  return (Pattern *) this;
}
/********************** End STAR Pattern **************************/



/********************* Begin PLUS Pattern  *************************
Match function for a RepitPattern used to handle one or more
repetitions of a subpattern and compute a new set of marked locations.

It should leave a mark at every location that could be the end of a
match. So, on the input string "abbb", the pattern "ab+" should leave
marks at " a b*b*b*"
*/
static void matchPlusPattern(Pattern *pat, int len, const char *str,
  const bool *before, bool *after)
{
  // Cast down to the struct type pat really points to.
  RepitPattern *this = (RepitPattern *)pat;

  // From the before array, compute all locations in the string, str,
  // that could be reached after going on to match subpattern, pat.


}

Pattern *makePlusPattern(Pattern *p)
{
  // Make an instance of RepPattern and fill in its fields.
  RepitPattern *this = (RepitPattern *)malloc(sizeof(RepitPattern));
  this->p = p;

  this->match = matchStarPattern;
  this->destroy = destroyRepitPattern;

  return (Pattern *) this;
}
/*********************** End PLUS Pattern *************************/



/********************* Begin QMARK Pattern *************************
Match function for a RepitPatter used to handle either zero or one
repetitions of a subpattern and compute a new set of marked locations.
*/
static void matchQMarkPattern(Pattern *pat, int len, const char *str,
  const bool *before, bool *after)
{
  // Cast down to the struct type pat really points to.
  RepitPattern *this = (RepitPattern *)pat;
}

Pattern *makeQMarkPattern(Pattern *p)
{
  // Make an instance of RepitPattern and fill in its fields.
  RepitPattern *this = (RepitPattern *)malloc(sizeof(RepitPattern));
  this->p = p;

  this->match = matchStarPattern;
  this->destroy = destroyRepitPattern;

  return (Pattern *) this;
}
/*********************** End QMARK Pattern *************************/
