/*
Параметры
Плата: Generic ESP8266 Module
Flash Mode: QIO
Flash Size: 4M (1M SPIFF)
Reset Method: ck
Crystall Frequency: 26 MHz
Flash Frequency: 80 MHz
Cpu Frequency: 80 MHz

*/
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <EspFileServer.h>
#include <Ticker.h>
#include <StreamString.h>
#include <FingerDB.h>
#include <Adafruit_Fingerprint.h>
#include <Dht12.h>
#define DHT_TIMEOUT	10000 // 10 сек

#define BUTTON_PIN	0
#define LIGHT_PIN	2
#define DOOR_PIN	15
#define DOOR_PULSE	100
#define DOOR_LIGHT	30000 // 30 сек

#define ADD_TIMEOUT	60000 // 60 сек - таймаут флага добавления пальца

ESP8266WebServer server(80);
EspFileServer fileServer( &server );
FingerDB fUsers;
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&Serial);
Ticker lighter;
Ticker adder;

Dht12 dht;
Ticker temper;

bool is_finger_connect = false;
bool process_add_flag = false;

void processDht() { if( !process_add_flag ) dht.read(); } // вызывать не чаще 1 раза в секунду

uint16_t getFingerprintIDez()
{
	if( process_add_flag || !is_finger_connect ) return 0xffff;
	if( finger.getImage() != FINGERPRINT_OK) return 0xffff;
	if( finger.image2Tz() != FINGERPRINT_OK) return 0xffff;
	if( finger.fingerFastSearch() != FINGERPRINT_OK) return 0xffff;
	return finger.fingerID; 
}

void clearAddFlag() { process_add_flag = false; }

void LightOff() { digitalWrite( LIGHT_PIN, 0 ); }

void LightOn()
{
	digitalWrite( LIGHT_PIN, 1 );
	lighter.once_ms( DOOR_LIGHT, LightOff );
}

bool isLightOn() { return digitalRead(LIGHT_PIN); }

void LightToogle()
{
	if( isLightOn() ) LightOff();
	else LightOn();
}

void OpenDoor()
{
	digitalWrite( DOOR_PIN, 1 );
	delay(DOOR_PULSE);
	digitalWrite( DOOR_PIN, 0 );
}

// Обработчик запросов по замку
void d_cmd() //  /d_cmd?q=d, s, t 
{
	StreamString web;
	web.print("");
	switch( server.arg("q")[0] )
	{
		// Работа с замком
		case 'd':	LightOn(); OpenDoor(); break;
		// Работа со светом
		case 'l':	if( server.arg("c") == "t" ) LightToogle();
					else if( server.arg("c") == "i" ) web.print( isLightOn() ? '1' : '0' );
					break;
		// Температура
		case 't':	web.print( dht.getTemperature() ); web.print( ',' ); web.print( dht.getHumidity() ); break;
	}
	server.send( 200, "text/plain", web );
}

