#!/bin/sh
#
# Test @include with garbage after the path name
# The standard error output is dup'd to the standard output.
#

# Avoid warnings about memory leaks when there is a syntax error
ASAN_OPTIONS=detect_leaks=0; export ASAN_OPTIONS

MYUID=`\ls -ln $TESTDIR/test2.inc | awk '{print $3}'`
MYGID=`\ls -ln $TESTDIR/test2.inc | awk '{print $4}'`

echo "Testing @include with garbage after the path name"
echo ""
./testsudoers -U $MYUID -G $MYGID root id <<EOF 2>&1 | sed 's/\(syntax error\), .*/\1/' 
@include sudoers.local womp womp
EOF

echo ""
echo "Testing #include with garbage after the path name"
echo ""
./testsudoers -U $MYUID -G $MYGID root id <<EOF 2>&1 | sed 's/\(syntax error\), .*/\1/' 
#include sudoers.local womp womp
EOF

exit 0
