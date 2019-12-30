#ifndef LCDDisp_H
#define LCDDisp_H

/* Quellen:
 * http://html.szaktilla.de/arduino/6.html
 * http://www.netzmafia.de/skripten/hardware/Arduino/LCD/index.html
 * https://www.heise.de/developer/artikel/Arduino-IoT-Extra-Bibliotheken-selbst-implementieren-3266419.html
 * https://www.mikrocontroller.net/articles/Erweiterte_LCD-Ansteuerung
 */


#include "Arduino.h"

#define PCF8574A_BASE     0x38  //There are two different versions of the device available
#define PCF8574_BASE      0x20  //with different base adresses
                                //without Bit0 (Rd/Wr), in Atmel Studio use 0x50  ->  0x50 = (0x28 << 1)  or 0x70 = 0x38 << 1

#define PCF8574_DEVICE_0  0x00  //Add this offset, if you use other adresses 
#define PCF8574_DEVICE_1  0x02
#define PCF8574_DEVICE_2  0x04
#define PCF8574_DEVICE_3  0x06
#define PCF8574_DEVICE_4  0x08
#define PCF8574_DEVICE_5  0x0A
#define PCF8574_DEVICE_6  0x0C
#define PCF8574_DEVICE_7  0x0E

#define LCD_INIT_VECT           0x30  //Der Wert muss aus dem Datenblatt übernommen werden. Typisch ist 0x30    alt: 0x38

#define CMD_CLS                 0x01  //clear display 
#define CMD_HOME                0x02  //set Cursor to upper left corner
#define CMD_MODE                0x04  //Set Cursor properties: 
#define CMD_MODE_INC            0x02  //  inc cursor position after write
#define CMD_MODE_SHIFT          0x01  //  shift display content

#define CMD_CTRL                0x08    //Display / Cursorverhalten festlegen
#define CMD_CTRL_ON             0x04    
#define CMD_CTRL_CURSOR_ON      0x02
#define CMD_CTRL_CURSOR_BLINK   0x01

#define CMD_SHIFT               0x10  // Moving display oder cursor to left or right 
#define CMD_SHIFT_DISP          0x08
#define CMD_SHIFT_RIGHT         0x04
#define CMD_SHIFT_LEFT          0x00

#define CMD_FUNC_SET            0x20  // init the display
#define CMD_FUNC_8BIT           0x10  // 8-bit databus
#define CMD_FUNC_2LINE          0x08  // 1 or 2 line display
#define CMD_FUNC_10DOTS         0x04  // font:  5*10 dots  

#define CMD_SET_CG_RAM_ADR      0x40
#define CMD_SET_DD_RAM_ADR      0x80


#define CTRL_BIT_ID 0x0001      //Cursor movement: increase
#define CTRL_BIT_SH 0x0002      //Display is shifted
#define CTRL_BIT_D  0x0004      //Display on
#define CTRL_BIT_C  0x0008      //Cursor on
#define CTRL_BIT_B  0x0010      //Cursor blinking on
#define CTRL_BIT_SC 0x0020      //Display shift
#define CTRL_BIT_RL 0x0040      //Right shift
#define CTRL_BIT_DL 0x0080      //8 bit inetrface
#define CTRL_BIT_N  0x0100      //2-Line-Display
#define CTRL_BIT_F  0x0200      //Font 5x10
#define CTRL_INIT   (CTRL_BIT_ID | CTRL_BIT_D | CTRL_BIT_C | CTRL_BIT_N)

typedef struct {
  unsigned ID:1;        //Set cursor move   1=increase          0=decrease
  unsigned SH:1;        //Display shift     1=shift             0=no shift       
  unsigned D:1;         //Display           1=On                0=Off
  unsigned C:1;         //Cursor            1=On                0=Off
  unsigned B:1;         //Cursor blink      1=blinking on       0=blinking off      
  unsigned SC:1;        //Display Shift     1=Display shift     0=Cursor move
  unsigned RL:1;        //Sift Direction    1=Right shift       0=Left Shift
  unsigned DL:1;        //Interface         1=8 Bit mode        0=4 Bit mode  
  unsigned N:1;         //Nr of Lines       1=2 line display    0=1 line display
  unsigned F:1;         //Font              1=5x10 dots         0=5x7 dots
  unsigned data:4;      //Nibble at the data lines D4..D8 of the HD44780
  unsigned na:2;        //not used
} TControlBits;

