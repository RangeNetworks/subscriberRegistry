#!/usr/bin/gawk

BEGIN {
    #print "Success = " success
    #print "Failure = " failure
    #print "Json = " json
    state = 0 # each line represents a different state, this is used for message processing flow
    first="true"
    if (json == "true") {
        print "{"
    } else {
        printf "%-32s %-9s %-20s\n", "TIME", "ID", "IMSI", "KI"
        printf "================================ ========= ====================\n"
    }
    debugState=0 # set to 1 to debug
    line=0
    start=systime()
    #print "Start @ " start
}
{
    ++line
    #printf "line %d\r", line
    switch (state) {
    case 0: # Initial state - this is the state that process the initial select id from sip_buddies line
        # We stay in this state until we get a "select id from sip_buddies
        # where username= type message
        timestamp=$1
        query=$0
        sub(/^.*SubscriberRegistry.cpp:[0-9]*:/,"",query)
        $0 = query
        if (debugState != 0) print "Line: " line ", State: " state ", Message: '" query "'"
        if (/^sqlLocal: select id from sip_buddies where username = "/) {
            imsi = $9
            sub(/"/,"",imsi) # leading quote
            sub(/"/,"",imsi) # trailing quote
            id = ""
            ki = ""
            state = 1 # Determine if success or failure
        } else
        {
            state = 0
        }
        break
    case 1: # Determine if success or failure
        # Result = ### line
        # or not found: select id... line
        query=$0
        sub(/^.*SubscriberRegistry.cpp:[0-9]*:/,"",query)
        $0 = query
        if (debugState != 0) print "Line: " line ", State: " state ", Message: '" query "'"
        if (/^.*not found: select id.*$/) {
                #print "Not found"
                if (failure == "true") {
                    if (json == "true") {
                        printf "    "
                        if (first == "true") {
                            first = "false"
                            printf " "
                        } else {
                            printf ","
                        }
                        printf "{ \"timestamp\": \"%s\", \"id\": \"%s\", \"imsi\": \"%s\" }\n",
                            timestamp, "NOT FOUND", imsi
                    } else {
                        printf "%-32s %-9s %-20s\n", timestamp, "NOT FOUND", imsi
                    }
                }
                state=0
        } else {
                # This should be a "result = " line with an id
                #print "We have an entry"
                id = $4
                state=2
        }
        break


    ################################################################
    ################################################################
    ##                                                            ##
    ## This set of steps is for the case where we found the entry ##
    ##                                                            ##
    ################################################################
    ################################################################
    case 2: # skip the KI query
        # select ki from sip_buddies where username =
        query=$0
        sub(/^.*SubscriberRegistry.cpp:[0-9]*:/,"",query)
        $0 = query
        if (/^sqlLocal: select ki from sip_buddies where username = "/) {
            if (debugState != 0) print "Line: " line ", State: " state ", Message: '" query "'"
            # Next state
            state=3
        } else {
            # stay in the state - a spurrious result line
        }
        break
    case 3: # KI result
        query=$0
        sub(/^.*SubscriberRegistry.cpp:[0-9]*:/,"",query)
        $0 = query
        if (debugState != 0) print "Line: " line ", State: " state ", Message: '" query "'"
        if (/^sqlQuery: result = /) {
            ki = $4
            #print "Success finished"
            if (success == "true") {
                if (json == "true") {
                    printf "    "
                    if (first == "true") {
                        first = "false"
                        printf " "
                    } else {
                        printf ","
                    }
                    printf "{ \"timestamp\": \"%s\", \"id\": %s, \"imsi\": \"%s\" }\n",
                        timestamp, id, imsi
                } else {
                    printf "%-32s %-9s %-20s\n", timestamp, id, imsi
                }
            }
            state=0
        } else {
            # stay in the state - a spurrious line
        }
        break


    default:
        print "Unknown state " state
        state=0
        break
    }
}
END {
    end=systime()
    #print "End @ " end
    if (json == "true") {
        print "    ,{ \"lines\": " line ", \"Duration\" : " end - start " }"
        print "}"
    } else
    {
        print "Lines: " line
        print "Duration: " end - start " seconds"
    }
}
