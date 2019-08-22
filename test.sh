#!/bin/bash
FAIL=0

# Make a fresh copy of the target program
make clean
make
if [ $? -ne 0 ]; then
  echo "**** Make (compilation) FAILED"
  exit 13
fi

# Compare the content of two given filenames, fail if they differ or
# if one of them doesn't exist.  The first filename should be the
# expected file, and the second should be the one created by the
# program.  An optional third parameter says what to call the file
# that doesn't match.
matchFile() {
    if [ "$#" -eq 3 ]; then
	name="$3"
    else
	name="$2"
    fi

    if [ ! -f "$1" ]; then
	echo "   **** Something's wrong.  Expected a file named $1."
	FAIL=1
	return 1
    fi

    if [ -f "$1" ] && [ ! -f "$2" ]; then
	echo "   **** Test failed - file $2 doesn't exist, but it should."
	FAIL=1
	return 1
    fi


    if ! diff -q "$1" "$2" >/dev/null; then
	echo "   **** Test failed - $name doesn't match expected"
	FAIL=1
	return 1
    fi

    return 0
}

# failure test in its own function, so we can use it for bad
# command-line arguments. Expects TESTNO and actual exit status.
failCheck() {
    TESTNO="$1"
    shift
    STATUS="$1"
    shift

    # Nothing on standard output
    if [ -s output.txt ]; then
	echo "   **** Test failed - shouldn't be any output on standard output"
	FAIL=1
	return 1
    fi

    # Make sure the program's error output looks right.
    if ! matchFile estderr_$TESTNO.txt stderr.txt "error output"; then
	return 1
    fi

    return 0
}

# Function to run the program against a test case, checking
# its output, exit status and error output against what's expected.
runtest() {
    TESTNO="$1"
    shift
    pattern="$1"
    shift
    MODE="$1"
    shift
    EX_STATUS="$1"
    shift

    # Remove any files that we want to test for.
    rm -f output.txt stderr.txt

    # Read either from a file or standard input.
    if [ "$MODE" == "stdin" ]
    then
	echo "Test $TESTNO: ./mygrep '$pattern' < input_$TESTNO.txt > output.txt 2> stderr.txt"
	./mygrep "$pattern" < input_$TESTNO.txt > output.txt 2> stderr.txt
	STATUS=$?
    else
	echo "Test $TESTNO: ./mygrep '$pattern' input_$TESTNO.txt > output.txt 2> stderr.txt"
	./mygrep "$pattern" input_$TESTNO.txt > output.txt 2> stderr.txt
	STATUS=$?
    fi

    # Make sure we got the expected exit status.
    if [ $STATUS -ne $EX_STATUS ]; then
	echo "   **** Test failed - incorrect exit status. Expected: $EX_STATUS Got: $STATUS"
	FAIL=1
	return 1
    fi

    if [ $EX_STATUS -eq 0 ]; then
	# Make sure the program's output looks right.
	if ! matchFile eoutput_$TESTNO.txt output.txt "standard output"; then
	    return 1
	fi

	# Nothing on standard error
	if [ -s stderr.txt ]; then
	    echo "   **** Test failed - shouldn't be any output on standard error"
	    FAIL=1
	    return 1
	fi
    else
	if ! failCheck "$TESTNO" "$STATUS"; then
	    return 1
	fi
    fi

    echo "Test $TESTNO passed"
    return 0
}

# Run each of the test cases
runtest 01 'b' file 0
runtest 02 'abc' file 0
runtest 03 'a.c' file 0
runtest 04 'a..c' stdin 0
runtest 05 '^123' file 0
runtest 06 'wxyz$' file 0
runtest 07 'a[bcdef]g' file 0
runtest 08 'abc|def|ghi' file 0
runtest 09 'ab*c' file 0
runtest 10 'ab+c' file 0
runtest 11 'ab?c' file 0
runtest 12 'a(bc)*d' file 0
runtest 13 '^Your (license|application|program) has been (revoked|accepted|tested)!$' file 0
runtest 14 '[0123456789]+[.][0123456789]+' file 0

runtest 15 '*' file 1
runtest 16 'abc[123' file 1
runtest 17 'abc' file 1


# Bad command-line arguments
rm -f output.txt stderr.txt
echo "Test 18: ./mygrep too many arguments > output.txt 2> stderr.txt"
./mygrep too many arguments > output.txt 2> stderr.txt
STATUS=$?

if failCheck 18 "$STATUS"; then
    echo "Test 18 passed"
fi

if [ $FAIL -ne 0 ]; then
  echo "FAILING TESTS!"
  exit 13
fi

exit 0
