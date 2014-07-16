--
-- This file was generated using: ./sipauthserve --gensql
-- binary version: release 5.0.0-prealpha+a5121d81ec CommonLibs:7e5af6cff7 built 2014-07-08T20:45:15 
--
-- Future changes should not be put in this file directly but
-- rather in the program's ConfigurationKey schema.
--
PRAGMA foreign_keys=OFF;
BEGIN TRANSACTION;
CREATE TABLE IF NOT EXISTS CONFIG ( KEYSTRING TEXT UNIQUE NOT NULL, VALUESTRING TEXT, STATIC INTEGER DEFAULT 0, OPTIONAL INTEGER DEFAULT 0, COMMENTS TEXT DEFAULT '');
INSERT OR IGNORE INTO "CONFIG" VALUES('Control.NumSQLTries','3',0,0,'Number of times to retry SQL queries before declaring a database access failure.');
INSERT OR IGNORE INTO "CONFIG" VALUES('Log.Alarms.Max','20',0,0,'Maximum number of alarms to remember inside the application.');
INSERT OR IGNORE INTO "CONFIG" VALUES('Log.File','',0,0,'Path to use for textfile based logging.  By default, this feature is disabled.  To enable, specify an absolute path to the file you wish to use, eg: /tmp/my-debug.log.  To disable again, execute "unconfig Log.File".');
INSERT OR IGNORE INTO "CONFIG" VALUES('Log.Level','NOTICE',0,0,'Default logging level when no other level is defined for a file.');
INSERT OR IGNORE INTO "CONFIG" VALUES('SubscriberRegistry.A3A8','/OpenBTS/comp128',0,0,'Path to the program that implements the A3/A8 algorithm.');
INSERT OR IGNORE INTO "CONFIG" VALUES('SubscriberRegistry.Port','5064',0,0,'Port used by the SIP Authentication Server.');
INSERT OR IGNORE INTO "CONFIG" VALUES('SubscriberRegistry.db','/var/lib/asterisk/sqlite3dir/sqlite3.db',0,0,'The location of the sqlite3 database holding the subscriber registry.');
COMMIT;


