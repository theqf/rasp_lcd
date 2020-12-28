#include "SpsParser.h"

SpsParser::SpsParser()
{

}

SpsParser::~SpsParser()
{

}


unsigned int SpsParser::Ue(char *pBuff, unsigned int nLen, unsigned int &nStartBit)
{
    unsigned int nZeroNum = 0;
    while (nStartBit < nLen * 8)
    {
        if (pBuff[nStartBit / 8] & (0x80 >> (nStartBit % 8)))
        {
            break;
        }
        nZeroNum++;
        nStartBit++;
    }
    nStartBit ++;


    int dwRet = 0;
    for (unsigned int i=0; i < nZeroNum; i++)
    {
        dwRet <<= 1;
        if (pBuff[nStartBit / 8] & (0x80 >> (nStartBit % 8)))
        {
            dwRet += 1;
        }
        nStartBit++;
    }
    return (1 << nZeroNum) - 1 + dwRet;
}


int SpsParser::Se(char *pBuff, unsigned int nLen, unsigned int &nStartBit)
{
	int UeVal = Ue(pBuff,nLen,nStartBit);
	double k = UeVal;
	int nValue = ceil(k/2);
	if (UeVal % 2==0)
		nValue = -nValue;
	return nValue;
}


int SpsParser::u(unsigned int BitCount,char * buf,unsigned int &nStartBit)
{
    int dwRet = 0;
    for (unsigned int i=0; i < BitCount; i++)
    {
        dwRet <<= 1;
        if (buf[nStartBit / 8] & (0x80 >> (nStartBit % 8)))
        {
            dwRet += 1;
        }
        nStartBit++;
    }
	return dwRet;
}


int SpsParser::h264_decode_seq_parameter_set(char * buf,unsigned int nLen,int &Width,int &Height)
{
	int forbidden_zero_bit, nal_ref_idc, nal_unit_type;
	int profile_idc, constraint_set0_flag, constraint_set1_flag, constraint_set2_flag, constraint_set3_flag;
	int reserved_zero_4bits, level_idc, seq_parameter_set_id;
	int chroma_format_idc, residual_colour_transform_flag, bit_depth_luma_minus8, bit_depth_chroma_minus8;
	int qpprime_y_zero_transform_bypass_flag, seq_scaling_matrix_present_flag, seq_scaling_list_present_flag[8];
	int log2_max_frame_num_minus4, pic_order_cnt_type, log2_max_pic_order_cnt_lsb_minus4;
	int delta_pic_order_always_zero_flag, offset_for_non_ref_pic, offset_for_top_to_bottom_field;
	int num_ref_frames_in_pic_order_cnt_cycle, *offset_for_ref_frame, num_ref_frames;
	int gaps_in_frame_num_value_allowed_flag, pic_width_in_mbs_minus1, pic_height_in_map_units_minus1;

	
	unsigned int StartBit = 0;
	forbidden_zero_bit = u(1,buf,StartBit);
	nal_ref_idc = u(2,buf,StartBit);
	nal_unit_type = u(5,buf,StartBit);

	if(nal_unit_type == 7)
	{
		profile_idc = u(8,buf,StartBit);
		constraint_set0_flag = u(1,buf,StartBit);//(buf[1] & 0x80)>>7;
		constraint_set1_flag = u(1,buf,StartBit);//(buf[1] & 0x40)>>6;
		constraint_set2_flag = u(1,buf,StartBit);//(buf[1] & 0x20)>>5;
		constraint_set3_flag = u(1,buf,StartBit);//(buf[1] & 0x10)>>4;
		reserved_zero_4bits = u(4,buf,StartBit);

		level_idc = u(8,buf,StartBit);
		seq_parameter_set_id = Ue(buf,nLen,StartBit);

		if( profile_idc == 100 || profile_idc == 110 ||
				profile_idc == 122 || profile_idc == 144 )
		{
			chroma_format_idc = Ue(buf,nLen,StartBit);
			if( chroma_format_idc == 3 )
				residual_colour_transform_flag = u(1,buf,StartBit);

			bit_depth_luma_minus8 = Ue(buf,nLen,StartBit);
			bit_depth_chroma_minus8 = Ue(buf,nLen,StartBit);
			qpprime_y_zero_transform_bypass_flag = u(1,buf,StartBit);
			seq_scaling_matrix_present_flag = u(1,buf,StartBit);

			
			if( seq_scaling_matrix_present_flag )
			{
					for( int i = 0; i < 8; i++ )
					{
						seq_scaling_list_present_flag[i] = u(1,buf,StartBit);
					}
			}
		}

		log2_max_frame_num_minus4 = Ue(buf,nLen,StartBit);
		pic_order_cnt_type = Ue(buf,nLen,StartBit);
		if( pic_order_cnt_type == 0 )
			log2_max_pic_order_cnt_lsb_minus4 = Ue(buf,nLen,StartBit);
		else if( pic_order_cnt_type == 1 )
		{
			delta_pic_order_always_zero_flag = u(1,buf,StartBit);
			offset_for_non_ref_pic = Se(buf,nLen,StartBit);
			offset_for_top_to_bottom_field = Se(buf,nLen,StartBit);
			num_ref_frames_in_pic_order_cnt_cycle = Ue(buf,nLen,StartBit);

			offset_for_ref_frame = new int[num_ref_frames_in_pic_order_cnt_cycle];
			for( int i = 0; i < num_ref_frames_in_pic_order_cnt_cycle; i++ )
				offset_for_ref_frame[i] = Se(buf,nLen,StartBit);
			delete [] offset_for_ref_frame;
		}

		num_ref_frames = Ue(buf,nLen,StartBit);
		gaps_in_frame_num_value_allowed_flag = u(1,buf,StartBit);
		pic_width_in_mbs_minus1 = Ue(buf,nLen,StartBit);
		pic_height_in_map_units_minus1 = Ue(buf,nLen,StartBit);

		Width = (pic_width_in_mbs_minus1+1) * 16;
		Height = (pic_height_in_map_units_minus1 + 1) * 16;

		int frame_mbs_only_flag, mb_adaptiv_frame_field_flag, direct_8x8_inference_flag;
		int frame_cropping_flag, frame_crop_left_offset, frame_crop_right_offset;
		int frame_crop_top_offset, frame_crop_bottom_offset;

		frame_mbs_only_flag = Ue(buf,nLen,StartBit);
		if (frame_mbs_only_flag)
		{
			mb_adaptiv_frame_field_flag = u(1,buf,StartBit);
		}
		direct_8x8_inference_flag = u(1,buf,StartBit);

		frame_cropping_flag = u(1,buf,StartBit);
		if (frame_cropping_flag)
		{
			frame_crop_left_offset = Ue(buf,nLen,StartBit);
			frame_crop_right_offset = Ue(buf,nLen,StartBit);
			frame_crop_top_offset = Ue(buf,nLen,StartBit);
			frame_crop_bottom_offset = Ue(buf,nLen,StartBit);
			Width -= (frame_crop_left_offset + frame_crop_right_offset) << 0x1;
			Height -= (frame_crop_top_offset + frame_crop_bottom_offset) << 0x1;
		}

		return 0;
	}
	else
		return -1;
}

int SpsParser::parseSPSFrame(char *frameBits, int *videoWidth, int *videoHeight)
{
	char *bytes = frameBits + 4;
	int Width,Height;
	if (h264_decode_seq_parameter_set(bytes,11, Width, Height) >= 0)
	{		
		*videoWidth = Width;
		*videoHeight = Height;
		return 0;
	}
	else
		return -1;
}

