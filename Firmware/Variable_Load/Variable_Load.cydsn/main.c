#include "project.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include "terminal.h"
#include "dataBlock.h"

#define KPN 100 //default 19
#define KPD 100 //default 100
#define KIN 20 //default 1
#define KID 12 //default 10

#define DEFAULT_I_LIM 0
#define DEFAULT_V_MIN 2000

/* Defines for SourceDMA */
#define SourceDMA_BYTES_PER_BURST 2
#define SourceDMA_REQUEST_PER_BURST 1
#define SourceDMA_SRC_BASE (CYDEV_PERIPH_BASE)
#define SourceDMA_DST_BASE (CYDEV_SRAM_BASE)

/* Defines for BufferDMA */
#define BufferDMA_BYTES_PER_BURST 4
#define BufferDMA_REQUEST_PER_BURST 1
#define BufferDMA_SRC_BASE (CYDEV_PERIPH_BASE)
#define BufferDMA_DST_BASE (CYDEV_SRAM_BASE)

/* Variable declarations for BufferDMA */
/* Move these variable declarations to the top of the function */
uint8 BufferDMA_Chan;
uint8 BufferDMA_TD[1];

/* Defines for CurrentDMA */
#define CurrentDMA_BYTES_PER_BURST 4
#define CurrentDMA_REQUEST_PER_BURST 1
#define CurrentDMA_SRC_BASE (CYDEV_SRAM_BASE)
#define CurrentDMA_DST_BASE (CYDEV_SRAM_BASE)

/* Variable declarations for CurrentDMA */
/* Move these variable declarations to the top of the function */
static uint8 CurrentDMA_Chan;
static uint8 CurrentDMA_TD[1];

/* Variable declarations for SourceDMA */
/* Move these variable declarations to the top of the function */
#define ADCSAMPLES 40
static uint8 SourceDMA_Chan;
static uint8 SourceDMA_TD[1];
static volatile uint16 SourceData[ADCSAMPLES];

#define CURRENTSAMPLES 10
static volatile uint32 CurrentReadings[CURRENTSAMPLES];
static uint32 DMABuffer;

void DoPid();
void OutputEnable(bool v);

volatile static int32 systemTimer = 0, tempTimer = 0, tempTimer2 = 0, tempTimer3 = 0, logTime = 60000;

static float fiLimit;
volatile static uint32 iLimit = DEFAULT_I_LIM, iRest = 1, iPulse1 = 100, iPulse2 = 60;
volatile static uint16 vSource = 0, vSourceMin = 65535, vSourceMax = 0;
volatile static int iSource = 0;
volatile static uint32 dt = 0;
volatile static int vMin = DEFAULT_V_MIN, vRest = DEFAULT_V_MIN, vPulse1 = DEFAULT_V_MIN, vPulse2 = DEFAULT_V_MIN;
volatile static uint32 maHours;
unsigned int preset = 1, pulseDuration1 = 20, pulseInterval1 = 500, pulseDuration2 = 100, pulseInterval2 = 300, pulseGroupDuration = 1000, pulseGroupInterval = 6000, iter = 0;
uint8 page = 0x01u, vSourceMin1 = 0xffu, vSourceMin2 = 0xffu, vSourceMax1 = 0x00u, vSourceMax2 = 0x00u;
uint16 rowNum = 0x0000u;
bool enableOutput = false, isGroupOn = false, once = true, last = false, LCDToggle = true;

cystatus WriteData(uint8 _page, uint16 _rowNum, const uint8 *_rowData) {
	CySetTemp();
	return CyWriteRowData(_page, _rowNum, _rowData);
}