// Обработчик запросов по сканнеру и базе пользователей
void f_cmd() // /f_cmd?q=x&...
{
	StreamString web;
	static uint16_t p; // для поиска ID
	web.print("");
	switch( server.arg("q")[0] )
	{
		// Очистить базу данных
		case 'z':	fUsers.emptyDatabase();
					if( is_finger_connect ) finger.emptyDatabase();
					break;
		// Получить список пользователей
		case 'l':	{ char buff[1024];
					fUsers.getUserNames( buff );
					web.print( buff ); }
					break;
		// Добавить или удалить пользователя
		case 'u':	if( server.arg("c") == "a" ) fUsers.addUser( server.arg("u").c_str() );
					else if( server.arg("c") == "d" )
					{
						uint16_t fingersID[10];
						fUsers.getUserFingers( server.arg("u").c_str(), fingersID );
						if( is_finger_connect ) {
							for( uint8_t i = 0; i < 10; i++ ) { if( fingersID[i] < 65000 ) finger.deleteModel( fingersID[i] ); }
						}
						fUsers.deleteUser( server.arg("u").c_str() );
					}
					break;
		// Удалить отпечаток или получить список отпечатков
		case 'f':	if( server.arg("c") == "d" )
					{
						uint16_t idx = (uint16_t)server.arg("idx").toInt();
						if( is_finger_connect )
						{
							uint16_t fingersID[10];
							fUsers.getUserFingers( server.arg("u").c_str(), fingersID );
							finger.deleteModel( fingersID[idx] );
						}
						fUsers.deleteFinger( server.arg("u").c_str(), idx );
					}
					else if( server.arg("c") == "l" )
					{
						uint16_t fingersID[10];
						fUsers.getUserFingers( server.arg("u").c_str(), fingersID );
						web.print( fingersID[0], DEC );
						for( uint8_t i = 1; i < 10; i++ ) { web.print( "," ); web.print( fingersID[i], DEC ); }
					}
					break;
		// добавить отпечаток
		case 'a':	if( is_finger_connect )
					{
						switch( (uint16_t)(server.arg("s").toInt()) )
						{
							case 60001:	process_add_flag = true; // взвести флаг, чтобы не опрашивать в главном цикле
										adder.detach(); adder.once_ms( ADD_TIMEOUT, clearAddFlag );
										p = finger.firstFreeModel(); // найдем свододный ID для сохранения отпечатка
										if( p == 0xFFFF ) web.print( 65103 );
										else if( finger.getImage() != FINGERPRINT_OK ) web.print( 60001 );
										else
										{
											if( finger.image2Tz(1) != FINGERPRINT_OK ) web.print( 65101 );
											else web.print( 60002 );
										}
										break;
							case 60002:	if( finger.getImage() != FINGERPRINT_NOFINGER ) web.print( 60002 );
										else web.print( 60003 );
										break;
							case 60003:	if( finger.getImage() != FINGERPRINT_OK ) web.print( 60003 );
										else
										{
											if( finger.image2Tz(2) != FINGERPRINT_OK ) web.print( 65102 );
											else web.print( 60004 );
										}
										break;
							case 60004:	if( finger.createModel() != FINGERPRINT_OK ) web.print( 65100 );
										else
										{
											if( finger.storeModel( p ) != FINGERPRINT_OK ) web.print( 65104 );
											else
											{
												fUsers.addFinger( server.arg("u").c_str(), p, server.arg("idx").toInt() );
												process_add_flag = false;
												adder.detach();
												web.print( 60005 );
											}
										}
										break;
							default :	web.print( 60001 );
										break;
						}
					}
					else web.print( 65105 );
					break;
		default:	break;
	}
	server.send( 200, "text/plain", web );
}

void InitWiFi()
{
	const char * WiFi_Name = "Oleg_Home";
	const char * WiFi_Pass = "lbvf1234";

	IPAddress staticIP(192,168,1,100);
	IPAddress gateway(192,168,1,1);
	IPAddress subnet(255,255,255,0);

	WiFi.config(staticIP, gateway, subnet);
	WiFi.begin(WiFi_Name, WiFi_Pass);

	while(WiFi.waitForConnectResult() != WL_CONNECTED) WiFi.begin(WiFi_Name, WiFi_Pass);
}

void InitPins()
{
	pinMode( BUTTON_PIN, INPUT );
	pinMode( DOOR_PIN, OUTPUT );
	digitalWrite( DOOR_PIN, 0 );
	pinMode( LIGHT_PIN, OUTPUT );
	digitalWrite( LIGHT_PIN, 0 );
}

bool isButtonPressed()
{
	return !digitalRead(BUTTON_PIN);
}

void setup(void)
{
	InitPins();
	temper.attach_ms( DHT_TIMEOUT, processDht );
	InitWiFi();
	server.on( String(F("/f_cmd")), HTTP_GET, f_cmd ); // установим обработчик на сканнер и базу
	server.on( String(F("/d_cmd")), HTTP_GET, d_cmd ); // установим обработчик на замок
	server.begin();
	finger.begin(57600);
	if( finger.verifyPassword() ) is_finger_connect = true;
}

void loop(void)
{
	server.handleClient();
	if( getFingerprintIDez() < 0xffff )
	{
		LightOn();
		OpenDoor();
		delay(500);
	}
	else if( isButtonPressed() )
	{
		LightToogle();
		delay(500);
	}
	//delay(1000);
}

