#include <iostream>
#include <wiringPi.h>
#include <functional>
#include "LCD.h"
#include "LCD_Draw.h"
#include "JpegReader.h"
#include "FFmpegGetFile.h"
#include "H264Decoder.h"

const int LCD_CLK = 1;
const int LCD_MOSI = 2;
const int LCD_RES = 3;
const int LCD_DC = 4;
const int LCD_CS = 5;
const int LCD_BLK = 6;

extern "C" void delay_ms(int ms)
{
    delay(ms);
}

int main(int argc,char **argv) {
    int ret = wiringPiSetup();
    if (ret < 0) {
        std::cout << "wiringPiSetup err "<< ret << std::endl;
    }
    if (argc != 2){
        printf("rasp_lcd test.jpg");
        return 0;
    }
    FFmpegGetFile getFile;

    H264Decoder h264Decoder;
    h264Decoder.init(true, LCD_W, LCD_H);

    getFile.setH264OutCallBack(std::bind(&H264Decoder::videoDecode, &h264Decoder,
            std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));

    init_gpio();
    LCD_Init();
    LCD_Fill(0,0,LCD_W,LCD_H,WHITE);
    JpegReader jpegReader;

    h264Decoder.setRGBCallBack( [&](void*, uint8_t * data, int w, int h, uint32_t){
        jpegReader.bit24tobit16(data,w,h);
        LCD_ShowPicture(0,0, h, w, data);
    });

    getFile.start(argv[1]);
    getFile.join();
    return 0;
}
