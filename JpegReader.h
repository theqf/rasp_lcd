//
// Created by wqwd on 2020/12/26.
//

#ifndef RASP_LCD_JPEGREADER_H
#define RASP_LCD_JPEGREADER_H
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "jpeglib.h"
#include <arpa/inet.h>
class JpegReader{
private:
    void j_putRGBScanline(uint8_t *jpegline,int widthPix, uint8_t *outBuf,int row)
    {
        int offset = row * widthPix * 3;
        int count;
        for (count=0;count < row;count++)
        {
            *(outBuf + offset + count * 3 + 0) = *(jpegline + count * 3 + 0);
            *(outBuf + offset + count * 3 + 1) = *(jpegline + count * 3 + 1);
            *(outBuf + offset + count * 3 + 2) = *(jpegline + count * 3 + 2);
        }
    }

    void j_putGrayScanlineToRGB(uint8_t *jpegline,int widthPix,uint8_t *outBuf,int row)
    {
        int offset = row * widthPix * 3;
        int count;
        for (count = 0;count < row; count++)
        {
            uint8_t iGray;
            // get our grayscale value
            iGray = *(jpegline + count);
            *(outBuf + offset + count * 3 + 0) = iGray;
            *(outBuf + offset + count * 3 + 1) = iGray;
            *(outBuf + offset + count * 3 + 2) = iGray;
        }
    }
public:
    int read_jpeg_file(const char *input_filename, uint8_t **out_buffer, int* pic_w, int* pic_h) {
#if 1
        struct jpeg_decompress_struct cinfo;//JPEG图像在解码过程中，使用jpeg_decompress_struct类型的结构体来表示，图像的所有信息都存储在结构体中
        struct jpeg_error_mgr jerr;//定义一个标准的错误结构体，一旦程序出现错误就会调用exit()函数退出进程
        FILE *input_file;
        input_file=fopen(input_filename,"rb");

        cinfo.err = jpeg_std_error(&jerr);//绑定错误处理结构对象

        jpeg_create_decompress(&cinfo);//初始化cinfo结构
        jpeg_stdio_src(&cinfo,input_file);//指定解压缩数据源
        jpeg_read_header(&cinfo,TRUE);//获取文件信息
        jpeg_start_decompress(&cinfo);//开始解压缩

        unsigned long width = cinfo.output_width;//图像宽度
        unsigned long height = cinfo.output_height;//图像高度
        *pic_w = width;
        *pic_h = height;
        unsigned short depth = cinfo.output_components;//图像深度

        unsigned char *src_buff;//用于存取解码之后的位图数据(RGB格式)
        src_buff = (unsigned char *)malloc(width * height * depth);//分配位图数据空间
        memset(src_buff, 0, sizeof(unsigned char) * width * height * depth);//清0

        JSAMPARRAY buffer;//用于存取一行数据
        buffer = (*cinfo.mem->alloc_sarray)((j_common_ptr)&cinfo, JPOOL_IMAGE, width*depth, 1);//分配一行数据空间

        unsigned char *point = src_buff;
        while(cinfo.output_scanline < height)//逐行读取位图数据
        {
            jpeg_read_scanlines(&cinfo, buffer, 1);    //读取一行jpg图像数据到buffer
            memcpy(point, *buffer, width*depth);    //将buffer中的数据逐行给src_buff
            point += width * depth;            //指针偏移一行
        }

        jpeg_finish_decompress(&cinfo);//解压缩完毕
        jpeg_destroy_decompress(&cinfo);// 释放资源
        *out_buffer = src_buff;
        fclose(input_file);

        return 0;
#else
        struct jpeg_decompress_struct cinfo;
        struct jpeg_error_mgr jerr;
        FILE *input_file;
        JSAMPARRAY buffer;
        int row_stride;

        uint8_t *output_buffer = NULL;
        unsigned char *tmp = NULL;
        cinfo.err = jpeg_std_error(&jerr);
        if ((input_file = fopen(input_filename, "rb")) == NULL) {
            fprintf(stderr, "can't open %s\n", input_filename);
            return -1;
        }

        // Initialization of JPEG compression objects
        jpeg_create_decompress(&cinfo);
        /* Specify data source for decompression */
        jpeg_stdio_src(&cinfo, input_file);
        /* 1.设置读取jpg文件头部，Read file header, set default decompression parameters */
        (void) jpeg_read_header(&cinfo, TRUE);
        /* 2.开始解码数据 Start decompressor */
        (void) jpeg_start_decompress(&cinfo);

        row_stride = cinfo.output_width * cinfo.output_components;

        *pic_w = cinfo.output_width;
        *pic_h = cinfo.output_height;

        /* 3.跳过读取的头文件字节Make a one-row-high sample array that will go away when done with image */
        buffer = (*cinfo.mem->alloc_sarray)
                ((j_common_ptr) & cinfo, JPOOL_IMAGE, row_stride, 1);

        output_buffer = (uint8_t *) malloc(cinfo.output_width * cinfo.output_height * 3);
        memset(output_buffer, 0, cinfo.output_width * cinfo.output_height * 3);
        tmp = output_buffer;

        /* 4.Process data由左上角从上到下行行扫描 */
        while (cinfo.output_scanline < cinfo.output_height) {
            (void) jpeg_read_scanlines(&cinfo, buffer, 1);
            // asuumer all 3-components are RGBs
            if (cinfo.out_color_components==3) {
                j_putRGBScanline(buffer[0],
                                 *pic_w,
                                 output_buffer,
                                 cinfo.output_scanline-1);
            } else if (cinfo.out_color_components==1) {
                // assume all single component images are grayscale
                j_putGrayScanlineToRGB(buffer[0],
                                       *pic_w,
                                       output_buffer,
                                       cinfo.output_scanline-1);
            }
        }


        (void) jpeg_finish_decompress(&cinfo);
        jpeg_destroy_decompress(&cinfo);
        /* Close files, if we opened them */
        fclose(input_file);
        *out_buffer = output_buffer;
        return 0;
#endif
    }

