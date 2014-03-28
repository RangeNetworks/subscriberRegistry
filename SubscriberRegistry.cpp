/*
* Copyright 2011 Kestrel Signal Processing, Inc.
* Copyright 2011, 2012, 2013, 2014 Range Networks, Inc.
*
* This software is distributed under the terms of the GNU Affero Public License.
* See the COPYING file in the main directory for details.
*
* This use of this software may be subject to additional restrictions.
* See the LEGAL file in the main directory for details.

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU Affero General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU Affero General Public License for more details.

	You should have received a copy of the GNU Affero General Public License
	along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#include "SubscriberRegistry.h"
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "sqlite3.h"
#include <iostream>
#include <sstream>
#include <fstream>
#include <algorithm> // for sort()
#include <Configuration.h>

extern ConfigurationTable gConfig;


using namespace std;

static const char* createRRLPTable = {
    "CREATE TABLE IF NOT EXISTS RRLP ("
		"id				INTEGER PRIMARY KEY, "
		"name           VARCHAR(80) not null, "
		"latitude       real not null, "
		"longitude      real not null, "
		"error          real not null, "
		"time           text not null "
    ")"
};

static const char* createDDTable = {
    "CREATE TABLE IF NOT EXISTS DIALDATA_TABLE ("
		"id				INTEGER PRIMARY KEY, "
		"exten           VARCHAR(40)     NOT NULL        DEFAULT '', "
		"dial			VARCHAR(128)    NOT NULL        DEFAULT '' "
    ")"
};

static const char* createRateTable = {
"create table if not exists rates (service varchar(30) not null, rate integer not null)"
};

static const char* createSBTable = {
    "CREATE TABLE IF NOT EXISTS SIP_BUDDIES ("
		"id                    integer primary key, "
		"name                  VARCHAR(80) not null, "
		"context               VARCHAR(80), "
		"callingpres           VARCHAR(30) DEFAULT 'allowed_not_screened', "
		"deny                  VARCHAR(95), "
		"permit                VARCHAR(95), "
		"secret                VARCHAR(80), "
		"md5secret             VARCHAR(80), "
		"remotesecret          VARCHAR(250), "
		"transport             VARCHAR(10), "
		"host                  VARCHAR(31) default '' not null, "
		"nat                   VARCHAR(5) DEFAULT 'no' not null, "
		"type                  VARCHAR(10) DEFAULT 'friend' not null, "
		"accountcode           VARCHAR(20), "
		"amaflags              VARCHAR(13), "
		"callgroup             VARCHAR(10), "
		"callerid              VARCHAR(80), "
		"defaultip             VARCHAR(40) DEFAULT '0.0.0.0', "
		"dtmfmode              VARCHAR(7) DEFAULT 'info', "
		"fromuser              VARCHAR(80), "
		"fromdomain            VARCHAR(80), "
		"insecure              VARCHAR(4), "
		"language              CHAR(2), "
		"mailbox               VARCHAR(50), "
		"pickupgroup           VARCHAR(10), "
		"qualify               CHAR(3), "
		"regexten              VARCHAR(80), "
		"rtptimeout            CHAR(3), "
		"rtpholdtimeout        CHAR(3), "
		"setvar                VARCHAR(100), "
		"disallow              VARCHAR(100) DEFAULT 'all', "
		"allow                 VARCHAR(100) DEFAULT 'gsm' not null, "
		"fullcontact           VARCHAR(80), "
		"ipaddr                VARCHAR(45), "
		"port                  int(5) DEFAULT 5062, "
		"username              VARCHAR(80), "
		"defaultuser           VARCHAR(80), "
		"subscribecontext      VARCHAR(80), "
		"directmedia           VARCHAR(3), "
		"trustrpid             VARCHAR(3), "
		"sendrpid              VARCHAR(3), "
		"progressinband        VARCHAR(5), "
		"promiscredir          VARCHAR(3), "
		"useclientcode         VARCHAR(3), "
		"callcounter           VARCHAR(3), "
		"busylevel             int(11) default 1, "
		"allowoverlap          VARCHAR(3) DEFAULT 'no', "
		"allowsubscribe        VARCHAR(3) DEFAULT 'no', "
		"allowtransfer         VARCHAR(3) DEFAULT 'no', "
		"ignoresdpversion      VARCHAR(3) DEFAULT 'no', "
		"template              VARCHAR(100), "
		"videosupport          VARCHAR(6) DEFAULT 'no', "
		"maxcallbitrate        int(11), "
		"rfc2833compensate     VARCHAR(3) DEFAULT 'yes', "
		"'session-timers'      VARCHAR(10) DEFAULT 'accept', "
		"'session-expires'     int(6) DEFAULT 1800, "
		"'session-minse'       int(6) DEFAULT 90, "
		"'session-refresher'   VARCHAR(3) DEFAULT 'uas', "
		"t38pt_usertpsource    VARCHAR(3), "
		"outboundproxy         VARCHAR(250), "
		"callbackextension     VARCHAR(250), "
		"registertrying        VARCHAR(3) DEFAULT 'yes', "
		"timert1               int(6) DEFAULT 500, "
		"timerb                int(9), "
		"qualifyfreq           int(6) DEFAULT 120, "
		"contactpermit         VARCHAR(250), "
		"contactdeny           VARCHAR(250), "
		"lastms                int(11) DEFAULT 0 not null, "
		"regserver             VARCHAR(100), "
		"regseconds            int(11) DEFAULT 0 not null, "
		"useragent             VARCHAR(100), "
		"cancallforward        CHAR(3) DEFAULT 'yes' not null, "
		"canreinvite           CHAR(3) DEFAULT 'no' not null, "
		"mask                  VARCHAR(95), "
		"musiconhold           VARCHAR(100), "
		"restrictcid           CHAR(3), "
		"calllimit             int(5) default 1, "
		"WhiteListFlag         timestamp not null default '0', "
		"WhiteListCode         varchar(8) not null default '0', "
		"rand                  varchar(33) default '', "
		"sres                  varchar(33) default '', "
		"ki                    varchar(33) default '', "
		"kc                    varchar(33) default '', "
		"prepaid               int(1) DEFAULT 0 not null, "	// flag to indicate prepaid customer
		"account_balance       int(9) default 0 not null, "	// current account, neg is debt, pos is credit
		"RRLPSupported         int(1) default 1 not null, "
  		"hardware              VARCHAR(20), "
		"regTime               INTEGER default 0 NOT NULL, " // Unix time of most recent registration
		"a3_a8                 varchar(45) default NULL"
    ")"
};

static const char* createMEMSBTable = {
    "CREATE TABLE IF NOT EXISTS memcache.mem_sip_buddies ("
		"username              varchar(80) primary key, "
		"ipaddr                varchar(45), "
		"port                  int(5) default 5062, "
		"rand                  varchar(33) default '', "
		"sres                  varchar(33) default ''"
    ")"
};

int SubscriberRegistry::init()
{
	string ldb = gConfig.getStr("SubscriberRegistry.db");
	size_t p = ldb.find_last_of('/');
	if (p == string::npos) {
		LOG(EMERG) << "SubscriberRegistry.db not in a directory?";
		mDB = NULL;
		return 1;
	}
	string dir = ldb.substr(0, p);
	struct stat buf;
	if (stat(dir.c_str(), &buf)) {
		LOG(EMERG) << dir << " does not exist";
		mDB = NULL;
		return 1;
	}
	mNumSQLTries=gConfig.getNum("Control.NumSQLTries"); 
	int rc = sqlite3_open(ldb.c_str(),&mDB);
	if (rc) {
		LOG(EMERG) << "Cannot open SubscriberRegistry database: " << ldb << " error: " << sqlite3_errmsg(mDB);
		sqlite3_close(mDB);
		mDB = NULL;
		return 1;
	}
	if (!sqlite3_command(mDB,createRRLPTable,mNumSQLTries)) {
		LOG(EMERG) << "Cannot create RRLP table";
		return 1;
	}
	if (!sqlite3_command(mDB,createDDTable,mNumSQLTries)) {
		LOG(EMERG) << "Cannot create DIALDATA_TABLE table";
		return 1;
	}
	if (!sqlite3_command(mDB,createRateTable,mNumSQLTries)) {
		LOG(EMERG) << "Cannot create rate table";
		return 1;
	}
	if (!sqlite3_command(mDB,createSBTable,mNumSQLTries)) {
		LOG(EMERG) << "Cannot create SIP_BUDDIES table";
		return 1;
	}
	// Set high-concurrency WAL mode.
	if (!sqlite3_command(mDB,enableWAL,mNumSQLTries)) {
		LOG(EMERG) << "Cannot enable WAL mode on database at " << ldb << ", error message: " << sqlite3_errmsg(mDB);
	}

#ifndef SR_API_ONLY
	// memory based sip_buddies table
	if (!sqlite3_command(mDB,"attach database ':memory:' as memcache",mNumSQLTries)) {
		LOG(EMERG) << "Cannot create memcache database";
		return 1;
	}
	if (!sqlite3_command(mDB,createMEMSBTable,mNumSQLTries)) {
		LOG(EMERG) << "Cannot create memcache mem_sip_buddies table";
		return 1;
	}
	if (!sqlite3_command(mDB,"INSERT INTO mem_sip_buddies SELECT username, ipaddr, port, rand, sres FROM sip_buddies",mNumSQLTries)) {
		LOG(EMERG) << "Cannot populate mem_sip_buddies table with disk contents";
		return 1;
	}
	if (!sqlite3_command(mDB,"ALTER TABLE mem_sip_buddies ADD dirty INTEGER DEFAULT 0",mNumSQLTries)) {
		LOG(EMERG) << "Cannot add dirty column to mem_sip_buddies table";
		return 1;
	}

	// Start the sync thread
	mSyncer.start((void*(*)(void*))subscriberRegistrySyncer,NULL);
#endif

	return 0;
}

string SubscriberRegistry::getResultsAsString(string query)
{
	sqlite3_stmt *stmt;

	if (sqlite3_prepare_statement(mDB, &stmt, query.c_str(), 5)) {
		std::cout << " - sqlite3_prepare_statement() failed" << std::endl;
 		return "";
	}

	int state;
	int row = 0;
	int col;
	int colMax = sqlite3_column_count(stmt);
	const char* colName;
	const char* colText;
	ostringstream colNames;
	ostringstream colTexts;
	while (1) {
		state = sqlite3_step(stmt);
		if (state == SQLITE_ROW) {
			for (col = 0; col < colMax; col++) {
				colName = (const char*)sqlite3_column_name(stmt, col);
				colText = (const char*)sqlite3_column_text(stmt, col);

				if (row == 0) {
					colNames << colName << " ";
				}
				if (colText == NULL) {
					colTexts << "NULL" << " ";
				} else {
					colTexts << colText << " ";
				}
			}
		} else if (state == SQLITE_DONE) {
			break;
		// TODO : handle more SQLITE_FAILURECODES here
		} else {
			return " - failed in while(1) in } else {";
		}
		colTexts << std::endl;
		row++;
	}
	sqlite3_finalize(stmt);

	ostringstream s;
	string title = colNames.str();
	if (title.length()) {
		s << title << std::endl;
		s << colTexts << std::endl;
	} else {
		s << " - none found" << std::endl;
	}

	return s.str();
}

vector<string> SubscriberRegistry::getTableColumns(string tableName)
{
	vector<string> colNames;
	stringstream tmp;
	tmp << "select * from " << tableName << " limit 1";

	sqlite3_stmt *stmt;

	if (sqlite3_prepare_statement(mDB, &stmt, tmp.str().c_str(), 5)) {
		std::cout << " - getTableColumns() - sqlite3_prepare_statement() failed" << std::endl;
 		return colNames;
	}
	tmp.str("");

	int state;
	int col;
	int colMax = sqlite3_column_count(stmt);
	while (1) {
		state = sqlite3_step(stmt);
		if (state == SQLITE_ROW) {
			for (col = 0; col < colMax; col++) {
				tmp << (const char*)sqlite3_column_name(stmt, col);
				colNames.push_back(tmp.str());
				tmp.str("");
			}
		} else if (state == SQLITE_DONE) {
			break;
		// TODO : handle more SQLITE_FAILURECODES here
		} else {
			std::cout << " - getTableColumns() - failed in while(1) in } else {" << std::endl;
	 		return colNames;
		}
	}
	sqlite3_finalize(stmt);

	return colNames;
}

#ifndef SR_API_ONLY
string SubscriberRegistry::generateSyncToDiskQuery()
{
	vector<string> columns = getTableColumns("sip_buddies");
	if (!columns.size()) {
		LOG(DEBUG) << "sip_buddies is empty";
		return "";
	}

	sort(columns.begin(), columns.end());
	std::stringstream ss;
	unsigned i;

	ss << "replace into sip_buddies (ipaddr, port, rand, sres, ";

	for (i = 0; i < columns.size(); i++) {
		if (columns[i].compare("ipaddr") == 0 || columns[i].compare("port") == 0 ||
			columns[i].compare("rand") == 0 || columns[i].compare("sres") == 0) {
			continue;
		}
		if (i != 0) {
			ss << ", ";
		}

		if (columns[i].find("-") != std::string::npos) {
			ss << "'" << columns[i] << "'";
		} else {
			ss << columns[i];
		}
	}

	ss << ") select src.ipaddr, src.port, src.rand, src.sres, ";

	for (i = 0; i < columns.size(); i++) {
		if (columns[i].compare("ipaddr") == 0 || columns[i].compare("port") == 0 ||
			columns[i].compare("rand") == 0 || columns[i].compare("sres") == 0) {
			continue;
		}
		if (i != 0) {
			ss << ", ";
		}

		ss << "dest.";
		if (columns[i].find("-") != std::string::npos) {
			ss << "'" << columns[i] << "'";
		} else {
			ss << columns[i];
		}
	}

	ss << " from mem_sip_buddies src ";
	ss << "inner join sip_buddies dest on src.username = dest.username ";
	ss << "where src.dirty = \"1\"";

	return ss.str();
}

bool SubscriberRegistry::syncMemoryDB()
{
	bool ret = true;
	string syncToDisk = generateSyncToDiskQuery();

	string cleanDirty = "update mem_sip_buddies set dirty = \"0\"";

	string syncFromDiskAddNewEntries = "INSERT INTO mem_sip_buddies "
		"(username, ipaddr, port, rand, sres, dirty) "
		"SELECT sip_buddies.username, sip_buddies.ipaddr, sip_buddies.port, sip_buddies.rand, sip_buddies.sres, '0' FROM sip_buddies "
		"LEFT OUTER JOIN mem_sip_buddies ON (mem_sip_buddies.username=sip_buddies.username) "
		"WHERE mem_sip_buddies.username IS NULL";

	string syncFromDiskDeleteOldEntries = "DELETE FROM mem_sip_buddies "
		"WHERE username IN "
		"(SELECT mem_sip_buddies.username FROM mem_sip_buddies"
		" LEFT JOIN sip_buddies ON mem_sip_buddies.username=sip_buddies.username"
		" WHERE sip_buddies.username IS NULL"
		")";

	Timeval timer;
	mLock.lock();

	// this string is only non-empty if there are entries in sip_buddies
	if (syncToDisk.length()) {
		// sync dirty entries to disk
		if (sqlUpdate(syncToDisk.c_str()) == FAILURE) {
			LOG(ERR) << "syncToDisk failed";
		} else {
			// if it succeeds, mark these entries as clean again
			LOG(INFO) << "syncToDisk succeeded";
			if (sqlUpdate(cleanDirty.c_str()) == FAILURE) {
				LOG(ERR) << "cleanDirty failed";
			} else {
				LOG(INFO) << "cleanDirty succeeded";
			}
		}

		// sync new rows from the disk into memory
		if (sqlUpdate(syncFromDiskAddNewEntries.c_str()) == FAILURE) {
			LOG(ERR) << "syncFromDiskAddNewEntries failed";
		} else {
			LOG(INFO) << "syncFromDiskAddNewEntries succeeded";
		}
	}

	// delete old entries from memory no longer found on the disk
	if (sqlUpdate(syncFromDiskDeleteOldEntries.c_str()) == FAILURE) {
		LOG(ERR) << "syncFromDiskDeleteOldEntries failed";
	} else {
		LOG(INFO) << "syncFromDiskDeleteOldEntries succeeded";
	}

	mLock.unlock();
	LOG(INFO) << "syncMemoryDB() locked the db for " << timer.elapsed() << "ms";

	return ret;
}
#endif

SubscriberRegistry::~SubscriberRegistry()
{
	if (mDB) sqlite3_close(mDB);
}



SubscriberRegistry::Status SubscriberRegistry::sqlLocal(const char *query, char **resultptr)
{
	LOG(INFO) << query;

	if (!resultptr) {
		if (!sqlite3_command(db(), query, mNumSQLTries)) return FAILURE;
		return SUCCESS;
	}

	sqlite3_stmt *stmt;
	if (sqlite3_prepare_statement(db(), &stmt, query, mNumSQLTries)) {
		LOG(ERR) << "sqlite3_prepare_statement problem with query \"" << query << "\"";
		return FAILURE;
	}
	int src = sqlite3_run_query(db(), stmt, mNumSQLTries);
	if (src != SQLITE_ROW) {
		sqlite3_finalize(stmt);
		return FAILURE;
	}
	char *column = (char*)sqlite3_column_text(stmt, 0);
	if (!column) {
		LOG(ERR) << "Subscriber registry returned a NULL column.";
		sqlite3_finalize(stmt);
		return FAILURE;
	}
	*resultptr = strdup(column);
	sqlite3_finalize(stmt);
	return SUCCESS;
}


char *SubscriberRegistry::sqlQuery(const char *unknownColumn, const char *table, const char *knownColumn, const char *knownValue)
{
	char *result = NULL;
	SubscriberRegistry::Status st;
	ostringstream os;
	os << "select " << unknownColumn << " from " << table << " where " << knownColumn << " = \"" << knownValue << "\"";
	// try to find locally
	st = sqlLocal(os.str().c_str(), &result);
	if ((st == SUCCESS) && result) {
		// got it.  return it.
		LOG(INFO) << "result = " << result;
		return result;
	}
	// didn't find locally
	LOG(INFO) << "not found: " << os.str();
	return NULL;
}


/*
 * Check for two of the same values for a single unknown
 */