union TControl {
  uint16_t      all;
  TControlBits  bits;
};

#define LCD_READ          1     //Signal RW
#define LCD_WRITE         0
#define LCD_ENABLE        1     //Signal E
#define LCD_DISABLE       0
#define LCD_DATA          1     //Signal RS
#define LCD_COMMAND       0

enum  TLCDREG { cLCD_CMD,     //Register für Befehlseingabe adressieren
                cLCD_DATA };  //Register für Daten adressieren

typedef struct {
  unsigned data:4;    //Low-nibble contains data
  unsigned EN:1;      //Enable-Signal
  unsigned RW:1;      //RW-Signal   1=Read    0=Write
  unsigned RS:1;      //RS-Signal   1=Data    0=Control
  unsigned BL:1;      //Backlight   1=On      0=Off
} TPCFBits;

union TPCF {
  uint8_t  all;
  TPCFBits bits;
};


//- Verschiedene Wartezeiten zur Befehlsausführung
#define LCD_DELAY_US_ENABLE_SETUP 50      //Wartezeit, bevor Enable aktiviert wird
#define LCD_DELAY_US_ENABLE       250     //Wie lange der Enable-Impuls in us ist
#define LCD_DELAY_US_WRITE        5   
#define LCD_DELAY_MS_STARTUP      50      //wait for power supply to become stable
#define LCD_DELAY_MS_INIT1        5       //1. step of init sequence
#define LCD_DELAY_MS_INIT2        1       //2. step of init sequence
#define LCD_DELAY_MS_INIT3        1       //3. step of init sequence 
#define LCD_DELAY_MS_HOME         2       //delay in ms after home-command
#define LCD_DELAY_MS_CLEAR        2       //delay in ms after clear-command

class LCDDisp 
{
  public:
   /**
   * Constructor
   *
   * @param pinSCL    Pin, to which the SCL-Signl is connected to (ESP only, ignored by ATmega328)
   * @param pinSDA    Pin, to which the SDA-Signl is connected to (ESP only, ignored by ATmega328)
   * @param i2cAddr   I2C slave address of the LCD display. Most likely printed on the
   *                  LCD circuit board, or look in the supplied LCD documentation.
   * @param displayTyp  the used display. This can be one of the following values:
   *                    DT8x1, DT8x2, DT16x1, DT16x2, DT16x4, DT20x2, DT20x4, DT40x2
   */
   LCDDisp( uint8_t pinSCL, uint8_t pinSDA, uint8_t i2cAdr, uint8_t cols, uint8_t rows );

  /**
   * @brief Set the LCD display in the correct begin state, must be called before anything else is done.
   */
  void begin();
  /**
   * @brief Check, wether the I2C-device is available or not
   */
  bool avail();
  /**
   * @brief Clears the display and set the cursor to the home-position (upper left corner)
   */
  void clear();
  /**
   * @brief Move the cursor to the home-position (upper left corner)
   */
  void home();
  /**
   * @brief Get the number of colums of the display
   * @return nr of colums (typ. 8..20)
   */
  uint8_t cols();
  /**
   * @brief Get the number of rows of the display
   * @return nr of rows (typ. 1..4)   */
  uint8_t rows();

  /**
   * @brief Turn backlight on
   */
  void backLightOn();
  /**
   * @brief Turn backlight off
   */
  void backLightOff();
  /**
   * @brief Check, whether  backlight is on or off
   * @return  0 = backlight is off
   *          1 = bachlight is on 
   */
  uint8_t isBacklightOn();
  /**
   * @brief Turn cursor on. Cursor is a underline.
   */
  void cursorOn();
  /**
   * @brief Turn cursor off.
   */
  void cursorOff();
  /**
   * @brief Switch cursor to a blinking block, if switched on
   */
  void cursorBlink();
  /**
   * @brief Switch cursor to a underline, if switched on
   */
  void cursorBlinkOff();
  /**
   * Switch display on
   */
  void displayOn();
  /**
   * Switch display off.
   */
  void displayOff();
  /**
   * @brief This is a method to combine the requests concerning display an cursor
   *        To set more than one option with one call use
   *        display( CMD_CTRL_ON | CMD_CTRL_CURSOR_ON | CMD_CTRL_CURSOR_BLINK );
   *        
   *        Any subset of 
   *          CMD_CTRL_ON
   *          CMD_CTRL_CURSOR_ON
   *          CMD_CTRL_CURSOR_BLINK
   *        is possible.  
   *        
   *        A call to display with 0 will result in: Display OFF, Cursor OFF 
   */
  void display( uint8_t c );

