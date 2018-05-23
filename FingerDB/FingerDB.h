#ifndef FingerDB_h
#define FingerDB_h
#include <arduino.h>

#define NO_ID		(0xFFFF)
#define NO_ID_HALF	(0xFF)

#define MAX_USER	255
#define N_FIGERS	10
#define MAX_NAME	12

// нумерация пальцев: 0 - левый мизинец, 1 левый безымянный и т.д.

typedef struct ur
{
	char  userName[MAX_NAME];
	uint16_t fingersID[N_FIGERS];
} userRecord;

class FingerDB
{
	public:
		FingerDB();
		bool addUser( const char * userName ); // return userID
		void deleteUser( const char * userName );
		bool getUserByfingerID( uint16_t fingerID, char * userName );
		bool getNextUser( char * nextUserName ); // search users
		char * getUserNames( char * userNames ); // name1,name2,...,nameN
		bool getUserFingers( const char * userName, uint16_t * fingersID ); // return 10 finger IDs, index - fingerIdx, NO_ID - no record
		void addFinger( const char * userName, uint16_t fingerID, uint8_t fingerIndex );
		void deleteFinger( const char * userName, uint8_t fingerIndex );
		void emptyDatabase();

	private:
		bool getBlock( uint16_t blockNum, userRecord * fBlock );
		void putBlock( uint16_t blockNum, const userRecord * fBlock );
		void fillZeroBlock( userRecord * fBlock );
		uint16_t getFreeBlock();
		uint16_t findBlock( const char * userName, userRecord * fBlock );
		uint8_t findInFingers( uint16_t fingerID, const uint16_t * fingersID );
		uint16_t nextIndex;
};

#endif
