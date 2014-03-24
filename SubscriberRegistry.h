/*
* Copyright 2011 Kestrel Signal Processing, Inc.
* Copyright 2011, 2012 Range Networks, Inc.
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

#ifndef SubscriberRegistry_H
#define SubscriberRegistry_H

#include <map>
#include <stdlib.h>
#include <Logger.h>
#include <Timeval.h>
#include <Threads.h>
#include <map>
#include <string>
#include "sqlite3.h"

using namespace std;

class SubscriberRegistry {

	private:

	sqlite3 *mDB;			///< database connection
	unsigned mNumSQLTries;		///< Number of times to try an sqlite command before giving up.

#ifndef SR_API_ONLY
	mutable Mutex mLock;	///< control for multithreaded read/write access to the memory based sip_buddies table
	Thread mSyncer;			///< thread responsible for synchronizing the memory and disk based sip_buddies tables
#endif

	public:

	~SubscriberRegistry();

	/**
			Initialize the subscriber registry using parameters from gConfig.
			@return 0 if the database was successfully opened and initialized; 1 otherwise
	*/
	int init();

	typedef enum {
		SUCCESS=0,		///< operation successful
		FAILURE=1,		///< operation not successful
		DELAYED=2,		///< operation successful, but effect delayed
		TRYAGAIN=3		///< operation not attempted, try again later
	} Status;


	sqlite3 *db()
	{
		return mDB;
	}


	/**
		Grab the memory based sqlite db as a table string
	*/
	string getResultsAsString(string query);

	/**
		Grab the columns from a table
	*/
	vector<string> getTableColumns(string tableName);

#ifndef SR_API_ONLY
	/**
		Generate the SQL query which is used to sync the in-memory database to disk
		to account for differences in database schemas.
	*/
	string generateSyncToDiskQuery();

	/**
		Sync the in-memory sip_buddies table to disk.
	*/
	bool syncMemoryDB();
#endif

	/**
		Resolve an ISDN or other numeric address to an IMSI.
		@param ISDN Any numeric address, E.164, local extension, etc.
		@return A C-string to be freed by the caller,
			NULL if the ISDN cannot be resolved.
	*/
	char* getIMSI(const char* ISDN);

	/**
		Given an IMSI, return the local CLID, which should be a numeric address.
		@param IMSI The subscriber IMSI.
		@return A C-string to be freed by the caller,
			NULL if the IMSI isn't found.
	*/
	char* getCLIDLocal(const char* IMSI);

	/**
		Given an IMSI, return the global CLID, which should be a numeric address.
		@param IMSI The subscriber IMSI.
		@return A C-string to be freed by the caller,
			NULL if the IMSI isn't found.
	*/
	char* getCLIDGlobal(const char* IMSI);

	/**
		Given an IMSI, return the IP address of the most recent registration.
		@param IMSI The subscriber IMSI
		@return A C-string to be freed by the caller, "111.222.333.444:port",
			NULL if the ISMI isn't registered.
	*/
	char* getRegistrationIP(const char* IMSI);

	/**
		Get a subscriber's property.
		@param imsi imsi of the subscriber
		@param key name of the property
	*/
	string imsiGet(string imsi, string key);

#ifndef SR_API_ONLY
	/**
		Set a subscriber's property.
		@param imsi imsi of the subscriber
		@param key name of the property
		@param value value of the property
	*/
	bool imsiSet(string imsi, string key, string value);

	/**
		Set a subscriber's property.
		@param imsi imsi of the subscriber
		@param key1 name of the property
		@param value1 value of the property
		@param key2 name of the property
		@param value2 value of the property
	*/
	bool imsiSet(string imsi, string key1, string value1, string key2, string value2);
#endif

	/**
		Add a new user to the SubscriberRegistry.
		@param IMSI The user's IMSI or SIP username.
		@param CLID The user's local CLID.
	*/
	Status addUser(const char* IMSI, const char* CLID);


	/**
		Remove a user from the SubscriberRegistry.
		@param IMSI The user's IMSI or SIP username.
	*/
	Status removeUser(const char* IMSI);


	/**
		Set the current time as the time of the most recent registration for an IMSI.
		@param IMSI The user's IMSI or SIP username.
	*/
	Status setRegTime(const char* IMSI);


	char *mapCLIDGlobal(const char *local);


	/**
		Update the RRLP location for user
		@param name IMSI to be updated
		@param lat Latitude
		@param lon Longitude
		@param err Approximate Error
	*/
	Status RRLPUpdate(string name, string lat, string lon, string err);


	private:



	/**
		Run sql statments locally.
		@param stmt The sql statements.
		@param resultptr Set this to point to the result of executing the statements.
	*/
	Status sqlLocal(const char *stmt, char **resultptr);



	/**
		Run an sql query (select unknownColumn from table where knownColumn = knownValue).
		@param unknownColumn The column whose value you want.
		@param table The table to look in.
		@param knownColumn The column with the value you know.
		@param knownValue The known value of knownColumn.
	*/
	char *sqlQuery(const char *unknownColumn, const char *table, const char *knownColumn, const char *knownValue);



	/**
		Run an sql update.
		@param stmt The update statement.
	*/
	Status sqlUpdate(const char *stmt);


};

/** Periodically triggers SubscriberRegistry::syncMemoryDB(). */
void* subscriberRegistrySyncer(void*);


#endif

// vim: ts=4 sw=4
