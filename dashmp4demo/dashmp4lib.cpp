#include "dashmp4lib.h"


BOOL32 generateSampleTable_(vector <H264Frame>& vecFrame, TSampleTableVec& samples, u32 baseDataOffset = 0);


Box mdat(u8* data, u32 dataSize);
Box trun(Track track, u32 offset);
Box sdtp(Track track);
Box traf(Track track, u32 baseMediaDecodeTime);
Box mfhd(u32 sequenceNumber);
Box moof(Track track, u32 baseMediaDecodeTime);
Box trex(Track meta);
Box mvex(Track meta);
Box avc1(Track meta);
Box esds(Track meta);
Box mp4a(Track meta);
Box stsd(Track meta);
Box stbl(Track meta);
Box dinf();
Box minf(Track meta);
Box hdlr(Track meta);
Box mdhd(Track meta);
Box mdia(Track meta);
Box tkhd(Track meta);
Box trak(Track meta);
Box mvhd(u32 timescale, u32 duration);
Box moov(Track meta);
Box btrt();
Box avcc(Track meta);

void mydump(char* str)
{
}

class ExpGolomb
{
public:
	ExpGolomb(u8* dwBuf, u32 dwLen)
	{
		workingBytesAvailable = dwLen;
		workingData = dwBuf;
		workingDataLen = dwLen;
		workingWord = 0;
		workingBitsAvailable = 0;
		
		loadWord();
	}
	u32 length()
	{
		return (8 * workingBytesAvailable);
	}

	u32 bitsAvailable()
	{
		return (8 * workingBytesAvailable) + workingBitsAvailable;
	}  

  void loadWord()
  {
      u32 position = workingDataLen - workingBytesAvailable;
      u8* workingBytes = new u8[4];
	  memset(workingBytes, 0, 4);
	  
      u32 availableBytes = min(4, workingBytesAvailable);

    if (availableBytes == 0) 
	{
       mydump("no bytes available");
	   //assert(0);
    }
	memcpy(workingBytes, &workingData[position], availableBytes);
    /*workingBytes.set(workingData.subarray(position,
                                          position + availableBytes));*/
	//´ó¶Ë±àÂë
	workingWord = 0;
	workingWord = workingBytes[0]<<24;
	workingWord +=  workingBytes[1]<<16;
	workingWord +=  workingBytes[2]<<8;
	workingWord +=  workingBytes[3];
	
    // big endian  workingWord = new DataView(workingBytes.buffer).getUint32(0);

    // track the amount of workingData that has been processed
    workingBitsAvailable = availableBytes * 8;
    workingBytesAvailable -= availableBytes;
  }

  // (count:int):void
  void skipBits(u32 count) 
  {
    u32 skipBytes; // :int
    if (workingBitsAvailable > count) 
	{
      workingWord <<= count;
      workingBitsAvailable -= count;
    } 
	else 
	{		
      count -= workingBitsAvailable;
      skipBytes = floor(count / 8.0f);

      count -= (skipBytes * 8);
      workingBytesAvailable -= skipBytes;

      loadWord();

      workingWord <<= count;
      workingBitsAvailable -= count;
    }
  }

  // (size:int):uint
  u32 readBits(u32 size) 
  {
	  if (size > 31)
	  {
		  mydump("0");
	  }
    u32 bits = min(workingBitsAvailable, size); // :uint
     u32 valu = workingWord >> (32 - bits); // :uint
    // if size > 31, handle error

    workingBitsAvailable -= bits;
    if (workingBitsAvailable > 0)
	{
      workingWord <<= bits;
    } else if (workingBytesAvailable > 0) 
	{
      loadWord();
    }

    bits = size - bits;
    if (bits > 0) 
	{
      return valu << bits | readBits(bits);
    }
    return valu;
  };

  // ():uint
  u32 skipLeadingZeros() 
  {
    u32 leadingZeroCount = 0; // :uint
    for (leadingZeroCount = 0; leadingZeroCount < workingBitsAvailable; ++leadingZeroCount) 
	{
		u32 dwFlags = 0x80000000;
      if ((workingWord & (dwFlags >> leadingZeroCount)) != 0) 
	  {
        // the first bit of working word is 1
        workingWord <<= leadingZeroCount;
        workingBitsAvailable -= leadingZeroCount;
        return leadingZeroCount;
      }
    }

    // we exhausted workingWord and still have not found a 1
    loadWord();
    return leadingZeroCount + skipLeadingZeros();
  }

  // ():void
  void skipUnsignedExpGolomb()
  {
    this->skipBits(1 + skipLeadingZeros());
  }

  // ():void
  void skipExpGolomb()
  {
    skipBits(1 + skipLeadingZeros());
  }

  // ():uint
  u32 readUnsignedExpGolomb()
  {
    u32 clz = skipLeadingZeros(); // :uint
    return readBits(clz + 1) - 1;
  };

  // ():int
  int readExpGolomb()
  {
    u32 valu = readUnsignedExpGolomb(); // :int
    if (0x01 & valu) 
	{
      // the number is odd if the low order bit is set
      return (u32(1 + valu)) >> 1; // add 1 to make it even, and divide by 2
    }
    return -1 * (valu >> 1); // divide by two then make it negative
  }

  // Some convenience functions
  // :Boolean
  BOOL32 readBoolean()
  {
    return readBits(1) == 1;
  }

  // ():int
  u32 readUnsignedByte() 
  {
    return readBits(8);
  }

