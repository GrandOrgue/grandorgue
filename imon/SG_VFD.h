#ifndef __SG_VFD_H__
#define __SG_VFD_H__

#ifdef IMONVFD_EXPORT
 #define IMONVFD_API __declspec(dllexport)
#else
 #define IMONVFD_API __declspec(dllimport)
#endif


#define VFDHW_IMON_VFD		4

#ifdef __cplusplus
extern "C" {
#endif

///////////////////////////////////////////////
//	Open VFD driver and initialize parameters.
//	Call this method, when application starts.
//	Return value informs driver is open or not 
IMONVFD_API bool iMONVFD_Init(int vfdType, int resevered=0);

///////////////////////////////////////////////
//	Close VFD driver.
//	Call this method, when application destroyed.
IMONVFD_API void iMONVFD_Uninit(void);

///////////////////////////////////////////////
//	Check if VFD driver is opened.
IMONVFD_API bool iMONVFD_IsInited(void);

///////////////////////////////////////////////
//	Send text data to VFD. VFD supports only English character set.
IMONVFD_API bool iMONVFD_SetText(char* szFirstLine, char* szSecondLine);

///////////////////////////////////////////////
//	Send EQ data to VFD.
//	Total 16band, each band ranges from 0 to 100 
//  make EQ data with integer array.
IMONVFD_API bool iMONVFD_SetEQ(int* arEQValue);

#ifdef __cplusplus
}	//	extern "C"
#endif

#endif	//__SG_VFD_H__