unsigned int numProfiles = 5; //This number must match the last preset number
void profile(unsigned int p) {
	if (p == 0) { //void profile
		preset = 0; //This number must be the same as the number in the if statement
		iRest = 0;
		iPulse1 = 0;
		iPulse2 = 0;
		vRest = 0;
		vMin = vRest; //Don't change this
		vPulse1 = 0;
		vPulse2 = 0;
		pulseDuration1 = 0;
		pulseInterval1 = 500;
		pulseDuration2 = 0;
		pulseInterval2 = 500;
		pulseGroupDuration = 0;
		pulseGroupInterval = 1000;
		logTime = 60000;
		return;
	}

	if (p == 1) { //default profile
		preset = 1; //This number must be the same as the number in the if statement
		iRest = 1;
		iPulse1 = 100;
		iPulse2 = 60;
		vRest = DEFAULT_V_MIN;
		vMin = vRest; //Don't change this
		vPulse1 = DEFAULT_V_MIN;
		vPulse2 = DEFAULT_V_MIN;
		pulseDuration1 = 20;
		pulseInterval1 = 500; //500
		pulseDuration2 = 100;
		pulseInterval2 = 300;
		pulseGroupDuration = 1000; //3000
		pulseGroupInterval = 6000; //9000
		logTime = 6000;
		return;
	}
	if (p == 2) {
		preset = 2; //This number must be the same as the number in the if statement
		iRest = 1;
		iPulse1 = 100;
		iPulse2 = 35;
		vRest = 1700;
		vMin = vRest; //Don't change this
		vPulse1 = 2800;
		vPulse2 = 2000;
		pulseDuration1 = 1;
		pulseInterval1 = 1200;
		pulseDuration2 = 30;
		pulseInterval2 = 300;
		pulseGroupDuration = 3000;
		pulseGroupInterval = 9000;
		logTime = 60000;
		return;
	}
	if (p == 3) {
		preset = 3; //This number must be the same as the number in the if statement
		iRest = 0;
		iPulse1 = 85;
		iPulse2 = 60;
		vRest = DEFAULT_V_MIN;
		vMin = vRest; //Don't change this
		vPulse1 = DEFAULT_V_MIN;
		vPulse2 = DEFAULT_V_MIN;
		pulseDuration1 = 10;
		pulseInterval1 = 100;
		pulseDuration2 = 25;
		pulseInterval2 = 400;
		pulseGroupDuration = 300;
		pulseGroupInterval = 1000;
		logTime = 60000;
		return;
	}
	if (p == 4) {
		preset = 4; //This number must be the same as the number in the if statement
		iRest = 0;
		iPulse1 = 60;
		iPulse2 = 46;
		vRest = DEFAULT_V_MIN;
		vMin = vRest; //Don't change this
		vPulse1 = DEFAULT_V_MIN;
		vPulse2 = DEFAULT_V_MIN;
		pulseDuration1 = 1;
		pulseInterval1 = 100;
		pulseDuration2 = 10;
		pulseInterval2 = 70;
		pulseGroupDuration = 200;
		pulseGroupInterval = 500;
		logTime = 60000;
		return;
	}
	if (p == 5) {
		preset = 5; //This number must be the same as the number in the if statement
		iRest = 0;
		iPulse1 = 90;
		iPulse2 = 60;
		vRest = DEFAULT_V_MIN;
		vMin = vRest; //Don't change this
		vPulse1 = DEFAULT_V_MIN;
		vPulse2 = DEFAULT_V_MIN;
		pulseDuration1 = 1;
		pulseInterval1 = 90;
		pulseDuration2 = 10;
		pulseInterval2 = 50;
		pulseGroupDuration = 250;
		pulseGroupInterval = 500;
		logTime = 60000;
		return;
	}
}

