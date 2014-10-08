/*
 * Main.cpp
 *
 * Created: 5/24/2014 1:15:02 PM
 *  Author: Ketil Wright
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


/*
    TODO:
    1) Various camera settings can interfere with focus stacking
        i) Auto focus must be enabled on the lens
        ii) AF must be enabled on the camera
        iii) AF mode must be AFS (not AFC).
*/

#ifndef SKETCH_H_
#include "Sketch.h"
#endif

#include <Usb.h>
#include <usbhub.h>
#include <ptp.h>
#include <ptpdebug.h>
#include <avr/eeprom.h>

// app includes
#include "Adafruit_RGBLCDShield.h"
#include "ReadAdaFruitLcdButtonState.h"
#include "NikType003.h"
#include "NikType003State.h"
#include "MessagePump.h"
#include "MessageHandler.h"
#include "MainMenu.h"
#include "SetupHandler.h"
#include "ManualHandler.h"
#include "RunFocusStackHandler.h"
#include "Button.h"
#include "LcdImpl.h"


// g_pump dispatches MSG from Button objects to the current IMessageHandler
MessagePump		g_pump(NULL);
// Default handler displayed on startup, and return from the other
// handlers.
MainMenuHandler	g_main(&g_pump);
// Handler to setup focus stack parameters
SetupHandler	g_setup(&g_pump, 250, 10);
// Handler to manually view the manual focus amount
ManualHandler   g_manual(&g_pump);
// Handler for focus stack execution
RunFocusStackHandler g_runStack(&g_pump);
// Global pointers for other modules
MainMenuHandler         *g_pMain     = &g_main;
SetupHandler            *g_pSetup	 = &g_setup;
ManualHandler			*g_pManual	 = &g_manual;
RunFocusStackHandler    *g_pRunStack = &g_runStack;
MessagePump             *g_pPump     = &g_pump;

// USB/Ptp objects
USB                 Usb;
//USBHub              Hub1(&Usb);
NikType003StateHandler nk3State;
NikType003  nk3(&Usb, &nk3State);

// EEPROM variables
uint16_t EEMEM ePromFocusAmount = 100;
uint16_t EEMEM ePromFrameDelay  = 0;
uint16_t EEMEM ePromNumFrames = 10;
uint8_t  EEMEM ePromRestoreFocus = 0;

uint16_t g_savedFocusAmount = 100;
uint16_t g_savedFrameDelay = 0;
uint16_t g_savedNumFrames = 3;
uint8_t g_savedRestoreFocus = 0;
bool g_usbOK = true;


// The shield uses the I2C SCL and SDA pins. On classic Arduinos
// this is Analog 4 and 5 so you can't use those for analogRead() anymore
// However, you can connect other I2C sensors to the I2C bus and share
// the I2C bus.
// Modify LCDImpl.h for whateved LCD implementation is used.
LCDImpl lcd = LCDImpl();
LCDImpl *g_print = &lcd;
ReadAdaFruitLcdButtonState buttonStateReader (g_print);
// Buttons also implemented using the Adafruit LCD shield kit.
Button buttonLeft(&buttonStateReader, BUTTON_LEFT, eLeft, HIGH);
Button buttonRight(&buttonStateReader, BUTTON_RIGHT, eRight, HIGH);
Button buttonUp(&buttonStateReader, BUTTON_UP, eUp, HIGH);
Button buttonDown(&buttonStateReader, BUTTON_DOWN, eDown, HIGH);
Button buttonSelect(&buttonStateReader, BUTTON_SELECT, eSelect, HIGH);
Button* buttons[5] = {&buttonLeft, &buttonRight, &buttonUp, &buttonDown, &buttonSelect};

#include ".\Utility\BuildInfo.h"

void setup() {
	// Debugging output
	Serial.begin(115200);
	// set up the LCD's number of columns and rows:
	lcd.begin(16, 2);
	lcd.cursor();
	// Setup usb.
	if (Usb.Init() == -1)
	{
		g_print->print(F("USB did not start."));
		g_usbOK = false;
	}
    else
    {
        char bufBuildDate[21];
        GetBuildDate(bufBuildDate, sizeof(bufBuildDate));
        g_print->print(bufBuildDate);
        delay(1000);
    }        
	// Restore previous settings
	if((g_savedFocusAmount = eeprom_read_word(&ePromFocusAmount)) != 0xffff)
	{
		g_setup.setDriveAmount(g_savedFocusAmount);	
	}
	
	if((g_savedFrameDelay = eeprom_read_word(&ePromFrameDelay)) != 0xffff)
	{
		g_setup.setFrameDelaySeconds(g_savedFrameDelay);	
	}
	
	if((g_savedNumFrames = eeprom_read_word(&ePromNumFrames)) != 0xffff)
	{
		g_setup.setNumFrames(g_savedNumFrames);
	}
	if((g_savedRestoreFocus = eeprom_read_byte(&ePromRestoreFocus)) != 0xff)
	{
		g_setup.setRestoreFocus(g_savedRestoreFocus);	
	}
}

void loop()
{
	Usb.Task();
	for(size_t b = 0; b < sizeof(buttons) / sizeof(buttons[0]); b++)
	{
		Msg& msg = buttons[b]->getMsg();
		
		if(msg.m_type != eButtonActionNone)
		{
			
			 g_pump.dispatch(msg);
		}
	}
	// Time the shutter release in case the user wants to let the flash recharge.
	const unsigned long now = millis();
	if(nk3.isConnected() && nk3.isFocusStackActive())
	{
		// we're connected and a focus stack op is in progress
		if(!nk3.isCaptureInProgress())
		{
			// Capture is complete. If there is no frame delay, prepare
			// the next frame & shoot it
			if(0 == g_pSetup->getFrameDelayMilliseconds())
			{
				if(!nk3.isNextFrameFocused())
				{
					if(PTP_RC_OK == nk3.prepareNextFrame())
					{
						nk3.focusStackNextFrame();
					}	
				}
			}
			// else user requested a frame delay. Use that time to prepare the next frame
			else if(!nk3.isNextFrameFocused())
			{
				nk3.prepareNextFrame();
			}
			// else we're focused & waiting on the frame delay
			else if(now - nk3.getTimeLastCaptureStart() >= g_pSetup->getFrameDelayMilliseconds())
			{
				nk3.focusStackNextFrame();
			}
			// else update the frame delay countdown once a second.
			if(0 != g_pSetup->getFrameDelayMilliseconds())
			{
				g_pRunStack->reportDelay(now);
			}
		}
	}
}

