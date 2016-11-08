#ifndef _DASH_MP4_LIB_H_
#define _DASH_MP4_LIB_H_

#include <stdio.h>
#include "math.h"
#include "stdarg.h"

#ifndef min
#define min(a,b)    (((a) < (b)) ? (a) : (b))
#endif


#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#define MYDEFINE_WXH_
#ifdef MYDEFINE_WXH_

typedef unsigned char	u8;
typedef unsigned short  u16;
typedef short           s16;
typedef char            s8;
typedef int		s32,BOOL32;
typedef unsigned long    u32;

#ifdef WIN32
typedef unsigned __int64 u64;
#else 
typedef unsigned long long u64;
#endif
#endif

#include <vector>
using namespace std;

struct Buffer 
{
	Buffer()
	{
		pBuffer = NULL;
		dwBufLen = 0;
	}
	u8* pBuffer;
	u32 dwBufLen;
};

struct NalUnitBuf
{
	u8* pBufStart;
	u32 dwNalLen; // no 0 0 0 1 or 0 0 1
	/*u8 nalUnitType;
	u32 pts;
	u32 dts;*/
};

struct  AVCInfo
{
	u32 profileIdc;
	u32 levelIdc;
	u32 profileCompatibility;
	u32 width;
	u32 height;
};

struct SampleFlag 
{
	SampleFlag()
	{
		isLeading = 0;
		dependsOn = 1;
		isDependedOn = 0;
		hasRedundancy = 0;
		degradationPriority = 0;
		paddingValue = 0;
		isNonSyncSample = 0;
	}
	
	u32 isLeading;
	u32 dependsOn;
	u32 isDependedOn;
	u32 hasRedundancy;
	u32 degradationPriority;
	u8 paddingValue;
	u8 isNonSyncSample;
};
struct SampleTable
{
	SampleTable()
	{
		size = 0;
		//dataOffset = 0;
		compositionTimeOffset = 0;
		duration = 0;
	}
	u32 size;
	//u32 dataOffset;
	u32 compositionTimeOffset;
	u32 duration;
	SampleFlag flags;
};
typedef vector < SampleTable > TSampleTableVec;

#define audio 0
#define video 1
struct Track
{
	Track()
	{
		timescale = 90000;
		duration = 0xffffffff;		
	}
	u32 timescale;
	u32 duration;
	u32 id;
	u32 sequenceNumber;	
	u32 type;
	u32 baseMediaDecodeTime;
	
	TSampleTableVec samples;
	
	AVCInfo avcinfo;
	NalUnitBuf spsNal;
	NalUnitBuf ppsNal;

	u8 channelCount;
	u16 audioSampleRate;
	u8* config;
	u32 configLength;
};
AVCInfo readSequenceParameterSet(u8* data, u32 dwLendata);

