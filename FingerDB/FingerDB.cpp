#include "FingerDB.h"
#include <FS.h>

// Признак свободного блока - нулевой байт имени равен нолю

const char __dbPath[] = "/f_db.dat";


FingerDB::FingerDB()
{
	nextIndex = 0;
}

bool FingerDB::getBlock( uint16_t blockNum, userRecord * fBlock )
{
	File f = SPIFFS.open( __dbPath, "r" );
	if( !f ) return false;
	f.seek( sizeof( userRecord ) * blockNum, SeekSet );
	if( f.read( (uint8_t *)fBlock, sizeof( userRecord ) ) != sizeof( userRecord ) ) { f.close(); return false; }
	f.close();
	return true;
}

void FingerDB::putBlock( uint16_t blockNum, const userRecord * fBlock )
{
	File f;
	if( SPIFFS.exists(__dbPath) )
	{
		f = SPIFFS.open( __dbPath, "r+" );
		f.seek( sizeof( userRecord ) * blockNum, SeekSet );
	}
	else f = SPIFFS.open( __dbPath, "w" );
	f.write( (uint8_t *)fBlock, sizeof( userRecord ) );
	f.close();
}

uint16_t FingerDB::getFreeBlock()
{
	uint16_t i = 0;
	userRecord fBlock;
	while( i < MAX_USER )
	{
		if( !getBlock( i, &fBlock ) ) return i;
		else if( fBlock.userName[0] == 0 ) return i;
		else i++;
	}
	return NO_ID;
}

uint16_t FingerDB::findBlock( const char * userName, userRecord * fBlock )
{
	uint16_t i = 0;
	while( (i < MAX_USER) && getBlock( i, fBlock ) )
	{
		if( strcmp( userName, fBlock->userName ) == 0 ) return i;
		else i++;
	}
	return NO_ID;
}

void FingerDB::fillZeroBlock( userRecord * fBlock )
{
	memset( &fBlock->fingersID[0], NO_ID_HALF, sizeof(fBlock->fingersID) );
	memset( &fBlock->userName[0], 0, sizeof(fBlock->userName) );
}

bool FingerDB::addUser( const char * userName )
{
	uint16_t id;
	userRecord fBlock;
	if( findBlock( userName, &fBlock ) != NO_ID ) return false;
	if( (id = getFreeBlock()) == NO_ID ) return false;
	fillZeroBlock( &fBlock );
	strcpy( &fBlock.userName[0], userName );
	putBlock( id, &fBlock );
	return true;
}

void FingerDB::deleteUser( const char * userName )
{
	uint16_t uID;
	userRecord fBlock;
	if( (uID = findBlock( userName, &fBlock )) == NO_ID ) return;
	fillZeroBlock( &fBlock );
	putBlock( uID, &fBlock );
}

uint8_t FingerDB::findInFingers( uint16_t fingerID, const uint16_t * fingersID )
{
	for( uint8_t i = 0; i < N_FIGERS; i++ ) if( fingersID[i] == fingerID ) return i;
	return NO_ID_HALF;
}

bool FingerDB::getUserByfingerID( uint16_t fingerID, char * userName )
{
	uint16_t idx;
	userRecord fBlock;
	while( getBlock( idx++, &fBlock ) )
	{
		if( fBlock.userName[0] == 0 ) continue;
		if( findInFingers( fingerID, &fBlock.fingersID[0] ) != NO_ID_HALF )
		{
			strcpy( userName, &fBlock.userName[0] );
			return true;
		}
	}
	return false;
}

bool FingerDB::getNextUser( char * nextUserName )
{
	userRecord fBlock;
	while( getBlock( nextIndex++, &fBlock ) )
	{
		if( fBlock.userName[0] == 0 ) continue;
		strcpy( nextUserName, fBlock.userName );
		return true;
	}
	nextIndex = 0;
	return false;
}

char * FingerDB::getUserNames( char * userNames ) // name1,name2,...,nameN
{
	char * ptr;
	uint8_t len;
	bool ok = false;
	
	ptr = userNames;
	while( getNextUser( ptr ) )
	{
		len = strlen( ptr );
		ptr[len++] = ',';
		ptr += len;
		ok = true;
	}
	if( ok ) ptr--;
	*ptr = 0;
	return userNames;
}

bool FingerDB::getUserFingers( const char * userName, uint16_t * fingersID )
{
	userRecord fBlock;
	if( findBlock( userName, &fBlock ) == NO_ID ) return false;
	memcpy( fingersID, &fBlock.fingersID[0], sizeof(fBlock.fingersID) );
	return true;
}

void FingerDB::addFinger( const char * userName, uint16_t fingerID, uint8_t fingerIndex )
{
	uint16_t id;
	userRecord fBlock;
	if(  fingerIndex >= N_FIGERS || ( id=findBlock( userName, &fBlock ) ) == NO_ID ) return;
	fBlock.fingersID[fingerIndex] = fingerID;
	putBlock( id, &fBlock );
}

void FingerDB::deleteFinger( const char * userName, uint8_t fingerIndex )
{
	addFinger( userName, NO_ID, fingerIndex );
}

void FingerDB::emptyDatabase()
{
	SPIFFS.remove(__dbPath);
}