char *SubscriberRegistry::sqlQuery2(const char *unknownColumn, const char *table, const char *knownColumn, const char *knownValue1, const char *knownValue2)
{
	char *result = NULL;
	SubscriberRegistry::Status st;
	ostringstream os;
	// select knownValue from table where knownColumn IN ('knownValue1', 'knownValue2')
	os << "select " << unknownColumn << " from " << table << " where " << knownColumn << " IN (\'"
			<< knownValue1 << "\', " << "\'" << knownValue2 << "\')";
	//LOG(INFO) << "QUERY STRING = " << os.str().c_str();

	// try to find locally
	st = sqlLocal(os.str().c_str(), &result);
	if ((st == SUCCESS) && result) {
		// got it.  return it.
		LOG(INFO) << "result = " << result;
		return result;
	}
	// didn't find locally
	LOG(INFO) << "not found: " << os.str();
	return NULL;
}


SubscriberRegistry::Status SubscriberRegistry::sqlUpdate(const char *stmt)
{
	LOG(INFO) << stmt;
 	return sqlLocal(stmt, NULL);
}

string SubscriberRegistry::imsiGet(string imsi, string key)
{
	string name = imsi.substr(0,4) == "IMSI" ? imsi : "IMSI" + imsi;
	char *st = sqlQuery(key.c_str(), "sip_buddies", "username", name.c_str());
	if (!st) {
		LOG(INFO) << "cannot get key " << key << " for username " << name;
		return "";
	}
	return st;
}