  void skipScalingList(u32 count)
  {
	  u32 lastScale = 8;
	  u32 nextScale = 8;
	  u32 j;
	  u32 deltaScale;
	  
	  for (j = 0; j < count; j++) 
	  {
		  if (nextScale != 0) 
		  {
			  deltaScale = this->readExpGolomb();
			  nextScale = (lastScale + deltaScale + 256) % 256;
		  }
		  
		  lastScale = (nextScale == 0) ? lastScale : nextScale;
	  }
  }
private:
	u32 workingBytesAvailable;
	u32 workingWord;
	u32 workingBitsAvailable;
	u8* workingData;
	u32 workingDataLen;
};





/*100: true,
110: true,
122: true,
244: true,
44: true,
83: true,
86: true,
118: true,
128: true,
138: true,
139: true,
134: true*/
u8 szEscapeProfile[] = {100, 110, 122, 244, 44, 83, 86, 118, 128, 138, 139, 134};
BOOL32 IsEscapeProfile(u8 byProfile)
{
	for(int inx = 0; inx < sizeof(szEscapeProfile); inx++)
	{
		if (szEscapeProfile[inx] == byProfile)
		{
			return TRUE;
		}
	}
	return FALSE;
}
struct RATIO
{
	void set(double dbfirst, double dbSecond)
	{
		first = dbfirst;
		second = dbSecond;
	}
	double first;
	double second;
};

AVCInfo readSequenceParameterSet(u8* data, u32 dwLendata) 
{
    
	u32 frameCropLeftOffset = 0;
	u32 frameCropRightOffset = 0;
	u32 frameCropTopOffset = 0;
	u32 frameCropBottomOffset = 0;
	double sarScale = 1.0f;
	u32 profileIdc;
	u32 levelIdc;
	u32 profileCompatibility;
	u32 chromaFormatIdc;
	u32 picOrderCntType;
	u32 numRefFramesInPicOrderCntCycle;
	u32 picWidthInMbsMinus1;
	u32 picHeightInMapUnitsMinus1;
	u32 frameMbsOnlyFlag;
	u32 scalingListCount;
	//u32 sarRatio;
	u32 aspectRatioIdc;
	u32 i;
	
    ExpGolomb expGolombDecoder = ExpGolomb(data, dwLendata);
    profileIdc = expGolombDecoder.readUnsignedByte(); // profile_idc
    profileCompatibility = expGolombDecoder.readUnsignedByte(); // constraint_set[0-5]_flag
    levelIdc = expGolombDecoder.readUnsignedByte(); // level_idc u(8)
    expGolombDecoder.skipUnsignedExpGolomb(); // seq_parameter_set_id
	
	
    // some profiles have more optional data we don't need
    if (IsEscapeProfile(profileIdc)) 
	{
		chromaFormatIdc = expGolombDecoder.readUnsignedExpGolomb();
		if (chromaFormatIdc == 3) 
		{
			expGolombDecoder.skipBits(1); // separate_colour_plane_flag
		}
		expGolombDecoder.skipUnsignedExpGolomb(); // bit_depth_luma_minus8
		expGolombDecoder.skipUnsignedExpGolomb(); // bit_depth_chroma_minus8
		expGolombDecoder.skipBits(1); // qpprime_y_zero_transform_bypass_flag
		if (expGolombDecoder.readBoolean()) 
		{ 
			// seq_scaling_matrix_present_flag
			scalingListCount = (chromaFormatIdc != 3) ? 8 : 12;
			for (i = 0; i < scalingListCount; i++) 
			{
				if (expGolombDecoder.readBoolean()) 
				{ 
					// seq_scaling_list_present_flag[ i ]
					if (i < 6) 
					{
						expGolombDecoder.skipScalingList(16);
					} 
					else 
					{
						expGolombDecoder.skipScalingList(64);
					}
				}
			}
		}
    }
	
    expGolombDecoder.skipUnsignedExpGolomb(); // log2_max_frame_num_minus4
    picOrderCntType = expGolombDecoder.readUnsignedExpGolomb();
	
    if (picOrderCntType == 0) 
	{
		expGolombDecoder.readUnsignedExpGolomb(); // log2_max_pic_order_cnt_lsb_minus4
    } 
	else if (picOrderCntType == 1) 
	{
		expGolombDecoder.skipBits(1); // delta_pic_order_always_zero_flag
		expGolombDecoder.skipExpGolomb(); // offset_for_non_ref_pic
		expGolombDecoder.skipExpGolomb(); // offset_for_top_to_bottom_field
		numRefFramesInPicOrderCntCycle = expGolombDecoder.readUnsignedExpGolomb();
		for (i = 0; i < numRefFramesInPicOrderCntCycle; i++) 
		{
			expGolombDecoder.skipExpGolomb(); // offset_for_ref_frame[ i ]
		}
    }
	
    expGolombDecoder.skipUnsignedExpGolomb(); // max_num_ref_frames
    expGolombDecoder.skipBits(1); // gaps_in_frame_num_value_allowed_flag
	
    picWidthInMbsMinus1 = expGolombDecoder.readUnsignedExpGolomb();
    picHeightInMapUnitsMinus1 = expGolombDecoder.readUnsignedExpGolomb();
	
    frameMbsOnlyFlag = expGolombDecoder.readBits(1);
    if (frameMbsOnlyFlag == 0) 
	{
		expGolombDecoder.skipBits(1); // mb_adaptive_frame_field_flag
    }
	
    expGolombDecoder.skipBits(1); // direct_8x8_inference_flag
    if (expGolombDecoder.readBoolean()) { // frame_cropping_flag
		frameCropLeftOffset = expGolombDecoder.readUnsignedExpGolomb();
		frameCropRightOffset = expGolombDecoder.readUnsignedExpGolomb();
		frameCropTopOffset = expGolombDecoder.readUnsignedExpGolomb();
		frameCropBottomOffset = expGolombDecoder.readUnsignedExpGolomb();
    }
    if (expGolombDecoder.readBoolean()) 
	{
		// vui_parameters_present_flag
		if (expGolombDecoder.readBoolean()) 
		{
			// aspect_ratio_info_present_flag
			aspectRatioIdc = expGolombDecoder.readUnsignedByte();
			RATIO sarRatio;
			BOOL32 bFind = TRUE;
			switch (aspectRatioIdc) 
			{
			case 1: sarRatio.set(1, 1); break;
			case 2: sarRatio.set(12, 11); break;
			case 3: sarRatio.set(10, 11); break;
			case 4: sarRatio.set(16, 11); break;
			case 5: sarRatio.set(40, 33); break;
			case 6: sarRatio.set(24, 11); break;
			case 7: sarRatio.set(20, 11); break;
			case 8: sarRatio.set(32, 11); break;
			case 9: sarRatio.set(80, 33); break;
			case 10: sarRatio.set(18, 11); break;
			case 11: sarRatio.set(15, 11); break;
			case 12: sarRatio.set(64, 33); break;
			case 13: sarRatio.set(160, 99); break;
			case 14: sarRatio.set(4, 3); break;
			case 15: sarRatio.set(3, 2); break;
			case 16: sarRatio.set(2, 1); break;
			case 255: 
				{
					sarRatio.set(expGolombDecoder.readUnsignedByte() << 8 |
                        expGolombDecoder.readUnsignedByte(),
                        expGolombDecoder.readUnsignedByte() << 8 |
                        expGolombDecoder.readUnsignedByte() );
					break;
				}
			default :
				bFind = FALSE; 
				break;
			}
			if (bFind) 
			{
				sarScale = sarRatio.first / sarRatio.second;
			}
		}
    }
	AVCInfo tAvcInfo;
	tAvcInfo.profileIdc = profileIdc;
	tAvcInfo.levelIdc = levelIdc;
	tAvcInfo.profileCompatibility = profileCompatibility;
	tAvcInfo.width = ceil((((picWidthInMbsMinus1 + 1) * 16) - frameCropLeftOffset * 2 - frameCropRightOffset * 2) * sarScale);
	tAvcInfo.height = ((2 - frameMbsOnlyFlag) * (picHeightInMapUnitsMinus1 + 1) * 16) - (frameCropTopOffset * 2) - (frameCropBottomOffset * 2);
    return tAvcInfo;
};