    void freep(uint8_t **out_buffer)
    {
        free(*out_buffer);
        *out_buffer = NULL;
    }

    void bit24tobit16(uint8_t *in_out_buffer, int pic_w, int pic_h){
        int sou_len = 0;
        int dest_len = 0;
        for (int i = 0; i < pic_w * pic_h; ++i) {
            uint8_t* sou_data = in_out_buffer + sou_len;
            uint8_t* dest_data = in_out_buffer + dest_len;

            uint8_t dest[2] = {0};
            dest[0] = (sou_data[0] & 0xf8);
            dest[0] |= ((sou_data[1] & 0xfc)>>5);
            dest[1] |= ((sou_data[1] & 0x1c)<<3);
            dest[1] |= ((sou_data[2] & 0xf8)>>3);
            dest_data[0] = dest[0];
            dest_data[1] = dest[1];

            sou_len += 3;
            dest_len += 2;
        }
    }

    void bit32tobit16(uint8_t *in_out_buffer, int pic_w, int pic_h){
        int sou_len = 0;
        int dest_len = 0;
        for (int i = 0; i < pic_w * pic_h; ++i) {
            uint8_t* sou_data = in_out_buffer + sou_len;
            uint8_t* dest_data = in_out_buffer + dest_len;

            uint8_t dest[2] = {0};

            dest[0] = (sou_data[2] & 0xf8);
            dest[0] |= ((sou_data[1] & 0xfc)>>5);
            dest[1] |= ((sou_data[1] & 0x1c)<<3);
            dest[1] |= ((sou_data[0] & 0xf8)>>3);

            dest_data[0] = dest[0];
            dest_data[1] = dest[1];

            sou_len += 4;
            dest_len += 2;
        }
    }
};


#endif //RASP_LCD_JPEGREADER_H
