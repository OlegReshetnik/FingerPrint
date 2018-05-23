#include "Fingerprint.h"

static const char _finger_err_free_mem[] PROGMEM = "Нет свободных ячеек в сканнере";
static const char _finger_err_write[] PROGMEM = "Ошибка записи шаблона в сканнер";
static const char _finger_ok_add[] PROGMEM = "Шаблон успешно добавлен";

static const char _finger_press[] PROGMEM = "Приложите палец к сканнеру";
static const char _finger_free[] PROGMEM = "Уберите палец от сканнера";



Fingerprint::Fingerprint( SoftwareSerial *ss, uint32_t baudrate, uint32_t address ) // baud = def, adr = def
{
	theAddress = address;
	mySerial = ss;
	ss->begin( baudrate );
}

Fingerprint::Fingerprint( HardwareSerial *hs, uint32_t baudrate, uint32_t address )
{
	theAddress = address;
	mySerial = hs;
	hs->begin( baudrate );
}

uint8_t Fingerprint::begin( uint32_t password )
{
	delay(1000); // one second delay to let the sensor 'boot up'
	return verifyPassword( password );
}

uint8_t Fingerprint::getImage()
{
	return fCmd( FINGERPRINT_GET_IMAGE );
}

uint8_t Fingerprint::getImageNoLed()
{
	return fCmd( FINGERPRINT_GET_IMAGE_NOLED );
}

uint8_t Fingerprint::TZtoTemplate()
{
	return fCmd( FINGERPRINT_TZ_2_TEMPL );
}

uint8_t Fingerprint::clearTemplateDB()
{
	return fCmd( FINGERPRINT_EMPTY );
}

uint8_t Fingerprint::lightSwitch( bool on )
{
	if( on ) return fCmd( FINGERPRINT_LED_ON );
	return fCmd( FINGERPRINT_LED_OFF );
}

uint8_t Fingerprint::verifyPassword( uint32_t password )
{
	uint8_t packet_type;
	writePacket( FINGERPRINT_CHECK_PASS, (uint8_t *)&password, sizeof(password) );
	return readPacket( &packet_type ); // , uint8_t *data = 0, uint16_t *n_bytes = 0
}

uint8_t Fingerprint::setPassword( uint32_t password )
{
	uint8_t packet_type;
	writePacket( FINGERPRINT_SET_PASS, (uint8_t *)&password, sizeof(password) );
	return readPacket( &packet_type );
}

uint8_t Fingerprint::setNewAddress( uint32_t theNewAddress )
{
	uint8_t packet_type;
	writePacket( FINGERPRINT_SET_ADDR, (uint8_t *)&theNewAddress, sizeof(theNewAddress) );
	theAddress = theNewAddress;
	return readPacket( &packet_type ); 
}

uint8_t Fingerprint::setSysParametr( uint8_t parameterNumber, uint8_t value )
{
	uint8_t packet_type, data[2];
	data[0] = parameterNumber;
	data[1] = value;
	writePacket( FINGERPRINT_SET_SYSPAR, data, sizeof(data) );
	return readPacket( &packet_type ); 
}

uint8_t Fingerprint::getSysParametrs( sysParametrs *sysData )
{
	uint8_t packet_type;
	uint16_t nn;
	writePacket( FINGERPRINT_GET_SYSPAR );
	return readPacket( &packet_type, (uint8_t *)sysData, &nn );
}

uint8_t Fingerprint::image2Tz( uint8_t slot )
{
	uint8_t packet_type;
	writePacket( FINGERPRINT_IMAGE_2_TZ, *slot, sizeof(slot) );
	return readPacket( &packet_type ); 
}

uint8_t Fingerprint::storeTemplate( uint16_t ID, uint8_t slot )
{
	uint8_t packet_type, data[3];
	data[0] = slot;
	*( (uint16_t *)(&data[1]) ) = ID;
	writePacket( FINGERPRINT_STORE_TEMPL, data, sizeof(data) );
	return readPacket( &packet_type ); 
}

uint8_t Fingerprint::loadTemplate( uint16_t ID, uint8_t slot )
{
	uint8_t packet_type, data[3];
	data[0] = slot;
	*( (uint16_t *)(&data[1]) ) = ID;
	writePacket( FINGERPRINT_LOAD_TEMPL, data, sizeof(data) );
	return readPacket( &packet_type );
}