BOOL32 generateSampleTable_(vector <H264Frame>& vecFrame, TSampleTableVec& samples, u32 baseDataOffset) 
{	 
	u32 dataOffset = baseDataOffset;
	
	
	for (u32 i = 0; i < vecFrame.size(); i++) 
	{
		H264Frame& currentFrame = vecFrame[i];
		
		//sample = createDefaultSample();
		SampleTable sample;
		
		//sample.dataOffset = dataOffset; 
		sample.compositionTimeOffset = 0;
		sample.duration = currentFrame.duration;
		/*sample.size = 4 * currentFrame.vecNals.size(); // Space for nal unit size
		sample.size += currentFrame.byteLength;*/
		sample.size = currentFrame.GetTotalFrameSize();
		
		if (currentFrame.IsKeyFrame())
		{
			sample.flags.dependsOn = 2;
		}
		
		dataOffset += sample.size;
		
		samples.push_back(sample);
	}
	
	return samples.size() > 0;
}

//////////////////////////////////////////////////////////////////////////
#include "stdio.h"

#define Box_types(type)  const u8 types_##type [] = #type;



Box_types(vmhd);
Box_types(smhd);

Box_types(trak);
Box_types(trun);
Box_types(trex);
Box_types(tkhd);

Box_types(stts);
Box_types(tfdt);
Box_types(tfhd);
Box_types(traf);

Box_types(stco);
Box_types(stsc);
Box_types(stsd);
Box_types(stsz);

Box_types(stbl);
Box_types(sdtp);
Box_types(mvhd);
Box_types(mvex);

Box_types(mp4a);
Box_types(moov);
Box_types(moof);
Box_types(minf);

Box_types(mfhd);
Box_types(mdia);
Box_types(mdhd);
Box_types(mdat);

Box_types(hdlr);
Box_types(ftyp);
Box_types(esds);
Box_types(dref);

Box_types(dinf);
Box_types(btrt);
Box_types(avcC);
Box_types(avc1);

u8 szFTYP[] = 
{
		0x69, 0x73, 0x6F, 0x6D,  // major_brand: isom
		0x0,  0x0,  0x0,  0x1,   // minor_version: 0x01
		0x69, 0x73, 0x6F, 0x6D,  // isom
		0x61, 0x76, 0x63, 0x31   // avc1
};


u8 szSTSD_PREFIX[] = 
{
		0x00, 0x00, 0x00, 0x00,  // version(0) + flags
		0x00, 0x00, 0x00, 0x01   // entry_count
};


