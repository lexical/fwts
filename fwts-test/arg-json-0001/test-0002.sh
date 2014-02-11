#!/bin/bash
#
TEST="Test -j option"
NAME=test-0002.sh
TMPLOG=$TMP/klog.log.$$
HERE=`pwd`

$FWTS --log-format="%line %owner " -w 80 -j $HERE --klog=klog.txt klog - | grep "^[0-9]*[ ]*klog" | cut -c7- > $TMPLOG
diff $TMPLOG klog-0002.log >> $FAILURE_LOG
ret=$?
if [ $ret -eq 0 ]; then 
	echo PASSED: $TEST, $NAME
else
	echo FAILED: $TEST, $NAME
fi

rm $TMPLOG
exit $ret