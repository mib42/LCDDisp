#include "Arduino.h"
#include <Wire.h>
#include "LCDDisp.h"


//Here we define the bit patterns for the upper case mutated vowals ÄÖÜ in german
//the lower case versions are already defined in the CG 
byte Auml[8] =  { B01010, B00000, B01110, B10001, B10001, B11111, B10001, B10001 };
byte Uuml[8] =  { B01010, B00000, B10001, B10001, B10001, B10001, B10001, B01110 };
byte Ouml[8] =  { B01010, B00000, B01110, B10001, B10001, B10001, B10001, B01110 };

//check layout of Display Data Ram for line 0 .. line 3
//uint8_t DD_RAM_Offsets[] = { 0x00, 0x40, 0x14, 0x54 };     //valid for 20x4 displays
uint8_t DD_RAM_Offsets[] = { 0x00, 0x40, 0x10, 0x50 };     //valid for 16x4 displays



LCDDisp::LCDDisp( uint8_t pinSCL, uint8_t pinSDA, uint8_t i2cAdr, uint8_t cols, uint8_t rows ) {
  _pinSCL  = pinSCL;
  _pinSDA  = pinSDA;
  _i2cAdr  = i2cAdr;
  _col     = cols;
  _row     = rows;
  _reg.all = CTRL_INIT;
  _pcf.all = 0;

}

void LCDDisp::begin() {
  #if defined(__AVR_ATmega328P__)  // Arduino UNO, NANO
    Wire.begin();
  #else                            //ESP 
    Wire.begin( _pinSDA, _pinSCL );
  #endif
  init();
}

bool LCDDisp::avail() {
  Wire.beginTransmission( _i2cAdr );// Kommunikation mit Gerät(ID=i) testen
  return( (Wire.endTransmission () == 0) );
}

void LCDDisp::clear() {
  writeLCD( cLCD_CMD, CMD_CLS );      // Befehl: Display löschen
  delay( LCD_DELAY_MS_CLEAR );
}

void LCDDisp::home() {
  writeLCD( cLCD_CMD, CMD_HOME );     // Cursor in erste Zeile und erste Spalte
  delay( LCD_DELAY_MS_HOME );
}

void LCDDisp::putChar( uint8_t c ) {
  writeLCD( cLCD_DATA, translate( c ) );
}


void LCDDisp::putNumber( uint8_t number, uint8_t len, uint8_t fill ) {
    uint8_t digit1 = 0;
    uint8_t digit2 = 0;
    while (number >= 100) {
        digit1++;
        number -= 100;
    }
    while (number >= 10) {
        digit2++;
        number -= 10;
    }
    if (len > 2) putChar( (digit1 != 0) ? digit1+'0' : fill );
    if (len > 1) putChar( ((digit1 != 0) || (digit2 != 0)) ? digit2+'0' : fill );
    putChar( number+'0' );  
}


void LCDDisp::putString( const char *data ) {
  while ( *data != '\0') {
    putChar( *data++ );
  } 
}

void LCDDisp::moveCursorTo( uint8_t col, uint8_t row ) {
  uint8_t MemPos;
  col = col % cols();    //Obergrenze beachten
  row = (row % rows()) & 0x03;
  MemPos = (col + DD_RAM_Offsets[ row ]) & 0x7F;
  writeLCD( cLCD_CMD, CMD_SET_DD_RAM_ADR | MemPos );
}

//  wird erst mit dem nächsten Schreibbefehl auf das Dispaly ausgeführt
void LCDDisp::backLightOn() {
  _pcf.bits.BL = 1;
}
void LCDDisp::backLightOff(){
  _pcf.bits.BL = 0;
}

uint8_t LCDDisp::isBacklightOn(){
  return _pcf.bits.BL;
}

void LCDDisp::cursorOn(){
  _reg.bits.C = 1;
  writeLCD( cLCD_CMD, getCMD_Display() );   //set Cursor mode and display on/off  
}
void LCDDisp::cursorOff(){
  _reg.bits.C = 0;
  writeLCD( cLCD_CMD, getCMD_Display() );   //set Cursor mode and display on/off  
}
void LCDDisp::cursorBlink(){
  _reg.bits.B = 1;
  writeLCD( cLCD_CMD, getCMD_Display() );   //set Cursor mode and display on/off  
}
void LCDDisp::cursorBlinkOff(){
  _reg.bits.B = 0;
  writeLCD( cLCD_CMD, getCMD_Display() );   //set Cursor mode and display on/off  
}
void LCDDisp::displayOn(){
  _reg.bits.D = 1;
  writeLCD( cLCD_CMD, getCMD_Display() );   //set Cursor mode and display on/off  
}
void LCDDisp::displayOff(){
  _reg.bits.D = 0;
  writeLCD( cLCD_CMD, getCMD_Display() );   //set Cursor mode and display on/off  
}

