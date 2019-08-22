/**
@file pattern.h
@author Stephen Hildebrand (sfhildeb@gmail.com)

The pattern.h file contains header components for the pattern.c file to be
shared with mygrep.c for the implementation of the mygrep program.
<p>
When match() is called for some subpattern, p, it is given an array
named before, containing marks for all locations in the string that
could be reached by matching everything in the pattern up to but not
including p. From this, the match() function for p has to compute all
the locations in the string that could be reached after going on to
match subpattern p itself.
*/
#ifndef _PATTERN_H_
#define _PATTERN_H_

#include <stdbool.h>

//////////////////////////////////////////////////////////////////////
// Superclass for Patterns

/** A short name to use for the Pattern interface. */
typedef struct PatternTag Pattern;

/**
Structure used as a superclass/interface for patterns. It includes
an overrideable method for matching against a given string, and for
destroying itself to free any allocated resources.
*/
struct PatternTag {
  /**
  Pointer to a function to match this pattern against the given string by
  computing a new set of marked locations. It fills in locations in the
  after array to indicate places in the string that could be reached
  after this pattern is matched. The before and after arrays must be one
  element longer than the length of the string being matched. Patterns are
  matched against input strings by computing what locations in the input
  string could be reached after matching a particular pattern or part of a
  pattern. The locations in a string are treated as being between the
  characters, including before the first character and after the last
  character. So, for a string of length n, there will be n + 1 locations.

  @param pat The pattern that's supposed to match itself against the string.
  @param len Length of the string, we could compute it, but it's more
             efficient to pass it in.
  @param str The input string being matched against.
  @param before Marks for locations in the string that could be reached
                before matching this pattern.
  @param after Marks for locations in the string that can be reached after
               matching this pattern.
  */
  void(*match)(Pattern *pat, int len, const char *str,
    const bool *before, bool *after);

  /**
  Free memory for this pattern, including any subpatterns it contains.
  @param pat pattern to free.
  */
  void(*destroy)(Pattern *pat);
};

/**
Make a pattern for a single, non-special character, like `a` or `5`.

@param sym The symbol this pattern is supposed to match.
@return A dynamically allocated representation for this new pattern.
*/
Pattern *makeSymbolPattern(char sym);

/**
Make a pattern for one occurrence of any character, as specified by
the . symbol.

@param sym The symbol this pattern is supposed to match.
@return A dynamically allocated representation for this new pattern.
*/
Pattern *makeDotPattern(char sym);

/**
Make a pattern for the start anchor, ^.

@param sym The symbol this pattern is supposed to match.
@return A dynamically allocated representation for this new pattern.
*/
Pattern *makeStartAnchorPattern(char sym);

/**
Make a pattern for the end anchor, ^.

@param sym The symbol this pattern is supposed to match.
@return A dynamically allocated representation for this new pattern.
*/
Pattern *makeEndAnchorPattern(char sym);

/**
Make a pattern for the concatenation of patterns p1 and p2. It should match
anything that can be broken into two substrings, s1 and s2, where the p1
matches the first part (s1) and p2 matches the second part (s2).

@param p1 Subpattern for matching the first part of the string.
@param p2 Subpattern for matching the second part of the string.
@return A dynamically allocated representation for this new pattern.
*/
Pattern *makeConcatenationPattern(Pattern *p1, Pattern *p2);

/**
Make a pattern for the alternation of patterns p1 and p2. It matches
anything that can be matched by either p1 or p2. So "cat|dog" will
match "cat" or "dog".

@param p1 Subpattern for matching the first part of the string.
@param p2 Subpattern for matching the second part of the string.
@return A dynamically allocated representation for this new pattern.
*/
Pattern *makeAlternationPattern(Pattern *p1, Pattern *p2);


/**
Make a pattern for matching zero or more consecutive occurrences of
anything that p matches. For example, b* would match the middle of the
strings "abc", "abbbc" or even "ac" (zero occurrences of b).

@param p A pattern followed by *.
@return A dynamically allocated representation of this new pattern.
*/
Pattern *makeStarPattern(Pattern *p);

/**
Make a pattern for matching one or more consecutive occurrences of
anything that p matches.

@param p A pattern followed by +.
@return A dynamically allocated representation of this new pattern.
*/
Pattern *makePlusPattern(Pattern *p);

/**
Make a pattern for matching zero or one consecutive occurrences of
anything that p matches. For example, a pattern followed b a
question mark is like an optional match in a pattern.

@param p A pattern followed by ?.
@return A dynamically allocated representation of this new pattern.
*/
Pattern *makeQMarkPattern(Pattern *p);

/**
Helpful function to print out a string, with the marks shown between
the characters as asterisks.

@param str String to report.
@param marks Array of marks between the characters of str, assumed
             to be one element longer than the length of str.
*/
void reportMarks(const char *str, const bool *marks);

/**
Returns whether or not the string was a pattern match.

@param str String to report.
@param marks Array of marks between the characters of str, assumed
to be one element longer than the length of str.
*/
bool isMatch(const char *str, const bool *marks);

#endif
