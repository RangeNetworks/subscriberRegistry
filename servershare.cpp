/*
* Copyright 2011 Kestrel Signal Processing, Inc.
* Copyright 2011, 2014 Range Networks, Inc.
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


#include <iostream>
#include <sstream>
#include <fstream>
#include <cstdlib>
#include <Configuration.h>
#include <Utils.h>
#include <string.h>

#include "servershare.h"
#include "sqlite3.h"
#include "Logger.h"
#include "SubscriberRegistry.h"

using namespace std;


extern ConfigurationTable gConfig;

// just using this for the database access
extern SubscriberRegistry gSubscriberRegistry;



ConfigurationKeyMap getConfigurationKeys()
{
	ConfigurationKeyMap map;
	ConfigurationKey *tmp;

	tmp = new ConfigurationKey("SubscriberRegistry.A3A8","/OpenBTS/comp128",
		"",
		ConfigurationKey::CUSTOMERWARN,
		ConfigurationKey::FILEPATH,
		"",
		false,
		"Path to the program that implements the A3/A8 algorithm."
	);
	map[tmp->getName()] = *tmp;
	delete tmp;

	tmp = new ConfigurationKey("SubscriberRegistry.db","/var/lib/asterisk/sqlite3dir/sqlite3.db",
		"",
		ConfigurationKey::CUSTOMERWARN,
		ConfigurationKey::FILEPATH,
		"",
		false,
		"The location of the sqlite3 database holding the subscriber registry."
	);
	map[tmp->getName()] = *tmp;
	delete tmp;

	tmp = new ConfigurationKey("SubscriberRegistry.Port","5064",
		"",
		ConfigurationKey::CUSTOMERWARN,
		ConfigurationKey::PORT,
		"",
		false,
		"Port used by the SIP Authentication Server."
	);
	map[tmp->getName()] = *tmp;
	delete tmp;

	return map;
}

string soGenerateIt()
{
	ostringstream os;
	for (int i = 0; i < 32; i++) {
		// if rand() is too slow you can call it fewer times
		os << hex << (rand() & 0xf);
	}
	return os.str();
}

// generate a 128' random number
string generateRand(string imsi)
{
	string ki = gSubscriberRegistry.imsiGet(imsi, "ki");
	string ret;
	if (ki.length() != 0) {
		LOG(INFO) << "ki is known";
		// generate and return rand (clear any cached rand or sres)
		gSubscriberRegistry.imsiSet(imsi, "rand", "", "sres", "");
		ret = soGenerateIt();
	} else {
		string wRand = gSubscriberRegistry.imsiGet(imsi, "rand");
		if (wRand.length() != 0) {
			LOG(INFO) << "ki is unknown, rand is cached";
			// return cached rand
			ret = wRand;
		} else {
			LOG(INFO) << "ki is unknown, rand is not cached";
			// generate rand, cache rand, clear sres, and return rand
			wRand = soGenerateIt();
			gSubscriberRegistry.imsiSet(imsi, "rand", wRand, "sres", "");
			ret = wRand;
		}
	}
	return ret;
}

bool strEqual(string a, string b)
{
	return 0 == strcasecmp(a.c_str(), b.c_str());
}

bool sresEqual(string a, string b)
{
	stringstream ss1;
	stringstream ss2;
	uint32_t sres1 = 0xffffffff;
	uint32_t sres2 = 0xffffffff;

	if (a.empty() || b.empty())
		return false;

	ss1 << hex << a;
	ss2 << hex << b;

	ss1 >> sres1;
	ss2 >> sres2;

	LOG(DEBUG) << "sres1 = " << sres1;
	LOG(DEBUG) << "sres2 = " << sres2;

	return (sres1 == sres2);
}

bool randEqual(string a, string b)
{
	uint64_t rand1h = 0;
	uint64_t rand1l = 0;
	uint64_t rand2h = 0;
	uint64_t rand2l = 0;

	if (a.empty() || b.empty())
		return false;

	Utils::stringToUint(a, &rand1h, &rand1l);
	Utils::stringToUint(b, &rand2h, &rand2l);

	LOG(DEBUG) << "rand1h = " << rand1h << ", rand1l = " << rand1l;
	LOG(DEBUG) << "rand2h = " << rand2h << ", rand2l = " << rand2l;

	return (rand1h == rand2h) && (rand1l == rand2l);
}

// verify sres given rand and imsi's ki
// may set kc
// may cache sres and rand
bool authenticate(string imsi, string randx, string sres, string *kc)
{
	string ki = gSubscriberRegistry.imsiGet(imsi, "ki");
	bool ret;
	if (ki.length() == 0) {
		// Ki is unknown
		string sres2 = gSubscriberRegistry.imsiGet(imsi, "sres");
		if (sres2.length() == 0) {
			LOG(INFO) << "ki unknown, no upstream server, sres not cached";
			// first time - cache sres and rand so next time
			// correct cell phone will calc same sres from same rand
			gSubscriberRegistry.imsiSet(imsi, "sres", sres, "rand", randx);
			ret = true;
		} else {
			LOG(INFO) << "ki unknown, no upstream server, sres cached";
			// check against cached values of rand and sres
			string rand2 = gSubscriberRegistry.imsiGet(imsi, "rand");
			// TODO - on success, compute and return kc
			LOG(DEBUG) << "comparing " << sres << " to " << sres2 << " and " << randx << " to " << rand2;
			ret = sresEqual(sres, sres2) && randEqual(randx, rand2);
		}
	} else {
		LOG(INFO) << "ki known";
		// Ki is known, so do normal authentication
		ostringstream os;
		// per user value from subscriber registry
		string a3a8 = gSubscriberRegistry.imsiGet(imsi, "a3_a8");
		if (a3a8.length() == 0) {
			// config value is default
			a3a8 = gConfig.getStr("SubscriberRegistry.A3A8");
		}
		os << a3a8 << " 0x" << ki << " 0x" << randx;
		// must not put ki into the log
		// LOG(INFO) << "running " << os.str();
		FILE *f = popen(os.str().c_str(), "r");
		if (f == NULL) {
			LOG(CRIT) << "error: popen failed";
			return false;
		}
		char sres2[26];
		char *str = fgets(sres2, 26, f);
		if (str != NULL && strlen(str) == 25) str[24] = 0;
		if (str == NULL || strlen(str) != 24) {
			LOG(CRIT) << "error: popen result failed";
			return false;
		}
		int st = pclose(f);
		if (st == -1) {
			LOG(CRIT) << "error: pclose failed";
			return false;
		}
		// first 8 chars are SRES;  rest are Kc
		*kc = sres2+8;
		sres2[8] = 0;
		LOG(INFO) << "result = " << sres2;
		ret = sresEqual(sres, sres2);
	}
	LOG(INFO) << "returning = " << ret;
	return ret;
}

string join(string separator, vector<string> &strings)
{
	string result("");
	vector<string>::iterator it;
	for (it = strings.begin(); it != strings.end(); it++) {
		if (it != strings.begin()) result.append(separator);
		result.append(*it);
	}
	return result;
}

void split(char separator, string tosplit, vector<string> *fields)
{
	int p = 0;
	while (1) {
		size_t q = tosplit.find(separator, p);
		if (q == string::npos) {
			fields->push_back(tosplit.substr(p));
			break;
		}
		fields->push_back(tosplit.substr(p, q-p));
		p = q+1;
	}
}
