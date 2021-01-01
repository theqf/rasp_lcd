#ifndef __LCD_INIT_H
#define __LCD_INIT_H
#include <wiringPi.h>
#include <stdint.h>

#define USE_HORIZONTAL 2  //设置横屏或者竖屏显示 0或1为竖屏 2或3为横屏

#define LCD80160

#ifdef LCD80160
#if USE_HORIZONTAL==0||USE_HORIZONTAL==1
#define LCD_W 80
#define LCD_H 160
#else
#define LCD_W 160
#define LCD_H 80
#endif
#else
#define LCD_W 240
#define LCD_H 240
#endif

extern const int LCD_CLK;
extern const int LCD_MOSI;
extern const int LCD_RES;
extern const int LCD_DC;
extern const int LCD_CS;
extern const int LCD_BLK;


#define LCD_SCLK_Clr() digitalWrite(LCD_CLK,LOW);
#define LCD_SCLK_Set() digitalWrite(LCD_CLK,HIGH);

#define LCD_SDIN_Clr() digitalWrite(LCD_MOSI,LOW);
#define LCD_SDIN_Set() digitalWrite(LCD_MOSI,HIGH);

#define LCD_RES_Clr()  digitalWrite(LCD_RES,LOW);
#define LCD_RES_Set()  digitalWrite(LCD_RES,HIGH);

#define LCD_DC_Clr()   digitalWrite(LCD_DC,LOW);
#define LCD_DC_Set()   digitalWrite(LCD_DC,HIGH);
 		     
#define LCD_CS_Clr()   digitalWrite(LCD_CS,LOW);
#define LCD_CS_Set()   digitalWrite(LCD_CS,HIGH);

#define LCD_BLK_Clr()  digitalWrite(LCD_BLK,LOW);
#define LCD_BLK_Set()  digitalWrite(LCD_BLK,HIGH);

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
#ifdef __cplusplus
extern "C" {
#endif
void init_gpio();
void LCD_Writ_Bus(u8 dat);
void LCD_WR_DATA8(u8 dat);
void LCD_WR_DATA(u16 dat);
void LCD_WR_REG(u8 dat);
void LCD_Address_Set(u16 x1, u16 y1, u16 x2, u16 y2);
void LCD_Init(void);
#ifdef __cplusplus
}
#endif
#endif