#ifndef SR_API_ONLY
bool SubscriberRegistry::imsiSet(string imsi, string key, string value)
{
	string name = imsi.substr(0,4) == "IMSI" ? imsi : "IMSI" + imsi;
	ostringstream os;
	os << "update mem_sip_buddies set dirty = \"1\", " << key << " = \"" << value << "\" where username = \"" << name << "\"";

	mLock.lock();
	SubscriberRegistry::Status ret = sqlUpdate(os.str().c_str());
	mLock.unlock();

	return ret == FAILURE;
}

bool SubscriberRegistry::imsiSet(string imsi, string key1, string value1, string key2, string value2)
{
	string name = imsi.substr(0,4) == "IMSI" ? imsi : "IMSI" + imsi;
	ostringstream os;
	os << "update mem_sip_buddies set dirty = \"1\", " << key1 << " = \"" << value1 << "\"," << key2 << " = \"" << value2 << "\" where username = \"" << name << "\"";

	mLock.lock();
	SubscriberRegistry::Status ret = sqlUpdate(os.str().c_str());
	mLock.unlock();

	return ret == FAILURE;
}
#endif

/*
 * Get IMSI from phone number
 * Should be able to get rid of this one
*/
char *SubscriberRegistry::getIMSI(const char *ISDN)
{
	if (!ISDN) {
		LOG(WARNING) << "SubscriberRegistry::getIMSI attempting lookup of NULL ISDN";
		return NULL;
	}
	LOG(INFO) << "getIMSI(" << ISDN << ")";
	return sqlQuery("dial", "dialdata_table", "exten", ISDN);
}


