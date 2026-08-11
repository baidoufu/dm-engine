//-----------------------------------------------------------------------------
// File: DSUtil.h
//
// Desc: 
//
// Copyright (c) 1999-2001 Microsoft Corp. All rights reserved.
//-----------------------------------------------------------------------------

#pragma once

#include "../../Global/Interface/CallBackInterface.h"

#include <basetsd.h>
#include <mmsystem.h>
#include <mmreg.h>
#include <dsound.h>

#pragma comment(lib,"dsound.lib")
#pragma comment(lib,"dxguid.lib")
#pragma comment(lib,"winmm.lib")

#define DEC_STEP -1000
#define NUM_PLAY_NOTIFICATIONS            16//equral dwNotifyCount
#define NUM_SECOND_STREAMING_BUFFER       3

class CSound;
class CWaveFile;
class CSoundManager;

#define WAVEFILE_READ   1
#define WAVEFILE_WRITE  2
//HANDLE              g_hNotificationEvent =NULL;
//DWORD               g_dwNotifyThreadID = 0;
//HANDLE              g_hNotifyThread;

class CWaveFile
{
public:
    WAVEFORMATEX* m_pwfx;        // Pointer to WAVEFORMATEX structure
    HMMIO         m_hmmio;       // MM I/O handle for the WAVE
    MMCKINFO      m_ck;          // Multimedia RIFF chunk
    MMCKINFO      m_ckRiff;      // Use in opening a WAVE file
    DWORD         m_dwSize;      // The size of the wave file
    MMIOINFO      m_mmioinfoOut;
    DWORD         m_dwFlags;
    BOOL          m_bIsReadingFromMemory;
    BYTE*         m_pbData;
    BYTE*         m_pbDataCur;
    ULONG         m_ulDataSize;
    CHAR*         m_pResourceBuffer;

protected:
    HRESULT ReadMMIO();
    HRESULT WriteMMIO( WAVEFORMATEX *pwfxDest );

public:
    CWaveFile();
    ~CWaveFile();

    HRESULT Open( LPTSTR strFileName, WAVEFORMATEX* pwfx, DWORD dwFlags );
    HRESULT OpenFromMemory( BYTE* pbData, ULONG ulDataSize, WAVEFORMATEX* pwfx, DWORD dwFlags );
    HRESULT Close();

    HRESULT Read( BYTE* pBuffer, DWORD dwSizeToRead, DWORD* pdwSizeRead );
    HRESULT Write( UINT nSizeToWrite, BYTE* pbData, UINT* pnSizeWrote );

    DWORD   GetSize();
    HRESULT ResetFile();
    WAVEFORMATEX* GetFormat() { return m_pwfx; };
};

class CSound
{
protected:
    LPDIRECTSOUNDBUFFER  m_apDSBuffer;
    DWORD                m_dwDSBufferSize;
    CWaveFile*           m_pWaveFile;
	DWORD				 m_dwCreationFlags;
	DWORD				 m_dwCurPlayPos;
	DWORD				 m_dwCurWritePos;
	long				 m_nVol;		//
	long				 m_nFinalVol;
	long				 m_nStep;
	long				 m_nIncStep ;

    HRESULT RestoreBuffer( LPDIRECTSOUNDBUFFER pDSB, BOOL* pbWasRestored );

public:
	CSound();
    CSound( LPDIRECTSOUNDBUFFER apDSBuffer, DWORD dwDSBufferSize,  CWaveFile* pWaveFile, DWORD dwCreationFlags );
    void Initialize( LPDIRECTSOUNDBUFFER apDSBuffer, DWORD dwDSBufferSize, CWaveFile* pWaveFile, DWORD dwCreationFlags );
	virtual ~CSound();
	void Free();

    HRESULT Get3DBufferInterface( DWORD dwIndex, LPDIRECTSOUND3DBUFFER* ppDS3DBuffer );
    HRESULT FillBufferWithSound( LPDIRECTSOUNDBUFFER pDSB, BOOL bRepeatWavIfBufferLarger,int i=0);
    LPDIRECTSOUNDBUFFER GetFreeBuffer();
    LPDIRECTSOUNDBUFFER GetBuffer( DWORD dwIndex );

    HRESULT Play( DWORD dwPriority = 0, DWORD dwFlags = 0 ,LONG lVolume=0, LONG lFrequency=-1, LONG lPan=0 );
    virtual HRESULT Stop();
    HRESULT Reset();
	void Pause();
	void Restart(DWORD dwFlags=0, DWORD dwPriority = 0);
    DWORD    IsSoundPlaying();
	UINT GetBufferSize()		{return m_dwDSBufferSize;}
	long DecVol();
	bool IncVol();

	HRESULT SetPan(LONG lPan);
	HRESULT SetVol(LONG lVol);
};

class CSoundManager
{
public:
	CSoundManager(void);
	~CSoundManager(void);
    HRESULT Initialize( HWND hWnd ,DWORD dwCoopLevel,char *path);
	HRESULT SetPrimaryBufferFormat( DWORD dwPrimaryChannels, DWORD dwPrimaryFreq, DWORD dwPrimaryBitRate );

    HRESULT Create( CSound* ppSound, LPTSTR strWaveFileName, DWORD dwCreationFlags = 0, GUID guid3DAlgorithm = GUID_NULL, DWORD dwNumBuffers = 1 );
    HRESULT CreateFromMemory( CSound** pSound, BYTE* pbData, ULONG ulDataSize, LPWAVEFORMATEX pwfx, DWORD dwCreationFlags = 0, GUID guid3DAlgorithm = GUID_NULL, DWORD dwNumBuffers = 1 ); 

	//MIDI
	HRESULT InitMidi();
	void    SetPath(char *path);	//设置工作路径；保持pack和普通路径一致	
	LPDIRECTSOUND8& GetDirectSound8(){return m_pDS;}
protected:
    LPDIRECTSOUND8 m_pDS;
};
