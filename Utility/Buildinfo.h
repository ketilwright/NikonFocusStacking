/*
 * BuildInfo.h
 *
 * Created: 6/20/2014 9:22:00 AM
 *  Author: Ketil
 */


#ifndef BUILDINFO_H_
#define BUILDINFO_H_

#include <stddef.h>
#include <stdint.h>
#include <avr/eeprom.h>
#include <avr/pgmspace.h>
#include <string.h>

#define BUILDINFO_RAM 1			// Store build information in RAM
//#define BUILDINFO_EEMEM 1		// Store build information in EEMEM (NOT SUPPORTED by bootloader)
//#define BUILDINFO_PROGMEM 1   // Store build information in PROGMEM
#define ENABLE_TEXT_MESSAGE 1	// Show build information during program initialization

const int HeaderMsgSize = 20;
const int BuildNumberSize = sizeof(uint16_t);

/* implemented in buildate.cpp */
void GetBuildDate( void *buffer, size_t bufferSize );

/* Get Build Number (times deployed to device) */
uint16_t GetBuildNumber( );

/* Show build information */
void ShowBuildInfo();

/* Return the amount of free RAM available */
int GetFreeRam ();



#endif /* BUILDINFO_H_ */