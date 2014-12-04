#!/bin/bash

# Copyright 2014 Range Networks, Inc.
#
#    This program is free software: you can redistribute it and/or modify
#    it under the terms of the GNU Affero General Public License as published by
#    the Free Software Foundation, either version 3 of the License, or
#    (at your option) any later version.

#    This program is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU Affero General Public License for more details.
#
#    You should have received a copy of the GNU Affero General Public License
#    along with this program.  If not, see <http://www.gnu.org/licenses/>.

set -e
WEBGROUP=www-data
CONFIG_DB='/etc/OpenBTS/sipauthserve.db'
SR_PATH='/var/lib/asterisk/sqlite3dir'
SR_DB='sqlite3.db'

DATE=$(date +'%Y-%m-%dT%H-%M')	# was $(date --rfc-3339='date')

backup()
{
	CONFIG_BACKUP="/etc/OpenBTS/sipauthserve.sql-${DATE}"
	if [ -f ${CONFIG_DB} ] && [ ! -e ${CONFIG_BACKUP} ]; then
		echo "backing up SAS configuration DB..."
		sqlite3 ${CONFIG_DB} ".dump" > ${CONFIG_BACKUP}
		echo "done"
	fi

	SR_BACKUP="sqlite3.sql-${DATE}"
	if [ -f ${SR_PATH}/${SR_DB} ] && [ ! -e ${SR_BACKUP} ]; then
		echo "backing up Subscriber Registry DB..."
		sqlite3 ${SR_PATH}/${SR_DB} ".dump" > ${SR_PATH}/${SR_BACKUP}
		echo "done"
	fi
}

updatedb() {
	HASPREPAID=`sqlite3 ${SR_PATH}/${SR_DB} "PRAGMA table_info(sip_buddies)" | grep prepaid || true`
	if [ "$HASPREPAID" = "" ]; then
		sqlite3 ${SR_PATH}/${SR_DB} "alter table sip_buddies add prepaid int(1) DEFAULT 0 not null"
	fi

	HASBALANCE=`sqlite3 ${SR_PATH}/${SR_DB} "PRAGMA table_info(sip_buddies)" | grep account_balance || true`
	if [ "$HASBALANCE" = "" ]; then
		sqlite3 ${SR_PATH}/${SR_DB} "alter table sip_buddies add column account_balance int(9) default 0"
	fi

	sqlite3 ${SR_PATH}/${SR_DB} "create table if not exists rates (service varchar(30) unique not null, rate integer not null)"
	sqlite3 ${SR_PATH}/${SR_DB} "insert or ignore into 'rates' values ('in-network-SMS', 10)"
	sqlite3 ${SR_PATH}/${SR_DB} "insert or ignore into 'rates' values ('out-of-network-SMS', 20)"
	sqlite3 ${SR_PATH}/${SR_DB} "insert or ignore into 'rates' values ('in-network-call', 1)"
	sqlite3 ${SR_PATH}/${SR_DB} "insert or ignore into 'rates' values ('out-of-network-call', 1)"
}

config() {
	# setup configuration db
	sqlite3 ${CONFIG_DB} ".read /etc/OpenBTS/sipauthserve.example.sql" > /dev/null 2>&1

	set +e
	if [ -e ${SR_PATH}/${SR_DB} ]; then
		if [ "$(sqlite3 ${SR_PATH}/${SR_DB} '.tables sip_buddies' | wc -l)" = "1" ]; then
			updatedb
		else
			echo "Unknown Subscriber Registry database format, exiting..."
			exit 1
		fi
	fi
	set -e

	chown -R asterisk:${WEBGROUP} ${SR_PATH}
	chmod -R ug+rw ${SR_PATH}
	chmod -R o-w ${SR_PATH}
}

backup
config
