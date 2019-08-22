# Regular Expression Matcher

## Overview
This application consists mainly of three files:
1. `mygrep.c`, a simplified version of the standard pattern matching program, grep. Uses inheritance hierarchy to implement a significant part of the regex syntax, which is a common way of describing and matching text patterns and is available in multiple programming languages and environments.
2. `pattern.c`, provides an abstract interface for different types of patterns, along with concrete implementations for all matching individual symbols and for matching concatenated patterns.
3. `pattern.h`, contains header components for the `pattern.c` file to be shared with `mygrep.c` for the implementation of the mygrep program.

## Operation
* After it's started, mygrep reads lines from its input until it reaches the end-of-file.
* It prints to standard output any line that contains a match for the pattern, and ignores lines that don't contain a match.
* It only matches against one input line at a time, so that it doesn't have to consider patterns that could match the newline character. Input lines could be arbitrarily long.

### Execution
The mygrep program can be run with either one or two command-line arguments.
* _First argument_ - the regular expression pattern to search for.
* _Second argument_ - the filename from which to read and match lines (optional).

#### Execution with One Argument
* If only one argument is given, it will read and match lines from standard input. `$ ./mygrep 'regular_expression'`
* Run as follows, the program will expect input lines entered from the terminal: `$ ./mygrep 'ab*c'`

#### Execution with Two Arguments
* If with two arguments and run as follows using test file test_09.txt as an example, the program will read lines from the file and print out those that match the pattern: `ab*c`: `$ ./mygrep 'ab*c' test_09.txt`
* If the user attempts to run the program with invalid arguments (e.g., too many or too few), it prints the following usage message to standard error and then exits with an exit status of `EXIT_FAILURE`: `usage: mygrep <pattern> [input-file.txt]`
* If it can't open the input file, it will print the following message to standard error (where filename is the name of the file it wasn't able to open) and exit status, `EXIT_FAILURE`: `Can't open input file: filename`
* If the given pattern isn't a valid regular expression, it will print the following message to standard error and exit with a status of `EXIT_FAILURE`. The program should try to open the input file before trying to parse the pattern, so if they're both bad, it will just report the Can't open input file message: `Invalid pattern`

### Input
* The mygrep program will be given a regular expression on the command line.
* It will then read lines of text from an input file or from standard input, printing out just the lines that match the given pattern.
* Syntax: `$./mygrep 'regular_expression' < input_file.txt`

#### Example
For example, test 12 (input12.text) uses the pattern `a(bc)*d`. This says to match any lines containing an 'a', followed by any number of occurrences of the string, 'bc' (including zero occurrences), followed by 'd':

`$ ./mygrep 'a(bc)*d' < input_12.txt`

The input file, input_12.txt contains the following lines. The first four of these match the pattern, since they contain a string that stats with 'a', followed by zero or more occurrences of 'bc', followed by 'd'. The rest of the lines don't match the pattern:

    abcd
    ad
    abcbcbcbcd
    xyz abcd 123
    abbbbd
    accccd
    acbcbd
    xbcbcx
    a bc d

##### Output
You can run the program as follows, giving it this pattern on the command line and reading input from standard input (redirected from the input_12.txt file):

    abcd
    ad
    abcbcbcbcd
    xyz abcd 123

Notice the use of single quotes around the pattern. This must be done for most patterns, since some of the special characters used in regular expressions are also special characters for the shell. Putting them in single quotes protects them from special interpretation by the shell.


## Matching Regular Expressions
* In the regular expression syntax, a pattern consists of ordinary characters (like 'a' and '5') that just match occurrences of themselves.
* A pattern can also contain metacharacters that match things other than themselves to help control how the regex is parsed or determine how parts of it behave.

### Order of Precedence
The matching rules described below are ordered by __precedence__ levels.
1. Everything from ordinary symbols up to parentheses are at the highest level of precedence.
2. The repetition operators, `*`, `+` and `?` are at the next highest level.
3. Concatenation is at the next highest level and alternation is at the lowest precedence.

### Matching Rules
* `.^$*?+|()[{` Ordinary characters
  - Any printable character other than newline and characters in the set `.^$*?+|()[{`.
  - An ordinary character matches any occurrence of itself, anywhere in the string.
  - E.g., the pattern `a` will match the 'a' in string "abc", "cba" or "xxxaaayyy", but nothing in "xyz".
* `.` Single occurrence of any character
  - E.g., The pattern `.` will match every character in the string "abc", "xyz" or even ".", but it won't match the empty string.
* `^` Start anchor
  - Match the start of the line, a location right before the first character on the line.
  - This can be used to describe patterns that will only match things at the beginning of the line.
* `$` End anchor
  - Match the end of the line, a location right after the last character on the line.
  - This can be used to make patterns that only match things at the end of the line, or used along with `^`, patterns that have to match everything on the line.
* `[]` Character class (a sequence of characters inside square brackets not including the ] or newline character)
  - This matches any one character given in the sequence.
  - E.g., `[abc]` will match any one occurrence of the letter 'a' or the letter 'b' or the letter 'c'.
  - Match characters from just about any set, like the set of decimal digits, `[0123456789]`.
    - And, since just about any character can appear inside square brackets, it serves as a clever way to literally match symbols that would otherwise be interpreted as metacharacters, like `[.]` or `[$]`.
  - Note: The same character can be given more than once when defining a character class (e.g., `[aabc]`); there's no good reason to do this, but it doesn't make the pattern invalid.
* `()` Any pattern
  - Match any pattern _p_, inside parentheses `(p)`.
  - Can use parentheses to control how a regular expression is parsed (as with mathematical expressions).
  - E.g., `ab*` would match an occurrence of 'a' followed by any number of repeated occurrences of 'b', while `(ab)*` would match any number of repeated occurrences of 'ab'.
* `*` 0+ occurrences
  - Zero or more consecutive occurrences of anything a pattern, _p_, matches.
  - E.g., `b*` would match the middle of the strings "abc", "abbbc" or even "ac" (zero occurrences of 'b', so, by itself, `b*` isn't particularly useful).
* `+` A pattern, _p_, followed by `+`.
  - This will match 1+ consecutive occurrences of anything _p_ matches.
* `?` Following a pattern, _p_, matches 0+ consecutive occurrences of anything _p_ matches.
  - A pattern followed by a question mark `?` is like an optional match in a pattern.
* `[][]` Concatenation
  - Match any two consecutive patterns, _p1_ and _p2_ `[p1][p2]`.
  - Concatenation matches anything that can be matched by _p1_ followed immediately by anything that matches _p2_.
  - E.g., `[ab][cd]` matches "ac", "ad", "bc" or "bd".
* `|` Alternation
  - For any two patterns, _p1_ and _p2_, matches anything matched by either _p1_ or _p2_.
  - So, `"cat|dog"` will match 'cat' or 'dog'.

### Matching Summary
The above __inductive__ description describes how to make the smallest regular expressions (the base cases), then how these can be combined into more complex regular expressions.
* This is how regular expressions (or programming languages) are normally described, starting with small bits of syntax and then showing how it can be composed.
* Anything inconsistent with the above description (e.g., `*` or `[abc`) would be considered an invalid pattern.