  /**
   * @brief Shift the cursor one position in the requestet direction
   * @param dir CMD_SHIFT_RIGHT = shift one position to the right
   *            CMD_SHIFT_LEFT  = shift one position to the left
   */
  void shiftCursor( uint8_t dir );
  /**
   * @brief Shift the display content one position in the requestet direction
   * @param dir CMD_SHIFT_RIGHT = shift one position to the right
   *            CMD_SHIFT_LEFT  = shift one position to the left
   */
  void shiftDisplay( uint8_t dir );
  /**
   * @brief  Set the cursor to any location inside the DD-RAM.            
   *         The location is adressed by column (col) ans row
   * @param col: The colums 0..39    
   * @param row: The row  0..3
  */  
  void moveCursorTo( uint8_t col, uint8_t row );
  
  /**    
   * @brief  The method places the character c at the actual cursor position 
   *         an moves the cursor to the next position
   *     
   * @param c: The char, see ASCII-Tabelle and datasheet
  */
  void putChar( uint8_t c );
  /**
   * @brief Writes the String data to the display, starting at the actual cursor location
   *        String is c-like an must end with an \0 as End-Of-Line
   * @param data  Pointer to an array of the containing the string.
   */
  void putString( const char *data );
  /**
   * @brief The value of numer is written to the display at the actual cursor position.
   * @param number  The value to display
   * @param len     The number of digits
   * @param fill    The filling char, if the number does not need the len places
   */
  void putNumber( uint8_t number, uint8_t len, uint8_t fill );

  /**
   * @brief The lcd controller allows the user to define 8 different characters, 
   *        locatet at position 0..7 in display character RAM.
   *        This method can be used to define these chars.
   *        The function is used here to define the missing german mutated vowel ÄÖÜ
   *        The lower case letters are already define in the character generator rom.
   *        
   * @param location The adress of the new char. Must be 0..7
   * @param charmap an array of 8 byte, holding the bit pattern of the new char.
   *        see http://www.netzmafia.de/skripten/hardware/Arduino/LCD/index.html      
   *        for more details (at the end of the chapter)
   * 
   */
  void createChar(uint8_t location, uint8_t charmap[]);
  
  private:
    //- data, provied by constructor:
    uint8_t     _pinSCL;      //beim UNO fest auf A5, beim ESP variabel
    uint8_t     _pinSDA;      //beim UNO fest auf A4, beim ESP variabel
    uint8_t     _i2cAdr;      //adress of the device
    uint8_t     _col;         //Nr of columns
    uint8_t     _row;         //Nr of rows

    //- internal
    union TControl  _reg;     //register of the HD44780
    union TPCF      _pcf;     //High-Nibble = Control-Bits, Low-Nibble = Data-nibble
    

    //-- alt:
//    uint8_t     _dispCtrl;    
//    uint8_t     _cursor;
//    uint8_t     _lcdCtrlData; //


    void    init();
    uint8_t getCMD_Clear();
    uint8_t getCMD_Home();
    uint8_t getCMD_Mode();
    uint8_t getCMD_Display();
    uint8_t getCMD_Shift( uint8_t width );
    uint8_t getCMD_Function();
    
    void    write4Bit( uint8_t data ) ;                   //write High-Nibble only to CONTROL-Register
    void    writeLCD( enum TLCDREG reg, uint8_t data ); 
    /**
    @brief  Create a triggerpuls at the pin EN (Enable) of the display
            Internal use.
    */
    void    triggerLCD( );
    /** 
     * @brief  Low-Level Routines to write to PCF8574
     */
    uint8_t pcf8574_write( );


    /** 
     *  @brief  Functions for using german special chars like äöüßÄÖÜ
     *          The patterns for äöü and ß are already located at the caracter rom, but on different places
     *          The followinf function will translate this positions
     *          Details see:  http://www.netzmafia.de/skripten/hardware/Arduino/LCD/index.html
     * @param   the char bevor translation         
     * @return  the translated char
     */
     uint8_t translate( uint8_t ascii );
};









#endif      //LCDDisp_H
