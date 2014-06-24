/*
 * SetupHandler.cpp
 *
 * Created: 4/29/2014 11:35:56 AM
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

#include "SetupHandler.h"
#include "LcdImpl.h"
#include <avr/eeprom.h>

extern uint16_t g_savedFocusAmount;
extern uint8_t g_savedNumFrames;

extern uint16_t EEMEM ePromFocusAmount;
extern uint16_t EEMEM ePromFrameDelay;
extern uint8_t  EEMEM ePromNumFrames;
extern uint8_t  EEMEM ePromRestoreFocus;

extern IMessageHandler *g_pMain;

// column start/end for setup field
#define AmountFieldStart 0
#define AmountFieldEnd   3
#define FrameFieldStart  5
#define FrameFieldEnd    7
#define DelayFieldStart  9
#define DelayFieldEnd	 11
#define RestoreFocusFieldStart 13
#define RestoreFocusFieldEnd 13



SetupHandler::SetupHandler(MessagePump *_pump, uint32_t driveAmount, uint32_t frames)
    :
    IMessageHandler(_pump),
    m_driveAmount(driveAmount),
    m_numFrames(frames),
	m_frameDelaySeconds(0),
	m_restoreFocus(false)
{
    menu[0] = "Amt";
    menu[1] = "Frm";
	menu[2] = "Dly";
	menu[3] = "Rst";
};
SetupHandler::~SetupHandler()
{}

void SetupHandler::advanceCaret(uint8_t dir) // -1 = left, 1 right. All other values ignored
{
	unsigned char caretColumn = getCaretCol();
	if(dir == 0xff)
	{
		switch(caretColumn)
		{
			case AmountFieldStart:			{ caretColumn = RestoreFocusFieldEnd; break;}
			case FrameFieldStart:			{ caretColumn = AmountFieldEnd; break;}
			case DelayFieldStart:			{ caretColumn = FrameFieldEnd; break;}
			case RestoreFocusFieldStart:	{ caretColumn = DelayFieldEnd; break;}
			default:						{ --caretColumn; break;}				
		}
	}
	else if(dir == 1)
	{
		switch(caretColumn)
		{
			case AmountFieldEnd:			{ caretColumn = FrameFieldStart; break;}
			case FrameFieldEnd:				{ caretColumn = DelayFieldStart; break;}
			case DelayFieldEnd:				{ caretColumn = RestoreFocusFieldStart; break;}
			case RestoreFocusFieldEnd:		{ caretColumn = AmountFieldStart; break;}
			default:						{ ++caretColumn; break;}
		}
	}
	moveCaret(caretColumn, 1);
}

MsgResp SetupHandler::processMessage(Msg& msg)
{
    MsgResp rsp = eFail;
	if(eButtonActionPress != msg.m_type) return rsp;
	// the amount up or down some parameter will be changed
	unsigned char caretColumn = getCaretCol();
	int change = 0;
    // which button?
    switch(msg.m_code)
    {
        case eDown:
        {
			if((caretColumn <= AmountFieldEnd) && (msg.m_type == eButtonActionPress))
			{
				// change amount entry
				change = -1000; // todo: consider a power of ten == AmountFieldEnd - AmountFieldStart + 1
				while(caretColumn-- != AmountFieldStart)
				{
					change /= 10;
				}
				updateDriveAmountUI(change);
			}
			else if((caretColumn >= FrameFieldStart) && (caretColumn <= FrameFieldEnd))
			{
				change = -100;// todo: consider a power of ten == FrameFieldEnd - FrameFieldStart + 1
				while(caretColumn-- != FrameFieldStart)
				{
					change /= 10;
				}
				updateFramesUI(change);
			}
			else if((caretColumn >= DelayFieldStart) && (caretColumn <= DelayFieldEnd))
			{
				// todo: consider a power of ten == DelayFieldEnd - DelayFieldStart + 1
				change = -100;
				while(caretColumn-- != DelayFieldStart)
				{
					change /= 10;
				}
				updateFrameDelayUI(change);
			}
			else if((caretColumn >= RestoreFocusFieldStart) && (caretColumn <= RestoreFocusFieldEnd))
			{
				updateRestoreFocusUI(-1);
			}
            break;
        }
        case eUp:
        {
			if((caretColumn <= AmountFieldEnd) && (msg.m_type == eButtonActionPress))
			{
				// change amount entry
				change = 1000; // todo: consider a power of ten == AmountFieldEnd - AmountFieldStart + 1
				while(caretColumn-- != AmountFieldStart)
				{
					change /= 10;
				}
				updateDriveAmountUI(change);
			}
			else if((caretColumn >= FrameFieldStart) && (caretColumn <= FrameFieldEnd))
			{
				change = 100;// todo: consider a power of ten == FrameFieldEnd - FrameFieldStart + 1
				while(caretColumn-- != FrameFieldStart)
				{
					change /= 10;
				}
				updateFramesUI(change);
			}
			else if((caretColumn >= DelayFieldStart) && (caretColumn <= DelayFieldEnd))
			{
				// todo: consider a power of ten == DelayFieldEnd - DelayFieldStart + 1
				change = 100;
				while(caretColumn-- != DelayFieldStart)
				{
					change /= 10;
				}
				updateFrameDelayUI(change);
			}
			else if((caretColumn >= RestoreFocusFieldStart) && (caretColumn <= RestoreFocusFieldEnd))
			{
				updateRestoreFocusUI(1);
			}
			break;
		}
        case eLeft:
        {
			advanceCaret(-1);
			break; 
		
        }
        case eRight:
        {
            advanceCaret(1);
			break;
        }
        case eSelect:
        {
            if(eButtonActionPress == msg.m_type)
            {
	            // write any settings to the eprom that have changed.
	            uint16_t savedFocusAmt = eeprom_read_word(&ePromFocusAmount);
	            if(savedFocusAmt != m_driveAmount)
	            {
		            eeprom_write_word(&ePromFocusAmount, m_driveAmount);
	            }
				uint16_t savedFrameDelay = eeprom_read_word(&ePromFrameDelay);
				if(savedFrameDelay != m_frameDelaySeconds)
				{
					eeprom_write_word(&ePromFrameDelay, m_frameDelaySeconds);
				}
	            uint8_t savedNumFrames = eeprom_read_byte(&ePromNumFrames);
	            if(savedNumFrames != m_numFrames)
	            {
		            eeprom_write_byte(&ePromNumFrames, m_numFrames);
	            }
				uint8_t savedRestoreFocus = eeprom_read_byte(&ePromRestoreFocus);
				if(savedRestoreFocus != m_restoreFocus)
				{
					eeprom_write_byte(&ePromRestoreFocus, m_restoreFocus);
				}
	            msg.m_nextHandler = g_pMain;
	            rsp = eSuccess;
            }
            break;
        } // eSelect handler
        default: break;
    }
    return rsp;
}

void SetupHandler::show()
{
	g_print->clear();
	printMenuItem(AmountFieldStart, 0);
	printMenuItem(FrameFieldStart, 1);
	printMenuItem(DelayFieldStart, 2);
	printMenuItem(RestoreFocusFieldStart, 3);
    updateDriveAmountUI(0); // 0: don't change, just show the current value
    updateFramesUI(0);      // 0: don't change, just show the current value
	updateFrameDelayUI(0);  // 0: don't change, just show the current value
	updateRestoreFocusUI(0);
	moveCaret(0, 1);
    showCaret(true);
}
void SetupHandler::updateDriveAmountUI(int change)
{
	m_driveAmount += change;
	if(m_driveAmount < 1 ) m_driveAmount = 1;
    if(m_driveAmount > 9999) m_driveAmount = 9999;
	// set cursor based on how many digits are shown
	unsigned char digits = 0; 
	uint32_t val = m_driveAmount;
	while(val > 0) 
	{
		val /= 10;
		++digits;
	}
    g_print->setCursor(AmountFieldStart, 1);
	for(unsigned char z = 0; z < 4 - digits; z++) g_print->print(F("0"));
    g_print->print(m_driveAmount);
	g_print->setCursor(getCaretCol(), 1);
}
void SetupHandler::updateFramesUI(int change)
{
    m_numFrames += change;
    if(m_numFrames < 1 ) m_numFrames = 1;
    if(m_numFrames > 999) m_numFrames = 999;
	// set cursor based on how many digits are shown
	unsigned char digits = 0;
	uint32_t val = m_numFrames;
	
	while(val > 0)
	{
		val /= 10;
		++digits;
	}
	g_print->setCursor(FrameFieldStart, 1);
    for(unsigned char z = 0; z < 3 - digits; z++) g_print->print(F("0"));
  //  g_print->setCursor(FrameFieldStart + 3 - digits, 1);
    g_print->print(m_numFrames);
	g_print->setCursor(getCaretCol(), 1);
}

void SetupHandler::updateFrameDelayUI(int change)
{
	
	m_frameDelaySeconds += change;
	if(m_frameDelaySeconds < 0 ) m_frameDelaySeconds = 0;
    if(m_frameDelaySeconds > 999) m_frameDelaySeconds = 999;
	unsigned char digits = 0;
	uint32_t val = m_frameDelaySeconds;
	
	while(val > 0)
	{
		val /= 10;
		++digits;
	}
    g_print->setCursor(DelayFieldStart, 1);
	for(unsigned char z = 0; z < 3 - digits; z++) g_print->print(F("0"));
    if(0 != m_frameDelaySeconds) g_print->print(m_frameDelaySeconds);
	g_print->setCursor(getCaretCol(), 1);
}

void SetupHandler::updateRestoreFocusUI(int change)
{
	g_print->setCursor(RestoreFocusFieldStart, 1);
	g_print->print(F("   "));		
	g_print->setCursor(RestoreFocusFieldStart, 1);
	if(0 != change) m_restoreFocus = !m_restoreFocus;
	g_print->print( m_restoreFocus ? F("Yes") : F("No"));
	g_print->setCursor(getCaretCol(), 1);
}
