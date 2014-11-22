/**
 * @file  RSPI.h
 * @brief RSPIの挙動を司るクラス
 */

#ifndef __RSPI_H__
#define __RSPI_H__
/*----------------------------------------------------------------------
	Includes
 ----------------------------------------------------------------------*/
#include "../typedefine.h"

/*----------------------------------------------------------------------
	Macro definitions
 ----------------------------------------------------------------------*/
#define	RSPI_DUMMY_TXDATA		(0xFFFFFFFF)

#define RSPI_CYCLE_INTERVAL		375	// Cycleモードの周期 375: 250[uSec]
//#define RSPI_CYCLE_INTERVAL		1500	// Cycleモードの周期 1500: 1[mSec]

/*----------------------------------------------------------------------
	Typedef definitions
 ----------------------------------------------------------------------*/
typedef enum
{
    RSPI_BYTE_DATA = 0x01,
    RSPI_WORD_DATA = 0x02,
    RSPI_LONG_DATA = 0x04
} RSPI_DATA_FORMAT;

typedef struct rspi_denpendence
{
	_UBYTE	brdv_val;
	_UBYTE	cpha_val;
	_UBYTE	cpol_val;
	_UBYTE	sslkp_val;
	_UBYTE	spb_val;
	_UBYTE	lsbf_val;
	_UBYTE	spnden_val;
	_UBYTE	slnden_val;
	_UBYTE	sckden_val;

	RSPI_DATA_FORMAT format;
	void	(*ChipSelectFunc)(void);
	void	(*ChipDeselectFunc)(void);
}RSPI_DEPENDENCE;


/*----------------------------------------------------------------------
	Public Methods Declarations
 ----------------------------------------------------------------------*/
void InitializeRSPI0(void);
inline void Int_SPRI0(void);
#pragma inline(Int_SPTI0)
void Int_SPTI0(void);
#pragma inline(Int_SPII0)
void Int_SPII0(void);
#pragma inline(Int_SPEI0)
void Int_SPEI0(void);
#pragma inline(RSPI0_IntCMT1)
void RSPI0_IntCMT1(void);
void RSPI0_Read(void *pdest, _UWORD length, RSPI_DEPENDENCE dep);
void RSPI0_Write(void *psrc, _UWORD length, RSPI_DEPENDENCE dep);
void RSPI0_WriteRead(void *psrc, void *pdest, _UWORD length, RSPI_DEPENDENCE dep);
void RSPI0_RegisterDeviceForCycleOperation(void (*cycleSendFunc)(void));
void RSPI0_StartCycleOperation(void);
void RSPI0_StopCycleOperation(void);

#endif
