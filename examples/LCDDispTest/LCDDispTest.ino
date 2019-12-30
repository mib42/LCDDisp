#include "LCDDisp.h"

#define LCD      PCF8574A_BASE | PCF8574_DEVICE_0   //the device typ and the uses adress setting
#define PIN_SCL  A5   //GPIO5   //SCL
#define PIN_SDA  A4   //GPIO4   //SDA

LCDDisp lcd( PIN_SCL, PIN_SDA, LCD, 16, 4 );       //create the object:  16 cols, 4 rows

void setup() {
  Serial.begin(115200);
  Serial.println("Begin:");
  lcd.begin();
  if ( lcd.avail() ) {  Serial.println("available"); } else {  Serial.println("not available");  }
  
}

void loop() {

  //Hintergrundbeleuchtung ausschalten
  lcd.backLightOff();

  //Bildschirm löschen; setzt auch den Cursor auf Spalte=0, Zeile=0
  lcd.clear();
  
  //Cursor auf Spalte=4, Zeile=0 setzen
  lcd.moveCursorTo( 4, 0 ); 
  //Text ab dieser Position ausgeben
  lcd.putString( "BBS TGHS" );  
  lcd.moveCursorTo( 1, 1 ); 
  lcd.putString( "Bad Kreuznach" );  
  delay( 2000 );

  //Text ausgeben und anschließend eine 8-Bit-Zahl
  lcd.moveCursorTo( 0, 2 ); 
  lcd.putString( "cols:" );
  lcd.putNumber( lcd.cols(), 3, ' ' );
  
  lcd.moveCursorTo( 0, 3 ); 
  lcd.putString( "rows:" );
  lcd.putNumber( lcd.rows(), 3, ' ' );

  lcd.cursorOn();
  delay( 2000 );
  lcd.cursorBlink();
  delay( 2000 );
  lcd.shiftCursor( CMD_SHIFT_RIGHT );  delay( 1000 );
  lcd.shiftCursor( CMD_SHIFT_RIGHT );  delay( 1000 );
  lcd.shiftCursor( CMD_SHIFT_RIGHT );  delay( 1000 );
  lcd.shiftCursor( CMD_SHIFT_LEFT );   delay( 1000 );
  lcd.shiftCursor( CMD_SHIFT_LEFT );   delay( 1000 );
  lcd.shiftCursor( CMD_SHIFT_LEFT );   delay( 1000 );
  lcd.cursorBlinkOff();
  delay( 2000 );
  lcd.cursorOff();
  delay( 2000 );

  lcd.shiftDisplay( CMD_SHIFT_RIGHT );  delay( 1000 );
  lcd.shiftDisplay( CMD_SHIFT_RIGHT );  delay( 1000 );
  lcd.shiftDisplay( CMD_SHIFT_RIGHT );  delay( 1000 );
  lcd.shiftDisplay( CMD_SHIFT_LEFT );   delay( 1000 );
  lcd.shiftDisplay( CMD_SHIFT_LEFT );   delay( 1000 );
  lcd.shiftDisplay( CMD_SHIFT_LEFT );   delay( 1000 );


  //Hintergrundbeleuchtung einschalten
  lcd.backLightOn( );

  //Cursor auf Spalte=10, Zeile=1 setzen
  lcd.moveCursorTo( 2, 1 );
  lcd.putChar( 'ä' );
  lcd.putChar( 'ö' );
  lcd.putChar( 'ü' );
  lcd.putChar( 'ß' );
  lcd.putChar( '°' );
  lcd.putChar( 'Ä' );
  lcd.putChar( 'Ö' );
  lcd.putChar( 'Ü' );
  delay( 2000 );
  lcd.backLightOff( );


}

