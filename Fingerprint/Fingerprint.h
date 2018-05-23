#ifndef FINGERPRINT_H
#define FINGERPRINT_H

#include "Arduino.h"
#if defined(__AVR__) || defined(ESP8266)
  #include <SoftwareSerial.h>
#endif

#define FINGERPRINT_STARTCODE			(0xEF01)

#define FINGERPRINT_DEFAULT_ADRESS		(0xFFFFFFFF)
#define FINGERPRINT_DEFAULT_PASSWORD	(0x00000000)
#define FINGERPRINT_DEFAULT_BAUDRATE	(57600)

#define FINGERPRINT_READ_TTIMEOUT	(1000) // UART reading timeout in milliseconds

#define FINGER_WRITE_BYTE( b )	mySerial->write( (uint8_t)(b) )
#define FINGER_WRITE_WORD( w )	FINGER_WRITE_BYTE( (uint16_t)(w) >> 8 ); FINGER_WRITE_BYTE( (uint16_t)(w) & 0xFF )
#define FINGER_WRITE_DWORD( w )	FINGER_WRITE_WORD( (uint32_t)(w) >> 16 ); FINGER_WRITE_WORD( (uint32_t)(w) & 0xFFFF )


enum fingerCommand : uint8_t
{
	FINGERPRINT_GET_IMAGE		= 0x01, // Получить изображение отпечатка
	FINGERPRINT_IMAGE_2_TZ		= 0x02, // Создать файл свертки в буффере 1 или 2
	FINGERPRINT_TZ_2_TEMPL		= 0x05, // Создание шаблона в буффере 1 и 2
	FINGERPRINT_STORE_TEMPL		= 0x06, // Записи шаблона в библитеку из буфера 1 или 2
	FINGERPRINT_LOAD_TEMPL		= 0x07, // Чтение шаблона из библитеки в буфер 1 или 2
	FINGERPRINT_UPLOAD			= 0x08, // Считать в хост содержимое буффера 1 или 2 файл свертки, предварительно прочитанный
	FINGERPRINT_DOWNPLOAD		= 0x09, // Загрузить из хоста в буффер 1 или 2 файл свертки
	FINGERPRINT_DELETE			= 0x0C, // Удалить шаблон из библиотеки
	FINGERPRINT_EMPTY 			= 0x0D, // Очистить библиотеку
	FINGERPRINT_SET_SYSPAR		= 0x0E,	// Установить системные параметры
	FINGERPRINT_GET_SYSPAR		= 0x0F,	// Читать системные параметры
	FINGERPRINT_SET_PASS		= 0x12, // Установить пароль модуля
	FINGERPRINT_CHECK_PASS		= 0x13, // Проверить пароль модуля, делается первой коммандой
	FINGERPRINT_SET_ADDR		= 0x15, // Установить новый адрес модуля
	FINGERPRINT_SEARCH			= 0x1B, // Поиск в библиотеке совпадения с шаблоном из буффера 1 или 2
	FINGERPRINT_TEMPL_NUM		= 0x1D, // Прлучить число шаблонов в библиотеке
	FINGERPRINT_TEMPL_LIST		= 0x1F, // Чтение таблицы занятости ячеек в библиотеке шаблонов
	FINGERPRINT_LED_ON			= 0x50, // Включить подсветку
	FINGERPRINT_LED_OFF			= 0x51, // Отключить подсветку
	FINGERPRINT_GET_IMAGE_NOLED	= 0x52, // Получить изображение отпечатка без управлени подсветкой
};

enum fingerCode : uint8_t
{
	FINGERPRINT_OK					= 0x00, // commad execution complete
	FINGERPRINT_PACKETRECIEVEERR	= 0x01, // error when receiving data package
	FINGERPRINT_NOFINGER			= 0x02, // no finger on the sensor
	FINGERPRINT_IMAGEFAIL			= 0x03, // fail to enroll the finger
	FINGERPRINT_IMAGEMESS			= 0x06, // fail to generate character file due to the over-disorderly fingerprint image
	FINGERPRINT_FEATUREFAIL			= 0x07, // fail to generate character file due to lackness of character point or over-smallness of fingerprint image
	FINGERPRINT_NOMATCH				= 0x08, // finger doesn’t match
	FINGERPRINT_NOTFOUND			= 0x09, // fail to find the matching finger
	FINGERPRINT_ENROLLMISMATCH		= 0x0A, // fail to combine the character files
	FINGERPRINT_BADLOCATION			= 0x0B, // addressing PageID is beyond the finger library
	FINGERPRINT_DBRANGEFAIL			= 0x0C, // error when reading template from library or the template is invalid
	FINGERPRINT_UPLOADFEATUREFAIL	= 0x0D, // error when uploading template
	FINGERPRINT_PACKETRESPONSEFAIL	= 0x0E, // Module can’t receive the following data packages. 0Fh: error when uploading image
	FINGERPRINT_UPLOADFAIL			= 0x0F, // error when uploading image
	FINGERPRINT_DELETEFAIL			= 0x10, // fail to delete the template
	FINGERPRINT_DBCLEARFAIL			= 0x11, // fail to clear finger library
	FINGERPRINT_PASSFAIL			= 0x13, // wrong password
	FINGERPRINT_INVALIDIMAGE		= 0x15, // fail to generate the image for the lackness of valid primary image
	FINGERPRINT_FLASHERR			= 0x18, // error when writing flash
	FINGERPRINT_NODEFINITION		= 0x19, // No definition error
	FINGERPRINT_INVALIDREG			= 0x1A, // invalid register number
	FINGERPRINT_INCORRREGCONF		= 0x1B, // incorrect configuration of register
	FINGERPRINT_ADDRCODE			= 0x20, // 
	FINGERPRINT_PASSVERIFY			= 0x21, // 
	FINGERPRINT_TIMEOUT 			= 0xF0, //
	FINGERPRINT_BADPACKET			= 0xFE, //
	FINGERPRINT_BADADDRESS			= 0xFF  //
};