int main(void)
{
	static int i;
	static uint32 vAve;
	static char buff[64];
  char testBuff[2];
  uint8_t testBuffIndex = 0;
	static uint8 inBuff[64];
	static char floatBuff[64];
	static uint8_t incCharIndex = 0;

	CyGlobalIntEnable; /* Enable global interrupts. */

	// Create our timing "tick" variables. These are used to record when the last
	// iteration of an action in the main loop happened and to trigger the next
	// iteration when some number of clock ticks have passed.
	int32_t SlowTick = 0, /*myPulse1Tick = 0, myPulse2Tick = 0, myPulseGroupTick = 0, */myLogTick = 0;

	Gate_Drive_Tune_Start();
	Offset_Start();
	Offset_Gain_Start();
	GDT_Buffer_Start();
	O_Buffer_Start();
	Source_ADC_Start();
	I_Source_ADC_Start();
  UART_Start();
  
	bool upPressed = false;
	bool downPressed = false;
	bool entPressed = false;
	bool backPressed = false;

	USBUART_Start(0, USBUART_5V_OPERATION);
	CapSense_Start();
	CapSense_InitializeAllBaselines();
	ConversionClock_Start();
	LCD_Start();
	LCD_DisplayOn();
	LCD_PrintString("Hello World!");

	/* DMA Configuration for SourceDMA */
	SourceDMA_Chan = SourceDMA_DmaInitialize(SourceDMA_BYTES_PER_BURST, SourceDMA_REQUEST_PER_BURST,
		HI16(SourceDMA_SRC_BASE), HI16(SourceDMA_DST_BASE));
	SourceDMA_TD[0] = CyDmaTdAllocate();
	CyDmaTdSetConfiguration(SourceDMA_TD[0], 2 * ADCSAMPLES, SourceDMA_TD[0], CY_DMA_TD_INC_DST_ADR);
	CyDmaTdSetAddress(SourceDMA_TD[0], LO16((uint32)Source_ADC_SAR_WRK0_PTR), LO16((uint32)SourceData));
	CyDmaChSetInitialTd(SourceDMA_Chan, SourceDMA_TD[0]);
	Source_ADC_StartConvert();
	CyDmaChEnable(SourceDMA_Chan, 1);

	/* DMA Configuration for BufferDMA */

	BufferDMA_Chan = BufferDMA_DmaInitialize(BufferDMA_BYTES_PER_BURST, BufferDMA_REQUEST_PER_BURST,
		HI16(BufferDMA_SRC_BASE), HI16(BufferDMA_DST_BASE));
	BufferDMA_TD[0] = CyDmaTdAllocate();
	CyDmaTdSetConfiguration(BufferDMA_TD[0], 4, BufferDMA_TD[0], TD_INC_SRC_ADR | BufferDMA__TD_TERMOUT_EN);
	CyDmaTdSetAddress(BufferDMA_TD[0], LO16((uint32)I_Source_ADC_DEC_SAMP_PTR), LO16((uint32)DMABuffer));
	CyDmaChSetInitialTd(BufferDMA_Chan, BufferDMA_TD[0]);

	/* DMA Configuration for CurrentDMA */
	CurrentDMA_Chan = CurrentDMA_DmaInitialize(CurrentDMA_BYTES_PER_BURST, CurrentDMA_REQUEST_PER_BURST,
		HI16(CurrentDMA_SRC_BASE), HI16(CurrentDMA_DST_BASE));
	CurrentDMA_TD[0] = CyDmaTdAllocate();
	CyDmaTdSetConfiguration(CurrentDMA_TD[0], 4 * CURRENTSAMPLES, CurrentDMA_TD[0], TD_INC_DST_ADR | CurrentDMA__TD_TERMOUT_EN);
	CyDmaTdSetAddress(CurrentDMA_TD[0], LO16((uint32)DMABuffer), LO16((uint32)CurrentReadings));
	CyDmaChSetInitialTd(CurrentDMA_Chan, CurrentDMA_TD[0]);

	/*Change the ADC coherent key to high byte*/
	I_Source_ADC_DEC_COHER_REG |= I_Source_ADC_DEC_SAMP_KEY_HIGH;
	I_Source_ADC_StartConvert();
	CyDmaChEnable(BufferDMA_Chan, 1);
	CyDmaChEnable(CurrentDMA_Chan, 1);

	PIDIsr_Start();

	init();

	// testing flash write and DMA read. **********************************************************************************************************************
	/*const uint8 testVoltage[] = {110};
	CySetTemp();
	CyWriteRowData(0x00u, 0x0000u, testVoltage);
	uint8* TEST = (uint8*)(CY_FLASH_BASE + ((0x00u * CY_FLASH_SIZEOF_ARRAY) + (0x0000u * CY_FLASH_SIZEOF_ROW)));*/
	/*uint16* TEST;
	uint8 yes = SourceDMA_DmaInitialize(0, 0, 0x00000000u, *TEST);*/
	//uint32 rowNumTemp = (uint32)&block << 16;
	//uint16 rowNum = rowNumTemp >> 16;
	// cystatus p;
    // CySetTemp();
	// CyWriteRowData(0x02u, 0x0000u, rowData);


	for (;;)
	{
		if (systemTimer - logTime > systemTimer)
			systemTimer += logTime;
		
		if (0u != USBUART_IsConfigurationChanged())
		{
			if (0u != USBUART_GetConfiguration())
			{
				USBUART_CDC_Init();
			}
		}
    if (0u != USBUART_IsConfigurationChanged())
    {
      if (0u != USBUART_GetConfiguration())
      {
        USBUART_CDC_Init();
      }
    }
    
    // UART is used during production test to test programming
    //  of chip. Test fixture issues '1', expects response of '2'.
    if (UART_GetRxBufferSize())
    {
      while (UART_GetRxBufferSize())
      {
        testBuff[testBuffIndex++] = UART_GetChar();
      }
      if (testBuff[testBuffIndex - 1] == '1')
      {
        UART_PutChar('2');
      }
    }
    if (testBuffIndex > 1)
    {
      testBuffIndex = 0;
      testBuff[0] = '\0';
      testBuff[1] = '\0';
    }
	
		if (0u == CapSense_IsBusy())
		{
			CapSense_UpdateEnabledBaselines();
			CapSense_ScanEnabledWidgets();
		}

		// Handle a press of the back key
		if (CapSense_CheckIsWidgetActive(CapSense_BACK__BTN) && backPressed == false)
		{
			backPressed = true;
			iRest = DEFAULT_I_LIM;
			vRest = DEFAULT_V_MIN;
			OutputEnable(false);
		}
		else if (!CapSense_CheckIsWidgetActive(CapSense_BACK__BTN) && backPressed == true)
		{
			backPressed = false;
		}

		if (CapSense_CheckIsWidgetActive(CapSense_ENTER__BTN) && entPressed == false)
		{
			entPressed = true;
			OutputEnable(!enableOutput);
		}
		else if (!CapSense_CheckIsWidgetActive(CapSense_ENTER__BTN) && entPressed == true)
		{
			entPressed = false;
		}

		if (CapSense_CheckIsWidgetActive(CapSense_DOWN__BTN) && downPressed == false)
		{
			downPressed = true;
			if (preset == 0)
				profile(1);
			else
				profile(preset - 1);
			/*old down button logic
			if (iRest < 10) iRest = 0;
			else if (iRest < 101) { if (iRest >= 10) iRest -= 10; }
			else if (iRest < 501) iRest -= 50;
			else if (iRest < 1001) iRest -= 100;
			else iRest -= 500;*/
		}
		else if (!CapSense_CheckIsWidgetActive(CapSense_DOWN__BTN) && downPressed == true)
		{
			downPressed = false;
		}

		if (CapSense_CheckIsWidgetActive(CapSense_UP__BTN) && upPressed == false)
		{
			upPressed = true;
			if (preset == numProfiles)
				profile(1);
			else
				profile(preset + 1);
			/*old up button logic
			if (iRest < 99) iRest += 10;
			else if (iRest < 500) iRest += 50;
			else if (iRest < 1000) iRest += 100;
			else if (iRest < 3500) iRest += 500;
			else iRest = 4000;*/
		}
		else if (!CapSense_CheckIsWidgetActive(CapSense_UP__BTN) && upPressed == true)
		{
			upPressed = false;
		}


		// Fetch any waiting characters from the USB UART.
		int charCount = USBUART_GetCount();
		if (charCount > 0)
		{
			int i = USBUART_GetAll(inBuff);

			for (i = 0; i < charCount; i++)
			{
				floatBuff[incCharIndex++] = inBuff[i];
				if (incCharIndex > 63)
					incCharIndex = 0;
				if (inBuff[i] == '\r' ||
					inBuff[i] == '\n')
				{
					floatBuff[incCharIndex] = '\0';
					break;
				}
			}
		}

		// Average the DMA data from the Source Voltage ADC
		vAve = 0;
		for (i = 0; i < ADCSAMPLES; i++)
			vAve += SourceData[i];

    vAve = vAve/40;
		vSource = 10*Source_ADC_CountsTo_mVolts((uint16)(vAve)); // 10*vAve/40

		if (vSource < vSourceMin) {
			vSourceMin = vSource;
			vSourceMin1 = (vSourceMin >> 8) & 0xff;
			vSourceMin2 = vSourceMin & 0xff;
		}
		if (vSource > vSourceMax) {
		    vSourceMax = vSource;
			vSourceMax1 = (vSourceMax >> 8) & 0xff;
			vSourceMax2 = vSourceMax & 0xff;
		}
		
		if (vSource < vPulse2) {
			if (tempTimer != 0)
				iLimit = iPulse1;
			else
				iLimit = iRest;

			if (enableOutput) {
				pulseDuration2 = 0;
				vPulse2 = 0;

				if (iter > 246) {
					WriteData(page, rowNum, rowData);
					iter = 0;
					rowNum++;
				}

				rowData[iter] = 0xff;
				rowData[iter+1] = 0xff;
				rowData[iter+2] = 0xff;
				rowData[iter+3] = 0xff;
				
				rowData[iter+4] = (maHours >> 24) & 0xff;
				rowData[iter+5] = (maHours >> 16) & 0xff;
				rowData[iter+6] = (maHours >> 8) & 0xff;
				rowData[iter+7] = maHours & 0xff;
				
				iter += 8;
			}
		}
		if (vSource < vPulse1) {
			if (tempTimer2 != 0)
				iLimit = iPulse2;
			else
				iLimit = iRest;

			if (enableOutput) {
				pulseDuration1 = 0;
				vPulse1 = 0;

				if (iter > 246) {
					WriteData(page, rowNum, rowData);
					iter = 0;
					rowNum++;
				}

				rowData[iter] = 0xff;
				rowData[iter+1] = 0xff;
				rowData[iter+2] = 0xff;
				rowData[iter+3] = 0xff;
				
				rowData[iter+4] = (maHours >> 24) & 0xff;
				rowData[iter+5] = (maHours >> 16) & 0xff;
				rowData[iter+6] = (maHours >> 8) & 0xff;
				rowData[iter+7] = maHours & 0xff;
				
				iter += 8;
			}
		}

		// Turns pulses on and off. Also turns the group of pulses on and off.
		// Changes voltage minimums with pulses.
		if (!isGroupOn)
			iLimit = iRest;
		if (systemTimer % pulseGroupInterval == 0/*systemTimer - pulseGroupInterval > myPulseGroupTick*/) {
			// myPulseGroupTick = systemTimer;
			isGroupOn = true;
			tempTimer3 = systemTimer + pulseGroupDuration;
		}
		if ((systemTimer % pulseInterval2 == 0/*systemTimer - pulseInterval2 > myPulse2Tick*/) && isGroupOn && (pulseDuration2 > 0)) {
			// myPulse2Tick = systemTimer;
			iLimit = iPulse2;
			tempTimer2 = systemTimer + pulseDuration2;
		}
		if ((systemTimer % pulseInterval1 == 0/*systemTimer - pulseInterval1 > myPulse1Tick*/) && isGroupOn && (pulseDuration1 > 0)) {
			// myPulse1Tick = systemTimer;
			iLimit = iPulse1;
			tempTimer = systemTimer + pulseDuration1;
		}
		if (systemTimer == tempTimer2) {
			iLimit = iRest;
			tempTimer2 = 0;
		}
		if (systemTimer == tempTimer) {
			if (tempTimer2 != 0)
				iLimit = iPulse2;
			else
				iLimit = iRest;
			tempTimer = 0;
		}
		if (systemTimer == tempTimer3) {
			isGroupOn = false;
			tempTimer3 = 0;
		}
		
		//voltage data storage to flash.
		if (page == 0x03u) {
			OutputEnable(false);
			page = 0x01u;
			rowNum = 0;
			iter = 0;
			last = false;
			once = true;
		}
		if (rowNum == 256) {
			rowNum = 0;
			page++;
		}
		if (iter == 256 || (!enableOutput && last)) {
			if (!enableOutput && last) {
				last = false;

				if (iter > 246)
					iter = 246;
				rowData[iter] = 0xff;
				rowData[iter+1] = 0xff;
				rowData[iter+2] = 0xff;
				rowData[iter+3] = 0xff;
				
				rowData[iter+4] = (maHours >> 24) & 0xff;
				rowData[iter+5] = (maHours >> 16) & 0xff;
				rowData[iter+6] = (maHours >> 8) & 0xff;
				rowData[iter+7] = maHours & 0xff;
				
				iter += 8;
				
				for (unsigned int it = iter; it < 256; it++) {
					rowData[it] = 0x00;
				}
				once = true;
			}
	
			WriteData(page, rowNum, rowData);
	
			if (once) {
				for (unsigned int it = 0; it < iter + 1; it++) {
					rowData[it] = 0x00;
				}
			}
	
			iter = 0;
			rowNum++;
		}
		if ((systemTimer - logTime > myLogTick) && enableOutput && (logTime > 0)) {
			myLogTick = systemTimer;
	
			if (once) {
				last = true;
				once = false;
				
				if (page == 1 && rowNum == 0 && iter == 0) {
					rowData[0] = (preset >> 8) & 0xff;
					rowData[1] = preset && 0xff;

					rowData[2] = (vRest >> 8) & 0xff;
					rowData[3] = vRest & 0xff;

					rowData[4] = (vPulse1 >> 8) & 0xff;
					rowData[5] = vPulse1 & 0xff;

					rowData[6] = (vPulse2 >> 8) & 0xff;
					rowData[7] = vPulse2 & 0xff;

					rowData[8] = (iRest >> 8) & 0xff;
					rowData[9] = iRest & 0xff;

					rowData[10] = (iPulse1 >> 8) & 0xff;
					rowData[11] = iPulse1 & 0xff;

					rowData[12] = (pulseDuration1 >> 8) & 0xff;
					rowData[13] = pulseDuration1 & 0xff;

					rowData[14] = (pulseInterval1 >> 8) & 0xff;
					rowData[15] = pulseInterval1 & 0xff;

					rowData[16] = (iPulse2 >> 8) & 0xff;
					rowData[17] = iPulse2 & 0xff;

					rowData[18] = (pulseDuration2 >> 8) & 0xff;
					rowData[19] = pulseDuration2 & 0xff;

					rowData[20] = (pulseInterval2 >> 8) & 0xff;
					rowData[21] = pulseInterval2 & 0xff;

					rowData[22] = (pulseGroupDuration >> 8) & 0xff;
					rowData[23] = pulseGroupDuration & 0xff;

					rowData[24] = (pulseGroupInterval >> 8) & 0xff;
					rowData[25] = pulseGroupInterval & 0xff;

					rowData[26] = (logTime >> 8) & 0xff;
					rowData[27] = logTime & 0xff;

					iter = 28;
				}
			}

			rowData[iter] = vSourceMin1;
			rowData[iter+1] = vSourceMin2;
			rowData[iter+2] = vSourceMax1;
			rowData[iter+3] = vSourceMax2;
			iter += 4;
			vSourceMin = 65535;
			vSourceMax = 0;
		}

		//goToPos(1, 1);
		//sprintf(buff, "%s", block);
		//unsigned int checker = block;

		if (systemTimer - 20 > SlowTick)
		{
			SlowTick = systemTimer;
			cls();
			goToPos(1, 1);
			if (preset == 0 )
				sprintf(buff, "Custom Profile (S)");
			else
				sprintf(buff, "Profile %u (S)", preset);
			putString(buff);
			goToPos(5, 2);
			putString("V Source:");
			goToPos(5, 3);
			putString("I Source:");
			goToPos(5, 4);
			putString("I Limit:");
			goToPos(5, 6);
			putString("V Min:");
			goToPos(5, 7);
			putString("V Rest (M):");
			goToPos(5, 8);
			putString("V Pulse1 (O):");
			goToPos(5, 9);
			putString("V Pulse2 (F):");
			goToPos(5, 11);
			putString("I Rest (R):");
			goToPos(5, 13);
			putString("I Pulse1 (P):");
			goToPos(5, 14);
			putString("P Duration1 (D):");
			goToPos(5, 15);
			putString("P Interval1 (N):");
			goToPos(5, 17);
			putString("I Pulse2 (U):");
			goToPos(5, 18);
			putString("P Duration2 (A):");
			goToPos(5, 19);
			putString("P Interval2 (T):");
			goToPos(5, 21);
			putString("Group Dur (G):");
			goToPos(5, 22);
			putString("Group Int (W):");
			goToPos(5, 24);
			putString("Log Time (L):");
			goToPos(5, 25);
			putString("mA Hours (Z):");
			goToPos(5, 26);
			putString("Enabled? (E):");
			goToPos(5, 27);
			putString("Clear Flash (C)");
			// sprintf(buff, "%x", *(ptr+0)); //make a myBuff array that is 256 bytes large and see if that will print the whole thing to the serial port.
			// putString(buff);
			/*goToPos(1, 17);
			sprintf(buff, "%u", *TEST);
			putString(buff);*/
			goToPos(22, 2);
			sprintf(buff, "%6.3fV", (float)vSource / 1000.0f);
			putString(buff);
			goToPos(22, 3);
			if (iSource < 0) iSource *= -1;
			sprintf(buff, "%6.3fA", iSource / 1000.0f);
			putString(buff);
			goToPos(22, 4);
			sprintf(buff, "%6.3fA", iLimit / 1000.0f);
			putString(buff);
			goToPos(22, 6);
			sprintf(buff, "%6.3fV", vMin / 1000.0f);
			putString(buff);
			goToPos(22, 7);
			sprintf(buff, "%6.3fV", vRest / 1000.0f);
			putString(buff);
			goToPos(22, 8);
			sprintf(buff, "%6.3fV", vPulse1 / 1000.0f);
			putString(buff);
			goToPos(22, 9);
			sprintf(buff, "%6.3fV", vPulse2 / 1000.0f);
			putString(buff);
			goToPos(22, 11);
			sprintf(buff, "%6.3fA", iRest / 1000.0f);
			putString(buff);
			goToPos(22, 13);
			sprintf(buff, "%6.3fA", iPulse1 / 1000.0f);
			putString(buff);
			goToPos(22, 14);
			sprintf(buff, "%6.2fs", pulseDuration1 / 100.0f);
			putString(buff);
			goToPos(22, 15);
			sprintf(buff, "%6.2fs", pulseInterval1 / 100.0f);
			putString(buff);
			goToPos(22, 17);
			sprintf(buff, "%6.3fA", iPulse2 / 1000.0f);
			putString(buff);
			goToPos(22, 18);
			sprintf(buff, "%6.2fs", pulseDuration2 / 100.0f);
			putString(buff);
			goToPos(22, 19);
			sprintf(buff, "%6.2fs", pulseInterval2 / 100.0f);
			putString(buff);
			goToPos(22, 21);
			sprintf(buff, "%6.2fs", pulseGroupDuration / 100.0f);
			putString(buff);
			goToPos(22, 22);
			sprintf(buff, "%6.2fs", pulseGroupInterval / 100.0f);
			putString(buff);
			goToPos(22, 24);
			sprintf(buff, "%6.2fs", logTime / 100.0f);
			putString(buff);
			goToPos(22, 25);
			sprintf(buff, "%6.2f", maHours / 3600.0f);
			putString(buff);
			goToPos(23, 26);
			if (enableOutput)
				putString("  Yes");
			else
				putString("   No");
			goToPos(5, 28);

			if (LCDToggle) {
				sprintf(buff, "Prf%u V:%.3f    ", preset, vSource / 1000.0f);
				LCD_Position(0, 0);
				LCD_PrintString(buff);
				if (systemTimer % 40 == 0)
					LCDToggle = false;
			}
			else {
				sprintf(buff, "Prf%u mAH:%.3f", preset, maHours / 3600.0f);
				LCD_Position(0, 0);
				LCD_PrintString(buff);
				if (systemTimer % 40 == 0)
					LCDToggle = true;
			}
			if (pulseDuration1 != 0 && pulseDuration2 != 0)
				sprintf(buff, "P1:ON P2:ON  ");
			else if (pulseDuration1 != 0 && pulseDuration2 == 0)
				sprintf(buff, "P1:ON P2:OFF ");
			else if (pulseDuration1 == 0 && pulseDuration2 != 0)
				sprintf(buff, "P1:OFF P2:ON ");
			else if (pulseDuration1 == 0 && pulseDuration2 == 0)
				sprintf(buff, "P1:OFF P2:OFF");
			LCD_Position(1, 0);
			LCD_PrintString(buff);
			/* Old LCD Printing
			sprintf(buff, "I: %.2f V: %.2f", iLimit / 1000.0f, vSource / 1000.0f);
			LCD_Position(0, 0);
			LCD_PrintString(buff);
			sprintf(buff, "Imeas: %.2f", iSource / 1000.0f);
			LCD_Position(1, 0);
			LCD_PrintString(buff);*/

			if (floatBuff[incCharIndex] == '\0')
			{
				fiLimit = atoff(floatBuff + 1);
				switch (toupper(floatBuff[0]))
				{
				case 'S': //profile
					if (fiLimit > 5 ||
						fiLimit < 0) fiLimit = 1;
					profile(fiLimit);
					break;
				case 'V': //voltage minimum
					vMin = 1000.0*fiLimit;
					break;
				case 'M': //rest voltage minimum
					vRest = 1000.0*fiLimit;
					vMin = vRest;
					break;
				case 'O': //pulse voltage minimum
					vPulse1 = 1000.0*fiLimit;
					break;
				case 'F': //pulse voltage minimum
					vPulse2 = 1000.0*fiLimit;
					break;
				case 'I': //current limit
					if (fiLimit > 4.000 ||
						fiLimit < 0.0) fiLimit = 0.0;
					iLimit = 1000.0*fiLimit;
					break;
				case 'R': //rest current
					if (fiLimit > 4.000 ||
						fiLimit < 0.0) fiLimit = 0.0;
					iRest = 1000.0*fiLimit;
					break;
				case 'P': //pulse current
					if (fiLimit > 4.000 ||
						fiLimit < 0.0) fiLimit = 0.0;
					iPulse1 = 1000.0*fiLimit;
					break;
				case 'D': //duration of pulse
					pulseDuration1 = 100.0*fiLimit;
					break;
				case 'N': //interval between pulses
					pulseInterval1 = 100.0*fiLimit;
					break;
				case 'U': //pulse current
					if (fiLimit > 4.000 ||
						fiLimit < 0.0) fiLimit = 0.0;
					iPulse2 = 1000.0*fiLimit;
					break;
				case 'A': //duration of pulse
					pulseDuration2 = 100.0*fiLimit;
					break;
				case 'T': //interval between pulses
					pulseInterval2 = 100.0*fiLimit;
					break;
				case 'G': //length of group of pulses
					pulseGroupDuration = 100.0*fiLimit;
					break;
				case 'W': //interval between group of pulses
					pulseGroupInterval = 100.0*fiLimit;
					break;
				case 'E': //enable
					OutputEnable(fiLimit == 1);
					break;
				case 'Z': //reset
					maHours = 0;
					break;
				case 'L':
					logTime = 100.0*fiLimit;
					break;
				case ',': //prints data to serial port ("logs" data)
					cls();
					goToPos(1, 1);
					putString("'");
					unsigned char* ptr = (unsigned char*)0x00010000u;
					for (unsigned int i = 0x0u; i < 0x2u; i++) {
						for (unsigned int j = 0x0u; j < 0x100u; j++) {
							for (unsigned int k = 0x0u; k < 0x100u; k++) {
								sprintf(buff, "%02x", *(ptr + (0x10000u * i) + (0x100 * j) + k));
								putString(buff);
							}
						}
					}
					putString("\"");
					break;
				case 'C': //clear flash
					cls();
					goToPos(1, 1);
					putString("Clearing Flash Storage...");
					//clears data storage section of flash.
					for (unsigned int i = 0; i < 256; i++) {
						CySetTemp();
						CyFlash_EraseRow(0x01u, i);
						CySetTemp();
						CyFlash_EraseRow(0x02u, i);
					}
					break;
        case 'B': //bootloader
          Bootloadable_Load();
          CySoftwareReset();
          break; // We'll never see this because the previous line resets the
                 //  processor.
				default:
					break;
				}

				memset(floatBuff, 1, 64);
				incCharIndex = 0;
			}
		}
	}
}