uint8_t Fingerprint::uploadTemplate( uint8_t *buffer, uint16_t *recived, uint8_t slot ) // читать Шаблон из сканнера
{
	uint8_t ret, packet_type;
	uint16_t nn, cnt = 0;

	writePacket( FINGERPRINT_UPLOAD, slot, sizeof(slot) );
	ret = readPacket( &packet_type );
	if( ret != FINGERPRINT_OK ) return ret;

	do {
		ret = readPacket( &packet_type, buffer, &nn );
		if( ret != FINGERPRINT_OK ) return ret;
		buffer += nn;
		cnt += nn;
	} while( packet_type != FINGERPRINT_ENDDATAPACKET );

	*recived = cnt;
	return FINGERPRINT_OK;
}

/*
uint8_t Fingerprint::downloadTemplate( uint8_t *buffer, uint16_t to_send, uint8_t slot = 1 ) // записать Шаблон из сканнера
{
	uint8_t ret, packet_type;
	uint16_t nn, cnt = 0;

	writePacket( FINGERPRINT_DOWNPLOAD, slot, sizeof(slot) );
	ret = readPacket( &packet_type );
	if( ret != FINGERPRINT_OK ) return ret;

	do {
		ret = readPacket( &packet_type, buffer, &nn );
		if( ret != FINGERPRINT_OK ) return ret;
		buffer += nn;
		cnt += nn;
	} while( packet_type != FINGERPRINT_ENDDATAPACKET );

	*recived = cnt;
	return FINGERPRINT_OK;
}
*/

uint8_t Fingerprint::deleteTemplate( uint16_t startID, uint16_t numbers )
{
	uint8_t packet_type;
	uint32_t dw = ( (uint32_t)(startID << 16) ) | ( (uint32_t)numbers );
	writePacket( FINGERPRINT_DELETE, (uint8_t *)&dw, sizeof(dw) );
	return readPacket( &packet_type );
}

uint8_t Fingerprint::getTemlateCount( uint16_t *count )
{
	uint8_t packet_type;
	uint16_t nn;
	writePacket( FINGERPRINT_TEMPL_NUM );
	return readPacket( &packet_type, count, &nn );
}

uint8_t Fingerprint::fingerSearch( uint16_t *ID, uint16_t *MatchScore, uint8_t slot )
{
	uint8_t ret, packet_type, data[5];
	uint16_t nn;

	writePacket( FINGERPRINT_TEMPL_NUM ); // найдем число отпечетков
	if( (ret = readPacket( &packet_type, data, &nn )) != FINGERPRINT_OK ) return ret;
	
	data[3] = data[0]; // 0 и 1 байты содержат число отпечатков
	data[4] = data[1]; // Ставим в поле 'to'
	data[0] = slot;
	data[1] = data[2] = 0; // search from 0

	writePacket( FINGERPRINT_SEARCH, data, sizeof(data) );
	ret = readPacket( &packet_type );
	if( ret != FINGERPRINT_OK ) return ret;

	*ID = *( (uint16_t *)(&data[0]) );
	*MatchScore = *( (uint16_t *)(&data[2]) );

	return FINGERPRINT_OK;
}

uint8_t Fingerprint::fingerGetPressFinger( uint16_t *ID, uint16_t *MatchScore, uint8_t slot )
{
	uint8_t ret;

	if( (ret = getImage()) != FINGERPRINT_OK ) return ret;
	if( (ret = image2Tz( slot )) != FINGERPRINT_OK ) return ret;
	if( (ret = fingerSearch( ID, MatchScore, slot )) != FINGERPRINT_OK ) return ret;
	return FINGERPRINT_OK;
}

uint8_t Fingerprint::fCmd( uint8_t instructionCode )
{
	uint8_t packet_type;
	writePacket( instructionCode );
	return Fingerprint::readPacket( &packet_type );
}

void Fingerprint::writePacket( uint8_t instructionCode, uint8_t *data, uint16_t n_bytes )
{
	while( mySerial->available() ) mySerial->read(); // Очистить входной буффер от старых пакетов ??

	FINGER_WRITE_WORD( FINGERPRINT_STARTCODE );
	FINGER_WRITE_DWORD( theAddress );
	FINGER_WRITE_BYTE( FINGERPRINT_COMMANDPACKET );

	uint16_t pack_length = n_bytes + 3;
	FINGER_WRITE_WORD( pack_length );
	FINGER_WRITE_BYTE( instructionCode );

	uint16_t sum = ( pack_length >> 8 ) + ( pack_length & 0xFF ) + FINGERPRINT_COMMANDPACKET + instructionCode;
	for( uint16_t i = 0; i < n_bytes; i++ )
	{
		FINGER_WRITE_BYTE( *data );
		sum += *data++;
	}
	FINGER_WRITE_WORD( sum );
}