/*
 * This version handle phone number with or without a plus at the beginning
 * This should work for all uses of getIMSI
 *
 */
char *SubscriberRegistry::getIMSI2(const char *ISDN)
{
	if (!ISDN) {
		LOG(WARNING) << "SubscriberRegistry::getIMSI2 attempting lookup of NULL ISDN";
		return NULL;
	}
	LOG(INFO) << "getIMSI2(" << ISDN << ")";
	char* str2;
	char localstr[50];
	if (ISDN[0] == '+') {
		str2 =  (char *) &ISDN[1];  // ISDN has plus remove plus for str2
	} else  {
		// ISDN no plus  add plus to str2
		strcpy(&localstr[0], "+");
		strncat(localstr, ISDN, sizeof(localstr)-1);
		localstr[sizeof(localstr)-1] = 0;
		str2 = localstr;
	}
	// Two strings one with plus one without
	return sqlQuery2("dial", "dialdata_table", "exten", ISDN, (const char*) str2);
}


char *SubscriberRegistry::getCLIDLocal(const char* IMSI)
{
	if (!IMSI) {
		LOG(WARNING) << "SubscriberRegistry::getCLIDLocal attempting lookup of NULL IMSI";
		return NULL;
	}
	LOG(INFO) << "getCLIDLocal(" << IMSI << ")";
	return sqlQuery("callerid", "sip_buddies", "username", IMSI);
}