void PIDIsr_Interrupt_InterruptCallback()
{
	systemTimer++;
	if (!enableOutput)
		iLimit = 0;
	DoPid();
}

void DoPid()
{
	static int error = 0;
	static int integral = 0;
	static int32_t iSourceRaw = 0;
	static uint16_t grossSetPoint = 0;
	static uint16_t fineSetPoint = 0;
	static int setPoint = 0;
	static int i;
	static int loopCount = 0;

	iSourceRaw = 0;
	for (i = 0; i < 10; i++)
		iSourceRaw += CurrentReadings[i];

	iSource = I_Source_ADC_CountsTo_mVolts(iSourceRaw);

	error = iLimit - iSource;
	integral = integral + error;
	setPoint = (KPN * iLimit) / KPD + (KIN * integral) / KID + 2000; // Use feed forward plus integral
	if (setPoint < 0)
		setPoint = 0;

	// setPoint is a voltage. We need to convert that
	//  into an integer that can be fed into our DACs.
	// First, find our grossSetPoint. This is a largish voltage that
	//  represents a coarse 0-4V offset in 16mV steps.
	grossSetPoint = (int)(setPoint / 16);
	// We want to limit our gross offset to 255, since it's an 8-bit
	//  DAC output.
	if (grossSetPoint > 255)
		grossSetPoint = 255;
	// Now, find the fineSetPoint. This is a 4mV step 8-bit DAC which
	//  allows us to tune the set point a little more finely.
	fineSetPoint = (setPoint - grossSetPoint * 16) / 4;
	if (fineSetPoint > 255)
		fineSetPoint = 255;

	// Finally, one last check: if the source voltage is below vMin,
	//  or the total power is greater than 15W,
	//  disable.
	if ((vSource < vMin) ||
		(vSource * iLimit > 15000000))
		OutputEnable(false);

	if (enableOutput == false)
	{
		error = 0;
		integral = 0;
		grossSetPoint = 0;
		fineSetPoint = 0;
	}
	else
	{
		// maHours += iSource; My code. I tried to have the maHours update faster to be more accurate but it did not work.
		if (loopCount++ == 100)
		{
			maHours += iSource;
			loopCount = 0;
		}
	}

	Offset_SetValue(grossSetPoint);
	Gate_Drive_Tune_SetValue(fineSetPoint);
}

void OutputEnable(bool v)
{
	enableOutput = v;
	if (enableOutput)
		Output_On_LED_Write(1);
	else
		Output_On_LED_Write(0);
}