class H264Frame
{
public:
	H264Frame(u8* pBuffer, u32 dwLen, u32 dwTime)
	{
		bIsKeyFrame = -1;
		this->dwTime = dwTime;
		this->tBuf.pBuffer = pBuffer;
		this->tBuf.dwBufLen = dwLen;

		this->GetNalunit();
	}
	BOOL32 IsKeyFrame()
	{
		if (-1 != bIsKeyFrame) 
		{
			return bIsKeyFrame;
		}
		bIsKeyFrame = FALSE;
		for(int inx = 0; inx < vecNals.size(); inx++)
		{
			NalUnitBuf nal = vecNals[inx];
			if ((nal.pBufStart[0] & 0x1f) == 5) 
			{
				bIsKeyFrame = TRUE;
			}
		}
		return bIsKeyFrame;
	}
	BOOL32 IsVideo()
	{
		return TRUE;
	}
	BOOL32 GetAvcInfo(Track& track)
	{	
		BOOL32 bFind = FALSE;
		for(int inx = 0; inx < vecNals.size(); inx++)
		{
			NalUnitBuf nal = vecNals[inx];
			if ((nal.pBufStart[0] & 0x1f) == 7) 
			{
				track.avcinfo = readSequenceParameterSet(&nal.pBufStart[1], nal.dwNalLen-1);
				
				bFind = TRUE;
				track.spsNal = nal;
			}
			else if ((nal.pBufStart[0] & 0x1f) == 8)
			{
				track.ppsNal = nal;
				bFind = TRUE;
			}			
		}
		
		return bFind;
	}
	u32 GetTotalFrameSize()
	{
		u32 dwTotalSize = 4 * vecNals.size();
		for(int inx = 0; inx < vecNals.size(); inx++)
		{
			dwTotalSize += vecNals[inx].dwNalLen;			
		}
		return dwTotalSize;
	}
	void WriteFrameToBuffer(u8* pBuffer)
	{
		u32 offset = 0;

		for(int inx = 0; inx < vecNals.size(); inx++)
		{
			u32 dwNalSize = vecNals[inx].dwNalLen;

			pBuffer[offset++] = (dwNalSize >> 24 & 0xff);
			pBuffer[offset++] = (dwNalSize >> 16 & 0xff);
			pBuffer[offset++] = (dwNalSize >> 8 & 0xff);
			pBuffer[offset++] = (dwNalSize & 0xff);
			
			memcpy(pBuffer+offset, vecNals[inx].pBufStart, dwNalSize);
			offset += dwNalSize;

		}		
	}
	void GetNalunit()
	{		
		vecNals.clear();

		u8* buffer = tBuf.pBuffer;
		u32 dwLen = tBuf.dwBufLen;				
		
		int i;
		int syncPoint = 0;
		for (; syncPoint < dwLen - 3; syncPoint++) 
		{
			if (buffer[syncPoint + 2] == 1) 
			{
				// the sync point is properly aligned
				i = syncPoint + 5;
				break;
			}
		}
		
		while (i < dwLen) 
		{
			// look at the current byte to determine if we've hit the end of
			// a NAL unit boundary
			switch (buffer[i]) 
			{
			case 0:
				// skip past non-sync sequences
				if (buffer[i - 1] != 0) {
					i += 2;
					break;
				} else if (buffer[i - 2] != 0) {
					i++;
					break;
				}
				
				// deliver the NAL unit if it isn't empty
				if (syncPoint + 3 != i - 2) 
				{
					//this.trigger('data', buffer.subarray(syncPoint + 3, i - 2));
					// one nalu
					NalUnitBuf tNal;
					tNal.pBufStart = &buffer[syncPoint+3];
					tNal.dwNalLen = i-2 - (syncPoint + 3);
					vecNals.push_back(tNal);
				}
				
				// drop trailing zeroes
				do 
				{
					i++;
				} while (buffer[i] != 1 && i < dwLen);
				syncPoint = i - 2;
				i += 3;
				break;
			case 1:
				// skip past non-sync sequences
				if (buffer[i - 1] != 0 ||
					buffer[i - 2] != 0) 
				{
					i += 3;
					break;
				}
				
				// deliver the NAL unit
				//this.trigger('data', buffer.subarray(syncPoint + 3, i - 2));
				
				NalUnitBuf tNal;
				tNal.pBufStart = &buffer[syncPoint+3];
				tNal.dwNalLen = i-2 - (syncPoint + 3);
				vecNals.push_back(tNal);
				
				syncPoint = i - 2;
				i += 3;
				break;
			default:
				// the current byte isn't a one or zero, so it cannot be part
				// of a sync sequence
				i += 3;
				break;
			}
		}
		// filter out the NAL units that were delivered
		//buffer = buffer.subarray(syncPoint);
		if (syncPoint < dwLen)
		{			
			NalUnitBuf tNal;
			tNal.pBufStart = &buffer[syncPoint+3];
			tNal.dwNalLen = dwLen - syncPoint - 3;
			vecNals.push_back(tNal);			
		}
		i -= syncPoint;
		syncPoint = 0;
	}
	
public:
	u32 dwTime;
	u32 duration;
	Buffer tBuf; // one full frame
private:
		