u8 szSTTS[] = 
{
		0x00, 0x00, 0x00, 0x00,  // version(0) + flags
		0x00, 0x00, 0x00, 0x00   // entry_count
};
u8 szSTSC[] = 
{
		0x00, 0x00, 0x00, 0x00,  // version(0) + flags
		0x00, 0x00, 0x00, 0x00   // entry_count
};
u8 szSTCO[] = 
{
		0x00, 0x00, 0x00, 0x00,  // version(0) + flags
		0x00, 0x00, 0x00, 0x00   // entry_count
};


u8 szSTSZ[] = 
{
		0x00, 0x00, 0x00, 0x00,  // version(0) + flags
		0x00, 0x00, 0x00, 0x00,  // sample_size
		0x00, 0x00, 0x00, 0x00   // sample_count
};


u8 szHDLR_VIDEO[] = 
{
	0x00, 0x00, 0x00, 0x00,  // version(0) + flags
		0x00, 0x00, 0x00, 0x00,  // pre_defined
		0x76, 0x69, 0x64, 0x65,  // handler_type: 'vide'
		0x00, 0x00, 0x00, 0x00,  // reserved: 3 * 4 bytes
		0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00,
		0x56, 0x69, 0x64, 0x65,
		0x6F, 0x48, 0x61, 0x6E,
		0x64, 0x6C, 0x65, 0x72, 0x00  // name: VideoHandler
};


u8 szHDLR_AUDIO[] = 
{
	0x00, 0x00, 0x00, 0x00,  // version(0) + flags
		0x00, 0x00, 0x00, 0x00,  // pre_defined
		0x73, 0x6F, 0x75, 0x6E,  // handler_type: 'soun'
		0x00, 0x00, 0x00, 0x00,  // reserved: 3 * 4 bytes
		0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00,
		0x53, 0x6F, 0x75, 0x6E,
		0x64, 0x48, 0x61, 0x6E,
		0x64, 0x6C, 0x65, 0x72, 0x00  // name: SoundHandler
};


u8 szDREF[] = 
{
	0x00, 0x00, 0x00, 0x00,  // version(0) + flags
		0x00, 0x00, 0x00, 0x01,  // entry_count
		0x00, 0x00, 0x00, 0x0C,  // entry_size
		0x75, 0x72, 0x6C, 0x20,  // type 'url '
		0x00, 0x00, 0x00, 0x01   // version(0) + flags
};


// Sound media header
u8 szSMHD[] = 
{
	0x00, 0x00, 0x00, 0x00,  // version(0) + flags
		0x00, 0x00, 0x00, 0x00   // balance(2) + reserved(2)
};