void LCDDisp::display( uint8_t c ) {
  if ((c & CMD_CTRL_ON) > 0)           { _reg.bits.D = 1; } else { _reg.bits.D = 0; }
  if ((c & CMD_CTRL_CURSOR_ON) > 0)    { _reg.bits.C = 1; } else { _reg.bits.C = 0; }
  if ((c & CMD_CTRL_CURSOR_BLINK) > 0) { _reg.bits.B = 1; } else { _reg.bits.B = 0; }
  writeLCD( cLCD_CMD, getCMD_Display() );   //set Cursor mode and display on/off  all together with one write access
}

void LCDDisp::shiftCursor( uint8_t dir ){
 if (dir == 0) { 
    _reg.bits.SC = 0;  _reg.bits.RL = 0;
 } else { 
    _reg.bits.SC = 0;  _reg.bits.RL = 1;
 }
  writeLCD( cLCD_CMD, getCMD_Shift( 0 ) ); 
}

void LCDDisp::shiftDisplay( uint8_t dir ){
 if (dir == 0) { 
    _reg.bits.SC = 1;  _reg.bits.RL = 0;
 } else { 
    _reg.bits.SC = 1;  _reg.bits.RL = 1;
 }
  writeLCD( cLCD_CMD, getCMD_Shift( 0 ) ); 
}

uint8_t LCDDisp::cols() {
  return _col;
}

uint8_t LCDDisp::rows() {
  return _row;
}

void LCDDisp::init() {
  delay( LCD_DELAY_MS_STARTUP );                    // > 15ms Pause
  write4Bit( LCD_INIT_VECT );   // muss 3x hintereinander gesendet werden -> Datenblatt
  delay( LCD_DELAY_MS_INIT1 );
  write4Bit( LCD_INIT_VECT );
  delay( LCD_DELAY_MS_INIT2 );
  write4Bit( LCD_INIT_VECT );
  delay( LCD_DELAY_MS_INIT3 );

  //Das ist bei einer 4-Bit-Initialisierung erforderlich! 
  //siehe http://www.mikrocontroller.net/articles/Erweiterte_LCD-Ansteuerung
  write4Bit( getCMD_Function() );     // Befehl: Function set schaltet auf 4-Bit-Modus um
  delay( 5 );

  // jetzt beginnt die eigentliche Initialisierung
  writeLCD( cLCD_CMD, getCMD_Function() );  //now: set nr of lines, font and again: interface width 
  writeLCD( cLCD_CMD, getCMD_Display() );   //set Cursor mode and display on/off
  writeLCD( cLCD_CMD, CMD_CLS );            //clear screen
  delay( LCD_DELAY_MS_CLEAR );              //braucht > 1,64 ms
  writeLCD( cLCD_CMD, getCMD_Mode() );      //Cursor- and Display movement
  delay( 2 );
  
  createChar(0, Auml);   //define addditional char maps
  createChar(1, Uuml);
  createChar(2, Ouml);          
 }  

uint8_t LCDDisp::getCMD_Clear() {
  return CMD_CLS;
}

uint8_t LCDDisp::getCMD_Home() {
  return CMD_HOME;
}
  
