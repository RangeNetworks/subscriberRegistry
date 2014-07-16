#!/bin/bash

# This script will extract entries from /var/log/OpenBTS for registrations
# and failed registrations.

# Usage: regs.sh [-s] [-f] [-j]
#       Where -s is entries that succeeded.
#       Where -f is entries that failed.
#       If neither is given, both are listed.
#       Where -j formats output for json

success="false"
failure="false"
json="false"
#logfile="/var/log/OpenBTS.log"
#logfile="data.txt"
logfile="foo.log"

while [ "$#" != "0" ]
do
    if [ "$1" == "-f" ]
    then
        failure="true"
    elif [ "$1" == "-s" ]
    then
        success="true"
    elif [ "$1" == "-j" ]
    then
        json="true"
    else
        echo Invalid parameter $1
        exit 1
    fi
    shift
done
if [ "$success" == "false" ] && [ "$failure" == "false" ]
then
    echo None given, setting both
    success="true"
    failure="true"
else
    echo Success $success
    echo Failure $failure
fi
#strsip="select id from sip_buddies"
#strki="select ki from sip_buddies"
#strres="sqlQuery: result = "
#egrep "${strsip}|${strki}|${strres}" | \

#grep sipauthserve $logfile | grep SubscriberRegistry.cpp | \

    sed 's/^Jan \(..\) \(..:..:..\) / 2014:01:\1T\2A /g' < $logfile | \
    sed 's/^Feb \(..\) \(..:..:..\) / 2014:02:\1T\2A /g' | \
    sed 's/^Mar \(..\) \(..:..:..\) / 2014:03:\1T\2A /g' | \
    sed 's/^Apr \(..\) \(..:..:..\) / 2014:04:\1T\2A /g' | \
    sed 's/^May \(..\) \(..:..:..\) / 2014:05:\1T\2A /g' | \
    sed 's/^Jun \(..\) \(..:..:..\) / 2014:06:\1T\2A /g' | \
    sed 's/^Jul \(..\) \(..:..:..\) / 2014:07:\1T\2A /g' | \
    sed 's/^Aug \(..\) \(..:..:..\) / 2014:08:\1T\2A /g' | \
    sed 's/^Sep \(..\) \(..:..:..\) / 2014:09:\1T\2A /g' | \
    sed 's/^Oct \(..\) \(..:..:..\) / 2014:10:\1T\2A /g' | \
    sed 's/^Nov \(..\) \(..:..:..\) / 2014:11:\1T\2A /g' | \
    sed 's/^Dec \(..\) \(..:..:..\) / 2014:12:\1T\2A /g' | \
    gawk -f regs.gawk -v success=$success -v failure=$failure -v json=$json