// video media header
u8 szVMHD[] = 
{
	0x00, 0x00, 0x00, 0x01,  // version(0) + flags
		0x00, 0x00,              // graphicsmode: 2 bytes
		0x00, 0x00, 0x00, 0x00,  // opcolor: 3 * 2 bytes
		0x00, 0x00
};
	Box ftyp()
	{
		return Box(types_ftyp, szFTYP, sizeof(szFTYP), TRUE);
	}

	    // Movie metadata box
    Box moov(Track meta)
	{
		
        Box boxmvhd = mvhd(meta.timescale, 0xffffffff);
        Box boxtrak = trak(meta);
        Box boxmvex = mvex(meta);
        return Box(types_moov, &boxmvhd, &boxtrak, &boxmvex, Box_end);
    }

    // Movie header box
    Box mvhd(u32 timescale, u32 duration) 
	{
		u8 szmvhd[] = {
				0x00, 0x00, 0x00, 0x00,  // version(0) + flags
				0x00, 0x00, 0x00, 0x01,  // creation_time
				0x00, 0x00, 0x00, 0x02,  // modification_time
				(timescale >> 24) & 0xFF,  // timescale: 4 bytes
				(timescale >> 16) & 0xFF,
				(timescale >>  8) & 0xFF,
				(timescale) & 0xFF,
				(duration >> 24) & 0xFF,   // duration: 4 bytes
				(duration >> 16) & 0xFF,
				(duration >>  8) & 0xFF,
				(duration) & 0xFF,
				0x00, 0x01, 0x00, 0x00,  // Preferred rate: 1.0
				0x01, 0x00, 0x00, 0x00,  // PreferredVolume(1.0, 2bytes) + reserved(2bytes)
				0x00, 0x00, 0x00, 0x00,  // reserved: 4 + 4 bytes
				0x00, 0x00, 0x00, 0x00,
				0x00, 0x01, 0x00, 0x00,  // ----begin composition matrix----
				0x00, 0x00, 0x00, 0x00,
				0x00, 0x00, 0x00, 0x00,
				0x00, 0x00, 0x00, 0x00,
				0x00, 0x01, 0x00, 0x00,
				0x00, 0x00, 0x00, 0x00,
				0x00, 0x00, 0x00, 0x00,
				0x00, 0x00, 0x00, 0x00,
				0x40, 0x00, 0x00, 0x00,  // ----end composition matrix----
				0x00, 0x00, 0x00, 0x00,  // ----begin pre_defined 6 * 4 bytes----
				0x00, 0x00, 0x00, 0x00,
				0x00, 0x00, 0x00, 0x00,
				0x00, 0x00, 0x00, 0x00,
				0x00, 0x00, 0x00, 0x00,
				0x00, 0x00, 0x00, 0x00,  // ----end pre_defined 6 * 4 bytes----
				0xFF, 0xFF, 0xFF, 0xFF   // next_track_ID	
		};

        return Box(types_mvhd, szmvhd, sizeof(szmvhd), TRUE);
    }

    // Track box
    Box trak(Track meta) 
	{
        return Box(types_trak, &tkhd(meta), &mdia(meta), Box_end);
    }

    // Track header box
    Box tkhd(Track meta) 
	{
        u32 trackId = meta.id;
		u32 duration = meta.duration;
        u32 width = meta.avcinfo.width;
		u32 height = meta.avcinfo.height;
		u8 sztkhd[] = {
				0x00, 0x00, 0x00, 0x07,  // version(0) + flags
				0x00, 0x00, 0x00, 0x00,  // creation_time
				0x00, 0x00, 0x00, 0x00,  // modification_time
				(trackId >> 24) & 0xFF,  // track_ID: 4 bytes
				(trackId >> 16) & 0xFF,
				(trackId >>  8) & 0xFF,
				(trackId) & 0xFF,
				0x00, 0x00, 0x00, 0x00,  // reserved: 4 bytes
				(duration >> 24) & 0xFF, // duration: 4 bytes
				(duration >> 16) & 0xFF,
				(duration >>  8) & 0xFF,
				(duration) & 0xFF,
				0x00, 0x00, 0x00, 0x00,
				0x00, 0x00, 0x00, 0x00, // reserved
				0x00, 0x00, // layer
				0x00, 0x00, // alternate_group
				0x01, 0x00, // non-audio track volume
				0x00, 0x00, // reserved
				0x00, 0x01, 0x00, 0x00,
				0x00, 0x00, 0x00, 0x00,
				0x00, 0x00, 0x00, 0x00,
				0x00, 0x00, 0x00, 0x00,
				0x00, 0x01, 0x00, 0x00,
				0x00, 0x00, 0x00, 0x00,
				0x00, 0x00, 0x00, 0x00,
				0x00, 0x00, 0x00, 0x00,
				0x40, 0x00, 0x00, 0x00, // transformation: unity matrix
				(width >> 8) & 0xFF,    // width and height
				(width) & 0xFF,
				0x00, 0x00,
				(height >> 8) & 0xFF,
				(height) & 0xFF,
				0x00, 0x00
		};
        return Box(types_tkhd, sztkhd, sizeof(sztkhd), TRUE);
    }

    // Media Box
    Box mdia(Track meta) 
	{
        return Box(types_mdia, &mdhd(meta), &hdlr(meta), &minf(meta), Box_end);
    }

    // Media header box
    Box mdhd(Track meta) 
	{
        u32 timescale = meta.timescale;
        u32 duration = meta.duration;
		u8 szmdhd[] = {
				0x00, 0x00, 0x00, 0x00,  // version(0) + flags
				0x00, 0x00, 0x00, 0x00,  // creation_time
				0x00, 0x00, 0x00, 0x00,  // modification_time
				(timescale >> 24) & 0xFF,  // timescale: 4 bytes
				(timescale >> 16) & 0xFF,
				(timescale >>  8) & 0xFF,
				(timescale) & 0xFF,
				(duration >> 24) & 0xFF,   // duration: 4 bytes
				(duration >> 16) & 0xFF,
				(duration >>  8) & 0xFF,
				(duration) & 0xFF,
				0x55, 0xC4,             // language: und (undetermined)
				0x00, 0x00              // pre_defined = 0
		};
        return Box(types_mdhd, szmdhd, sizeof(szmdhd), TRUE);
    }

    // Media handler reference box
    Box hdlr(Track meta)
	{
        u8* data = NULL;
		u32 dataSize = 0;
        if (meta.type == audio) 
		{
            data = szHDLR_AUDIO;
			dataSize = sizeof(szHDLR_AUDIO);
        }
		else 
		{
            data = szHDLR_VIDEO;
			dataSize = sizeof(szHDLR_VIDEO);
        }
        return Box(types_hdlr, data, dataSize, TRUE);
    }

    // Media infomation box
    Box minf(Track meta) 
	{
        Box boxxmhd;
        if (meta.type == audio) 
		{
            boxxmhd = Box(types_smhd, szSMHD, sizeof(szSMHD), TRUE);
        } 
		else 
		{
            boxxmhd = Box(types_vmhd, szVMHD, sizeof(szVMHD), TRUE);
        }
        return Box(types_minf, &boxxmhd, &dinf(), &stbl(meta), Box_end);
    }

    // Data infomation box
    Box dinf() 
	{
        Box result = Box(types_dinf, &Box(types_dref, szDREF, sizeof(szDREF), TRUE), Box_end);
        return result;
    }

    // Sample table box
    Box stbl(Track meta) 
	{
        Box result = Box(types_stbl,  // type: stbl
            &stsd(meta),  // Sample Description Table
            &Box(types_stts, szSTTS, sizeof(szSTTS), TRUE),  // Time-To-Sample
            &Box(types_stsc, szSTSC, sizeof(szSTSC), TRUE),  // Sample-To-Chunk
            &Box(types_stsz, szSTSZ, sizeof(szSTSZ), TRUE),  // Sample size
            &Box(types_stco, szSTCO, sizeof(szSTCO), TRUE),  // // Chunk offset
			Box_end); 
        return result; 
    }

    // Sample description box
    Box stsd(Track meta) 
	{
        if (meta.type == audio)
		{
            return Box(types_stsd, szSTSD_PREFIX, sizeof(szSTSD_PREFIX), TRUE, &mp4a(meta));
        } 
		else
		{
            return Box(types_stsd, szSTSD_PREFIX, sizeof(szSTSD_PREFIX), TRUE, &avc1(meta));
        }
    }

    Box mp4a(Track meta) 
	{
        u8 channelCount = meta.channelCount;
        u16 sampleRate = meta.audioSampleRate;
		u8 data[] = {        
            0x00, 0x00, 0x00, 0x00,  // reserved(4)
            0x00, 0x00, 0x00, 0x01,  // reserved(2) + data_reference_index(2)
            0x00, 0x00, 0x00, 0x00,  // reserved: 2 * 4 bytes
            0x00, 0x00, 0x00, 0x00,
            0x00, channelCount,      // channelCount(2)
            0x00, 0x10,              // sampleSize(2)
            0x00, 0x00, 0x00, 0x00,  // reserved(4)
            (sampleRate >> 8) & 0xFF,  // Audio sample rate
            (sampleRate) & 0xFF,
            0x00, 0x00
        };

        return Box(types_mp4a, data, sizeof(data), TRUE, &esds(meta));
    }

    Box esds(Track meta)
	{
        u8* config = meta.config;
        u8 configSize = meta.configLength;
		u8 szPrefix[] = {
			0x00, 0x00, 0x00, 0x00,  // version 0 + flags
			
            0x03,                    // descriptor_type
            0x17 + configSize,       // length3
            0x00, 0x01,              // es_id
            0x00,                    // stream_priority
			
            0x04,                    // descriptor_type
            0x0F + configSize,       // length
            0x40,                    // codec: mpeg4_audio
            0x15,                    // stream_type: Audio
            0x00, 0x00, 0x00,        // buffer_size
            0x00, 0x00, 0x00, 0x00,  // maxBitrate
            0x00, 0x00, 0x00, 0x00,  // avgBitrate
			
            0x05                     // descriptor_type
		};
		u32 dataSize = sizeof(szPrefix) + 1 + configSize + 3;
		u8* data = new u8[dataSize];
		
		memcpy(data, szPrefix, sizeof(szPrefix));
		u32 offset = sizeof(szPrefix);
		data[offset++] = configSize;		
		memcpy(data+offset, config, configSize);
		offset += configSize;

		//0x06, 0x01, 0x02         // GASpecificConfig
		data[offset++] = 0x06;
        data[offset++] = 0x01;
		data[offset++] = 0x02;
        return Box(types_esds, data, dataSize);
    }

    Box avc1(Track meta) 
	{        
        u32 width = meta.avcinfo.width;
		u32 height = meta.avcinfo.height;

        u8 szavc1[] = {
            0x00, 0x00, 0x00,
				0x00, 0x00, 0x00, // reserved
				0x00, 0x01, // data_reference_index
				0x00, 0x00, // pre_defined
				0x00, 0x00, // reserved
				0x00, 0x00, 0x00, 0x00,
				0x00, 0x00, 0x00, 0x00,
				0x00, 0x00, 0x00, 0x00, // pre_defined
				(width & 0xff00) >> 8,
				width & 0xff, // width
				(height & 0xff00) >> 8,
				height & 0xff, // height
				0x00, 0x48, 0x00, 0x00, // horizresolution
				0x00, 0x48, 0x00, 0x00, // vertresolution
				0x00, 0x00, 0x00, 0x00, // reserved
				0x00, 0x01, // frame_count
				0x13,
				0x76, 0x69, 0x64, 0x65,
				0x6f, 0x6a, 0x73, 0x2d,
				0x63, 0x6f, 0x6e, 0x74,
				0x72, 0x69, 0x62, 0x2d,
				0x68, 0x6c, 0x73, 0x00,
				0x00, 0x00, 0x00, 0x00,
				0x00, 0x00, 0x00, 0x00,
				0x00, 0x00, 0x00, // compressorname
				0x00, 0x18, // depth = 24
				0x11, 0x11 // pre_defined = -1
        };
        return Box(types_avc1, szavc1, sizeof(szavc1), TRUE, 
			                   &avcc(meta)); // now just no btrt  by wxh
    }
	Box avcc(Track track)
	{
		
		u32 dataSize = 5 + 1 + 2 + track.spsNal.dwNalLen + 1 + 2 + track.ppsNal.dwNalLen;
		u8* data = new u8[dataSize];
		u32 offset = 0;
		data[offset++] = 0x01;// configurationVersion
		data[offset++] = track.avcinfo.profileIdc, // AVCProfileIndication
		data[offset++] = track.avcinfo.profileCompatibility, // profile_compatibility
		data[offset++] = track.avcinfo.levelIdc, // AVCLevelIndication
		data[offset++] = 0xff; // lengthSizeMinusOne, hard-coded to 4 bytes

		data[offset++] = 0x01;  // numOfSequenceParameterSets
		data[offset++] = (track.spsNal.dwNalLen >> 8) & 0xff;
		data[offset++] = (track.spsNal.dwNalLen) & 0xff;
		memcpy(&data[offset], track.spsNal.pBufStart, track.spsNal.dwNalLen);
		offset += track.spsNal.dwNalLen;

		data[offset++] = 0x01;  // numOfSequenceParameterSets
		data[offset++] = (track.ppsNal.dwNalLen >> 8) & 0xff;
		data[offset++] = (track.ppsNal.dwNalLen) & 0xff;
		memcpy(&data[offset], track.ppsNal.pBufStart, track.ppsNal.dwNalLen);
		
		
		return Box(types_avcC, data, dataSize);
	}
	Box btrt()
	{
		u8 szbtrt[] = {
			0x00, 0x1c, 0x9c, 0x80, // bufferSizeDB
			0x00, 0x2d, 0xc6, 0xc0, // maxBitrate
			0x00, 0x2d, 0xc6, 0xc0
		};
		return Box(types_btrt, szbtrt, sizeof(szbtrt), TRUE);
	}
    // Movie Extends box
    Box mvex(Track meta) 
	{
        return Box(types_mvex, &trex(meta), Box_end);
    }

    // Track Extends box
    Box trex(Track meta) 
	{
        u32 trackId = meta.id;
        u8 sztrex[] = {
            0x00, 0x00, 0x00, 0x00,  // version(0) + flags
            (trackId >> 24) & 0xFF, // track_ID
            (trackId >> 16) & 0xFF,
            (trackId >>  8) & 0xFF,
            (trackId) & 0xFF,
            0x00, 0x00, 0x00, 0x01,  // default_sample_description_index
            0x00, 0x00, 0x00, 0x00,  // default_sample_duration
            0x00, 0x00, 0x00, 0x00,  // default_sample_size
            0x00, 0x01, 0x00, 0x01   // default_sample_flags
        };
        return Box(types_trex, sztrex, sizeof(sztrex), TRUE);
    }

    // Movie fragment box
    Box moof(Track track, u32 baseMediaDecodeTime) 
	{
        return Box(types_moof, &mfhd(track.sequenceNumber), &traf(track, baseMediaDecodeTime), Box_end);
    }

    Box mfhd(u32 sequenceNumber) 
	{
        u8 szmfhd[] = {
            0x00, 0x00, 0x00, 0x00,
            (sequenceNumber >> 24) & 0xFF,  // sequence_number: int32
            (sequenceNumber >> 16) & 0xFF,
            (sequenceNumber >>  8) & 0xFF,
            (sequenceNumber) & 0xFF
        };
        return Box(types_mfhd, szmfhd, sizeof(szmfhd), TRUE);
    }

    // Track fragment box
    Box traf(Track track, u32 baseMediaDecodeTime) 
	{
        u32 trackId = track.id;

        // Track fragment header box
		u8 sztfhd[] = {
				0x00, 0x00, 0x00, 0x00,  // version(0) & flags
				(trackId >> 24) & 0xFF, // track_ID
				(trackId >> 16) & 0xFF,
				(trackId >>  8) & 0xFF,
				(trackId) & 0xFF,
				0x00, 0x00, 0x00, 0x01, // sample_description_index
				0x00, 0x00, 0x00, 0x00, // default_sample_duration
				0x00, 0x00, 0x00, 0x00, // default_sample_size
				0x00, 0x00, 0x00, 0x00  // default_sample_flags
		};
        Box boxtfhd = Box(types_tfhd, sztfhd, sizeof(sztfhd), TRUE);
		
		u8 sztfdt[] = {
				0x00, 0x00, 0x00, 0x00,  // version(0) & flags
				(baseMediaDecodeTime >> 24) & 0xFF,  // baseMediaDecodeTime: int32
				(baseMediaDecodeTime >> 16) & 0xFF,
				(baseMediaDecodeTime >>  8) & 0xFF,
				(baseMediaDecodeTime) & 0xFF
		};
        // Track Fragment Decode Time
        Box boxtfdt = Box(types_tfdt, sztfdt, sizeof(sztfdt), TRUE);

        Box boxsdtp = sdtp(track);
        Box boxtrun = trun(track, /*sdtp.byteLength*/
			               boxsdtp.BoxSize() + 32 + 16 + 8 + 16 + 8 + 8);

        return Box(types_traf, &boxtfhd, &boxtfdt, &boxtrun, &boxsdtp, Box_end);
    }

    // Sample Dependency Type box
    Box sdtp(Track track)
	{
        TSampleTableVec &samples = track.samples;
        u32 sampleCount = samples.size();
		u32 dataSize = 4 + sampleCount;
        u8* data = new u8[dataSize];
		memset(data, 0, dataSize);
        // 0~4 bytes: version(0) & flags

        for (u32 i = 0; i < sampleCount; i++) 
		{
            SampleFlag flags = samples[i].flags;
            data[i + 4] = (flags.isLeading << 6)    // is_leading: 2 (bit)
                        | (flags.dependsOn << 4)    // sample_depends_on
                        | (flags.isDependedOn << 2) // sample_is_depended_on
                        | (flags.hasRedundancy);    // sample_has_redundancy
        }
        return Box(types_sdtp, data, dataSize);
    }

    // Track fragment run box
    Box trun(Track track, u32 offset) 
	{
        TSampleTableVec &samples = track.samples;
        u32 sampleCount = samples.size();
        u32 dataSize = 12 + 16 * sampleCount;
        u8* data = new u8[dataSize];
		memset(data, 0, dataSize);

        offset += 8 + dataSize;

		u8 szFlag[] = {
			0x00, 0x00, 0x0F, 0x01,      // version(0) & flags
            (sampleCount >> 24) & 0xFF, // sample_count
            (sampleCount >> 16) & 0xFF,
            (sampleCount >>  8) & 0xFF,
            (sampleCount) & 0xFF,
            (offset >> 24) & 0xFF,      // data_offset
            (offset >> 16) & 0xFF,
            (offset >>  8) & 0xFF,
            (offset) & 0xFF
        };
		memcpy(data, szFlag, sizeof(szFlag)); // 12 bytes

        for (u32 i = 0; i < sampleCount; i++) 
		{
            u32 duration = samples[i].duration;
            u32 size = samples[i].size;
            SampleFlag flags = samples[i].flags;
            //let cts = samples[i].cts;
			u32 cts = samples[i].compositionTimeOffset;
            u8 szSampleFlag [] = 
			{
                (duration >> 24) & 0xFF,  // sample_duration
                (duration >> 16) & 0xFF,
                (duration >>  8) & 0xFF,
                (duration) & 0xFF,
                (size >> 24) & 0xFF,      // sample_size
                (size >> 16) & 0xFF,
                (size >>  8) & 0xFF,
                (size) & 0xFF,
                (flags.isLeading << 2) | flags.dependsOn,  // sample_flags
                (flags.isDependedOn << 6) | (flags.hasRedundancy << 4) | flags.isNonSyncSample,//old isNonSync,
                //0x00, 0x00,                // sample_degradation_priority
				
				flags.degradationPriority & 0xF0 << 8,
				flags.degradationPriority & 0x0F, // sample_flags
				
                (cts >> 24) & 0xFF,       // sample_composition_time_offset
                (cts >> 16) & 0xFF,
                (cts >>  8) & 0xFF,
                (cts) & 0xFF
			}; //, 12 + 16 * i);
			memcpy(data+12+16*i, szSampleFlag, sizeof(szSampleFlag));
        }
        return Box(types_trun, data, dataSize);
    }

    Box mdat(u8* data, u32 dataSize) 
	{
        return Box(types_mdat, data, dataSize);
    }