	vector <NalUnitBuf> vecNals;
	BOOL32 bIsKeyFrame;	
}; 



#define Box_end ((Box*)-1)
class Box
{
public:
	// the buffer better is new alloced
	Box()
	{
		
	}
	Box(const u8* szType, u8* pBuffer, u32 dwLen, BOOL32 bRealloc = FALSE)
	{
		m_szType = szType;
		m_dwTotalSize = 4 + 4 + dwLen;

		m_tBuf.pBuffer = pBuffer;
		m_tBuf.dwBufLen = dwLen;

		if (bRealloc)
		{
			if (0 == dwLen) 
			{
				int x = 0;
			}
			u8* pNewBuf = new u8[dwLen];
			memcpy(pNewBuf, pBuffer, dwLen);
			m_tBuf.pBuffer = pNewBuf;			
		}		
	}
	Box(const u8* szType, u8* pBuffer, u32 dwLen, BOOL32 bRealloc, Box* box1)
	{
		*this = Box(szType, pBuffer, dwLen, bRealloc);

		m_vecSon.push_back(*box1);
		m_dwTotalSize += box1->m_dwTotalSize;
	}
	Box(const u8* szType, Box* box1, ...)
	{
		m_szType = szType;
		m_dwTotalSize = 4 + 4;

		va_list ap;
		va_start(ap, box1);
		
		Box* pBox = box1;
		while (pBox && (u32)pBox != -1)
		{
			m_vecSon.push_back(*pBox);
			m_dwTotalSize += pBox->m_dwTotalSize;

			pBox = (Box*)va_arg(ap, void*);
		}
		va_end(ap);
	}
	void WriteToBuffer(u8* pDstBuf)
	{
		u32 dwOffset = 0;
		// write totalsize
		pDstBuf[0] = (m_dwTotalSize >> 24) & 0xFF;  // size
        pDstBuf[1] = (m_dwTotalSize >> 16) & 0xFF;
        pDstBuf[2] = (m_dwTotalSize >> 8) & 0xFF;
        pDstBuf[3] = (m_dwTotalSize) & 0xFF;
		dwOffset += 4;
		
		// write type
		memcpy(pDstBuf+dwOffset, m_szType, 4);
		dwOffset += 4;

		// write buffer
		if (m_tBuf.pBuffer)
		{
			memcpy(pDstBuf+dwOffset, m_tBuf.pBuffer, m_tBuf.dwBufLen);
			dwOffset += m_tBuf.dwBufLen;
		}
		
		for(int inx = 0; inx < m_vecSon.size(); inx++)
		{
			m_vecSon[inx].WriteToBuffer(pDstBuf+dwOffset);
			dwOffset += m_vecSon[inx].m_dwTotalSize;
		}
	}

	void Destroy()
	{
		if (m_tBuf.pBuffer)
		{
			delete [] m_tBuf.pBuffer;
			m_tBuf.pBuffer = NULL;
			m_tBuf.dwBufLen = 0;
		}
		for(int inx = 0; inx < m_vecSon.size(); inx++)
		{
			m_vecSon[inx].Destroy();
		}
	}
	u32 BoxSize()
	{
		return m_dwTotalSize;
	}
private:
	const u8* m_szType;
	u32 m_dwTotalSize;
	Buffer m_tBuf;
	vector < Box > m_vecSon;
};



class Mp4  
{
public:
	Mp4(vector <H264Frame>& tVecFrames, u32 dwsequenceNumber);
	virtual ~Mp4();
	
	
	u32 GetMp4FileTotalSize();
	BOOL32 WriteMp4ToBuffer(u8* dwFileBuf, u32 dwLen);
private:
	void WriteMdatToBuffer(u8* dwFileBuf);
	BOOL32 Init();

	Track m_tTrack;
	Box m_boxFtyp;
	Box m_boxMoov;
	Box m_boxMoof;
	u32 m_dwMdatSize;
	u32 m_dwMp4FileSize;
	vector <H264Frame>& m_tVecFrames;
};


#endif