char *SubscriberRegistry::getCLIDGlobal(const char* IMSI)
{
	if (!IMSI) {
		LOG(WARNING) << "SubscriberRegistry::getCLIDGlobal attempting lookup of NULL IMSI";
		return NULL;
	}
	LOG(INFO) << "getCLIDGlobal(" << IMSI << ")";
	return sqlQuery("callerid", "sip_buddies", "username", IMSI);
}



char *SubscriberRegistry::getRegistrationIP(const char* IMSI)
{
	if (!IMSI) {
		LOG(WARNING) << "SubscriberRegistry::getRegistrationIP attempting lookup of NULL IMSI";
		return NULL;
	}
	LOG(INFO) << "getRegistrationIP(" << IMSI << ")";
	return sqlQuery("ipaddr", "sip_buddies", "username", IMSI);
}



SubscriberRegistry::Status SubscriberRegistry::setRegTime(const char* IMSI)
{
	if (!IMSI) {
		LOG(WARNING) << "SubscriberRegistry::setRegTime attempting set for NULL IMSI";
		return FAILURE;
	}
	unsigned now = (unsigned)time(NULL);
	ostringstream os;
	os << "update sip_buddies set regTime = " << now  << " where username = " << '"' << IMSI << '"';
	return sqlUpdate(os.str().c_str());
}