uint8_t Fingerprint::readByte( uint8_t * b )
{
	while( !mySerial->available() )
	{
		delay(1);
		timer++;
		if( ++timer > FINGERPRINT_READ_TTIMEOUT ) return FINGERPRINT_TTIMEOUT;
	}
	*b = mySerial->read();
	return FINGERPRINT_OK;
}

uint8_t Fingerprint::readWord( uint16_t * w )
{
	uint8_t bb;
	uint16_t ret = 0;
	
	if( readByte( &bb ) != FINGERPRINT_OK ) return FINGERPRINT_TTIMEOUT;
	*w = (uint16_t)(bb << 8);
	if( readByte( &bb ) != FINGERPRINT_OK ) return FINGERPRINT_TTIMEOUT;
	*w |= (uint16_t)bb;
	return FINGERPRINT_OK;
}

uint8_t Fingerprint::readDword( uint32_t * dw )
{
	uint16_t ww;
	uint32_t ret = 0;
	
	if( readWord( &ww ) != FINGERPRINT_OK ) return FINGERPRINT_TTIMEOUT;
	*dw = (uint32_t)(ww << 16);
	if( readWord( &ww ) != FINGERPRINT_OK ) return FINGERPRINT_TTIMEOUT;
	*dw |= (uint32_t)ww;
	return FINGERPRINT_OK;
}

uint8_t Fingerprint::readPacket( uint8_t *packet_type, uint8_t *data, uint16_t *n_bytes )
{
	uint32_t dw;
	uint16_t ww, len;
	uint8_t confirmCode, 

	if( readWord( &ww ) != FINGERPRINT_OK ) return FINGERPRINT_TTIMEOUT; // start code
	if( ww != FINGERPRINT_STARTCODE ) return FINGERPRINT_BADPACKET;

	if( readDword( &dw ) != FINGERPRINT_OK ) return FINGERPRINT_TTIMEOUT; // address
	if( dw != theAddress ) return FINGERPRINT_BADADDRESS;

	if( readByte( packet_type ) != FINGERPRINT_OK ) return FINGERPRINT_TTIMEOUT; // packet type

	if( readWord( &ww ) != FINGERPRINT_OK ) return FINGERPRINT_TTIMEOUT; // packet length
	len = ww - 3;

	if( readByte( &confirmCode ) != FINGERPRINT_OK ) return FINGERPRINT_TTIMEOUT; // return code

	if( len )
	{
		*n_bytes = len;
		while( len-- ) if( readByte( data++ ) != FINGERPRINT_OK ) return FINGERPRINT_TTIMEOUT;
	}
	
	if( readWord( &ww ) != FINGERPRINT_OK ) return FINGERPRINT_TTIMEOUT; // summ

	return confirmCode;
}

uint8_t getFirstFreeId( uint16_t *ID )
{
	uint8_t ret;
	uint16_t num;
	sysParametrs param;
	
	if( (ret = getTemlateCount( &num )) != FINGERPRINT_OK ) return ret;
	if( (ret = getSysParametrs( &param )) != FINGERPRINT_OK ) return ret;

	while( loadTemplate( i ) == FINGERPRINT_OK ) i++;
	if( i == param.librarySize ) return FINGERPRINT_NOTFOUND;

	return FINGERPRINT_OK;
}

uint8_t Fingerprint::addNewFinger( uint8_t step, uint8_t * msg, uint16_t *ID )
{
	static uint16_t id;
	uint8_t ret;

	switch( step )
	{
		case 0: if( (ret = getFirstFreeId( &id )) != FINGERPRINT_OK ) { msg = _finger_err_free_mem; return ret; }
				msg = _finger_press;
				return FINGERPRINT_OK;

		case 1: if( (ret = getImage()) != FINGERPRINT_OK ) { msg = _finger_press; return ret; }
				image2Tz(1);
				msg = _finger_free;
				return FINGERPRINT_OK;

		case 2: if( (ret = getImage()) == FINGERPRINT_OK ) { msg = _finger_free; return FINGERPRINT_NOFINGER; }
				msg = _finger_press;
				return FINGERPRINT_OK;

		case 3: if( (ret = getImage()) != FINGERPRINT_OK ) { msg = _finger_press; return ret; }
				image2Tz(2);
				msg = _finger_free;
				return FINGERPRINT_OK;

		case 4: if( (ret = getImage()) != FINGERPRINT_OK ) { msg = _finger_press; return ret; }
				TZtoTemplate();
				if( (ret = storeTemplate( id )) != FINGERPRINT_OK ) { msg = _finger_err_write; return ret; }
				msg = _finger_ok_add;
				return FINGERPRINT_OK;
	}

	msg = _finger_press;
	return FINGERPRINT_NOFINGER;
}