uint8_t LCDDisp::getCMD_Mode() {
  uint8_t x = CMD_MODE;
  if (_reg.bits.ID) { x |= CMD_MODE_INC; }
  if (_reg.bits.SH) { x |= CMD_MODE_SHIFT; }
  return x; 
}
uint8_t LCDDisp::getCMD_Display() {
  uint8_t x = CMD_CTRL; //set display an cursor behaviour
  if (_reg.bits.D) { x |= CMD_CTRL_ON; }
  if (_reg.bits.C) { x |= CMD_CTRL_CURSOR_ON; }
  if (_reg.bits.B) { x |= CMD_CTRL_CURSOR_BLINK; }
  return x; 
}
uint8_t LCDDisp::getCMD_Shift( uint8_t width ) {
  uint8_t x = CMD_SHIFT;
  if (_reg.bits.SC) { x |= CMD_SHIFT_DISP; }
  if (_reg.bits.RL) { x |= CMD_SHIFT_RIGHT; }
  x |= (width & 0x03);  //only bit1 and bit 0 are relevant 
  return x;
}
uint8_t LCDDisp::getCMD_Function() {
  uint8_t x = CMD_FUNC_SET;
  if (_reg.bits.DL) { x |= CMD_FUNC_8BIT; }
  if (_reg.bits.N)  { x |= CMD_FUNC_2LINE; }
  if (_reg.bits.F)  { x |= CMD_FUNC_10DOTS; }
  return x;  
}
  

/*
 * Nur das High-Nibble schreiben
 */
void LCDDisp::write4Bit( uint8_t data ) {
  _pcf.bits.RS = LCD_COMMAND;     //RS = 0
  _pcf.bits.RW = LCD_WRITE;       //RW = 0  (Write)
  _pcf.bits.data = (data >> 4);   //only High-nibble 
 
  pcf8574_write( );               //write to the port
  triggerLCD( );                  //generate Enable-Puls
}

void LCDDisp::writeLCD( enum TLCDREG reg, uint8_t data ) {
  if (reg == cLCD_CMD) {          //-Befehlsregister
    _pcf.bits.RS = LCD_COMMAND;   //RS = 0
  } else {                        //Datenregister
    _pcf.bits.RS = LCD_DATA;      //RS = 1
  }
  _pcf.bits.RW = LCD_WRITE;       //RW = 0  (Write)

  _pcf.bits.data = ((0xF0 & data) >> 4);    //High Nibble of data first
  pcf8574_write( );                         //write to the port
  triggerLCD( );                            //create Enable-Pulse

  //- Low-Nibble schreiben
  _pcf.bits.data = ((0x0F & data));         //Low Nibble of data second
  pcf8574_write( );                         //write to the port
  triggerLCD( );                            //create Enable-Pulse
}

void LCDDisp::triggerLCD( ) { //per Scope gemessen:  750us  mit LCD_DELAY_US_ENABLE    480us ohne LCD_DELAY_US_ENABLE
  _pcf.bits.EN = 1;     //EN = 1
  pcf8574_write( );     //und auf den Port schreiben -> Impuls auf High
  //delayMicroseconds( LCD_DELAY_US_ENABLE );     //etwas warten, das brauchen wir bei I2C nicht
  _pcf.bits.EN = 0;     //EN = 0
  pcf8574_write( );     //und auf den Port schreiben -> Impuls auf High
}


uint8_t LCDDisp::pcf8574_write( ) {
  Wire.beginTransmission( _i2cAdr );    // transmit to device #...
  Wire.write( _pcf.all );               // 
  uint8_t x = Wire.endTransmission();   // stop transmitting
  //if (x != 0) {  Serial.print("err> "); Serial.println( x );  }
  return x;
}


//Ob dieses setsame Verhalten mit UTF-8 Kodierung des quelltextes zusammenhängt?
//statt 164  sollte die Kodierung für ein ä eigentlich 228 sein
uint8_t LCDDisp::translate( uint8_t ascii ) {
  switch (ascii) {
      case 164: return  225; break; // ä  statt 228
      case 182: return  239; break; // ö  statt 246
      case 188: return  245; break; // ü  statt 252
      case 159: return  226; break; // ß  statt 223
      case 132: return  0; break;   // Ä  statt 196
      case 150: return  2; break;   // Ö  statt 214
      case 156: return  1; break;   // Ü  statt 220
      case 176: return  223; break; // grad  statt  176
      default:  return  ascii; break;
  }      
}


//Allows us to fill the first 8 CGRAM locations
//with custom characters
//We use the first 3 of them for Ä, Ö and Ü
void LCDDisp::createChar(uint8_t location, uint8_t charmap[]) {
  location &= 0x7; // we only have 8 locations 0-7
  writeLCD( cLCD_CMD, CMD_SET_CG_RAM_ADR | (location << 3) );
  for (int i=0; i<8; i++) {
    writeLCD( cLCD_DATA, charmap[i] );
  }
}





