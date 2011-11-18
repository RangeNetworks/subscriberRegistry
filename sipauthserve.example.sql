PRAGMA foreign_keys=OFF;
BEGIN TRANSACTION;
CREATE TABLE CONFIG ( KEYSTRING TEXT UNIQUE NOT NULL, VALUESTRING TEXT, STATIC INTEGER DEFAULT 0, OPTIONAL INTEGER DEFAULT 0, COMMENTS TEXT DEFAULT '');
INSERT INTO "CONFIG" VALUES('Log.Alarms.Max','20',0,0,'Maximum number of alarms to remember inside the application.');
INSERT INTO "CONFIG" VALUES('Log.Level','WARNING',0,0,'Default logging level when no other level is defined for a file.');
INSERT INTO "CONFIG" VALUES('SubscriberRegistry.db','/var/lib/asterisk/sqlite3dir/sqlite3.db',0,0,'The location of the sqlite3 database holding the subscriber registry.');
INSERT INTO "CONFIG" VALUES('SubscriberRegistry.Port','5064',0,0,'The port for the subscriber registry. Static.');
COMMIT;