SubscriberRegistry::Status SubscriberRegistry::addUser(const char* IMSI, const char* CLID)
{
	if (!IMSI) {
		LOG(WARNING) << "SubscriberRegistry::addUser attempting add of NULL IMSI";
		return FAILURE;
	}
	if (!CLID) {
		LOG(WARNING) << "SubscriberRegistry::addUser attempting add of NULL CLID";
		return FAILURE;
	}
	LOG(INFO) << "addUser(" << IMSI << "," << CLID << ")";
	ostringstream os;
	os << "insert into sip_buddies (name, username, type, context, host, callerid, canreinvite, allow, dtmfmode, ipaddr, port) values (";
	os << "\"" << IMSI << "\"";
	os << ",";
	os << "\"" << IMSI << "\"";
	os << ",";
	os << "\"" << "friend" << "\"";
	os << ",";
	os << "\"" << "phones" << "\"";
	os << ",";
	os << "\"" << "dynamic" << "\"";
	os << ",";
	os << "\"" << CLID << "\"";
	os << ",";
	os << "\"" << "no" << "\"";
	os << ",";
	os << "\"" << "gsm" << "\"";
	os << ",";
	os << "\"" << "info" << "\"";
	os << ",";
	os << "\"" << "127.0.0.1" << "\"";
	os << ",";
	os << "\"" << "5062" << "\"";
	os << ")";
	os << ";";
	SubscriberRegistry::Status st = sqlUpdate(os.str().c_str());
	ostringstream os2;
	os2 << "insert into dialdata_table (exten, dial) values (";
	os2 << "\"" << CLID << "\"";
	os2 << ",";
	os2 << "\"" << IMSI << "\"";
	os2 << ")";
	SubscriberRegistry::Status st2 = sqlUpdate(os2.str().c_str());
	return st == SUCCESS && st2 == SUCCESS ? SUCCESS : FAILURE;
}



char *SubscriberRegistry::mapCLIDGlobal(const char *local)
{
	if (!local) {
		LOG(WARNING) << "SubscriberRegistry::mapCLIDGlobal attempting lookup of NULL local";
		return NULL;
	}
	LOG(INFO) << "mapCLIDGlobal(" << local << ")";
	char *IMSI = getIMSI2(local);
	if (!IMSI) return NULL;
	char *global = getCLIDGlobal(IMSI);
	free(IMSI);
	return global;
}

SubscriberRegistry::Status SubscriberRegistry::RRLPUpdate(string name, string lat, string lon, string err){
	ostringstream os;
	os << "insert into RRLP (name, latitude, longitude, error, time) values (" <<
	  '"' << name << '"' << "," <<
	  lat << "," <<
	  lon << "," <<
	  err << "," <<
	  "datetime('now')"
	  ")";
	LOG(INFO) << os.str();
	return sqlUpdate(os.str().c_str());
}

#ifndef SR_API_ONLY
extern SubscriberRegistry gSubscriberRegistry;
void* subscriberRegistrySyncer(void*)
{
	while (true) {
		sleep(15);
		gSubscriberRegistry.syncMemoryDB();
	}

	return NULL;
}
#endif

// vim: ts=4 sw=4