enum fingerPacket : uint8_t
{
	FINGERPRINT_COMMANDPACKET		= 0x01,
	FINGERPRINT_DATAPACKET			= 0x02,
	FINGERPRINT_ACKPACKET			= 0x07,
	FINGERPRINT_ENDDATAPACKET		= 0x08
};

enum setSysParametr : uint8_t
{
	FINGER_SYSPAR_BAUDRATE			= 4, // 1..12 baud rate is 9600*N bps。
	FINGER_SYSPAR_SECURITY_LEVEL	= 5, // 1..5 5 - max paranoia
	FINGER_SYSPAR_DATA_PACKAGE_LEN	= 6  // 0 - 32 bytes, 1 - 64 bytes, 2 - 128 bytes, 3 - 256 bytes
};

enum addFingerSteps : uint8_t
{
	ADD_FINGER_BEGIN = 0,       // поиск свободной ячейки, если успех, то вернет ID, далее передавать ID не нужно
	ADD_FINGER_SCAN = 1,        // сканировать палец
	ADD_FINGER_NO_SCAN = 1,     // убрать палец
	ADD_FINGER_SCAN_AND_ADD = 1 // сканировать снова и записать в библиотеку сканнера, ID не нужен, т.к. используется найденный вначале
};

struct systemStatusRegister
{
	uint16_t Busy ： 1;       // system is executing commands; 0: system is free
	uint16_t Pass ： 1;       // find the matching finger; 0: wrong finger
	uint16_t PWD ： 1;        // Verified device’s handshaking password
	uint16_t ImgBufStat ： 1; // image buffer contains valid image.
	uint16_t Reserved : 12;
}

struct sysParametrs
{
	uint16_t statusRegister;   // Contents of system status register
	uint16_t systemIdentifier; // Fixed value: 0x0009
	uint16_t librarySize;      // Finger library size
	uint16_t securityLevel;    // Security level (1, 2, 3, 4, 5)
	uint32_t deviceAddress;    // 32-bit device address
	uint16_t dataPacketSize;   // Size code (0, 1, 2, 3)
	uint16_t baudSettings;     // N (baud = 9600*N bps)
}

class Fingerprint
{
	public:
		Fingerprint( SoftwareSerial *ss, uint32_t baudrate = FINGERPRINT_DEFAULT_BAUDRATE, uint32_t address = FINGERPRINT_DEFAULT_ADRESS );
		Fingerprint( HardwareSerial *hs, uint32_t baudrate = FINGERPRINT_DEFAULT_BAUDRATE, uint32_t address = FINGERPRINT_DEFAULT_ADRESS );
		uint8_t begin( uint32_t password = FINGERPRINT_DEFAULT_PASSWORD );

		uint8_t fingerGetPressFinger( uint16_t *ID, uint16_t *MatchScore, uint8_t slot = 1 );

		uint8_t addNewFinger( uint8_t step, uint8_t * msg, uint16_t *ID = 0 );

		uint8_t getImage();
		uint8_t getImageNoLed();
		uint8_t image2Tz( uint8_t slot = 1 );
		uint8_t fingerSearch( uint16_t *ID, uint16_t *MatchScore, uint8_t slot = 1 );

		uint8_t TZtoTemplate();
		uint8_t storeTemplate( uint16_t ID, uint8_t slot = 1 );
		uint8_t loadTemplate( uint16_t ID, uint8_t slot = 1 );
		uint8_t uploadTemplate( uint8_t *buffer, uint16_t *recived, uint8_t slot = 1 );
		uint8_t deleteTemplate( uint16_t startID, uint16_t numbers = 1 );
		uint8_t getTemlateCount( uint16_t *count );

		uint8_t clearTemplateDB();

		uint8_t lightSwitch( bool on );
		uint8_t setPassword( uint32_t password );
		uint8_t setNewAddress( uint32_t theNewAddress );
		uint8_t setSysParametr( uint8_t parameterNumber, uint8_t value );
		uint8_t getSysParametrs( sysParametrs *sysData );

	protected:
		uint8_t verifyPassword( uint32_t password );
		uint8_t fCmd( uint8_t instructionCode );
		void writePacket( uint8_t instructionCode, uint8_t *data = 0, uint16_t n_bytes = 0 );
		uint8_t readByte( uint8_t * b );
		uint8_t readWord( uint16_t * w );
		uint8_t readDword( uint32_t * dw );
		uint8_t readPacket( uint8_t *packet_type, uint8_t *data = 0, uint16_t *n_bytes = 0 );

	private:
		uint32_t theAddress;
		Stream *mySerial;
}

#endif