Mp4::Mp4(vector <H264Frame>& tVecFrames, u32 dwsequenceNumber) : m_tVecFrames(tVecFrames)
{
	m_dwMp4FileSize = -1;
	m_tTrack.sequenceNumber = dwsequenceNumber;
	this->Init();
}

Mp4::~Mp4()
{
	m_boxFtyp.Destroy();
	m_boxMoov.Destroy();
	m_boxMoof.Destroy();
}
BOOL32 Mp4::Init()
{
	m_tTrack.type = video;
	m_tTrack.id = 101;
	m_tTrack.timescale = 90000;			
	m_tTrack.baseMediaDecodeTime = m_tVecFrames[0].dwTime;
	
	
	m_tVecFrames[0].GetAvcInfo(m_tTrack);	

	u32 dwFrames = m_tVecFrames.size();
	if (dwFrames >= 2)
	{	
		for(int inx = 1; inx < dwFrames; inx++)
		{
			m_tVecFrames[inx-1].duration = m_tVecFrames[inx].dwTime - m_tVecFrames[inx-1].dwTime;
		}
		m_tVecFrames[dwFrames-1].duration =m_tVecFrames[dwFrames-2].duration;

		m_tTrack.duration = m_tVecFrames[dwFrames-1].dwTime - m_tVecFrames[0].dwTime + 
			                m_tVecFrames[dwFrames-1].duration;
	}
	else
	{
		m_tVecFrames[0].duration = 40;
		m_tTrack.duration = 40;
	}
	//m_tTrack.duration = 0xffffffff;
	
	generateSampleTable_(m_tVecFrames, m_tTrack.samples);

	m_boxFtyp = ftyp();
	m_boxMoov = moov(m_tTrack);
	m_boxMoof = moof(m_tTrack, m_tTrack.baseMediaDecodeTime);
	return TRUE;
}
u32 Mp4::GetMp4FileTotalSize()
{
	if (-1 == m_dwMp4FileSize)
	{
		m_dwMp4FileSize = m_boxFtyp.BoxSize() + m_boxMoov.BoxSize() + m_boxMoof.BoxSize();
		m_dwMdatSize = 8;
		for(int inx = 0; inx < m_tVecFrames.size(); inx++)
		{		
			m_dwMdatSize += m_tVecFrames[inx].GetTotalFrameSize();
		}
		m_dwMp4FileSize += m_dwMdatSize;
	}
	return m_dwMp4FileSize;
}
BOOL32 Mp4::WriteMp4ToBuffer(u8* dwFileBuf, u32 dwLen)
{
	if (dwLen < this->GetMp4FileTotalSize())
	{
		return FALSE;
	}
	m_boxFtyp.WriteToBuffer(dwFileBuf);
	dwFileBuf += m_boxFtyp.BoxSize();

	m_boxMoov.WriteToBuffer(dwFileBuf);
	dwFileBuf += m_boxMoov.BoxSize();

	m_boxMoof.WriteToBuffer(dwFileBuf);
	dwFileBuf += m_boxMoof.BoxSize();

	WriteMdatToBuffer(dwFileBuf);
	return TRUE;
}

void Mp4::WriteMdatToBuffer(u8* dwFileBuf)
{
	dwFileBuf[0] = (m_dwMdatSize >> 24 & 0xff);
	dwFileBuf[1] = (m_dwMdatSize >> 16 & 0xff);
	dwFileBuf[2] = (m_dwMdatSize >> 8 & 0xff);
	dwFileBuf[3] = (m_dwMdatSize & 0xff);
	dwFileBuf += 4;

	memcpy(dwFileBuf, types_mdat, 4);
	dwFileBuf += 4;

	for(int inx = 0; inx < m_tVecFrames.size(); inx++)
	{		
		m_tVecFrames[inx].WriteFrameToBuffer(dwFileBuf);
		dwFileBuf += m_tVecFrames[inx].GetTotalFrameSize();
	}
}