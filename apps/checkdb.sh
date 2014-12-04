#!/bin/sh

set -e

tmp_DB='/tmp/SR.db'
conf_DATA='./sipauthserve.example.sql'

if [ ! -f ${conf_DATA} ]; then
	exit 1
fi

echo "* check integrity of ${conf_DATA}"
sqlite3 -bail ${tmp_DB} ".read ${conf_DATA}" > /dev/null
exit $?
