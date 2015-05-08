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
extern uint16_t g_savedNumFrames;

extern uint16_t EEMEM ePromFocusAmount;
extern uint16_t EEMEM ePromFrameDelay;
extern uint16_t  EEMEM ePromNumFrames;
extern uint8_t  EEMEM ePromRestoreFocus;

extern IMessageHandler *g_pMain;

/*
 *  A table of available apertures on various lenses, at various zoom values & focal distance/ magnifications
 *  The intersection of values in this table play a role in which apertures can be applied
 *  to all lenses generally.
 *
 *   nikkor 16-85
 *   at 16mm
 *                                      3.5,     4, 4.5,     5,  5.6, 6.3, 7.1, 8, 9, 10, 11, 13, 14, 16, 18, 20, 22
 *    at 85 mm
 *                                                               5.6, 6.3, 7.1, 8, 9, 10, 11, 13, 14, 16, 18, 20, 22, 25, 29, 32, 36,
 * 
 * tamron 180mm macro
 *
 * 1:1
 *                                                               5.6, 6.3, 7.1, 8, 9, 10, 11, 13, 14, 16, 18, 20, 22, 25, 29, 32, 36, 40, 45, 51
 *
 * 1:1.2
 *                                                            5, 5.6, 6.3, 7.1, 8, 9, 10, 11, 13, 14, 16, 18, 20, 22, 25, 29, 32, 36, 40, 45,
 * 
 * 1:1.5
 *                                                       4.8, 5, 5.6, 6.3, 7.1, 8, 9, 10, 11, 13, 14, 16, 18, 20, 22, 25, 29, 32, 36, 40, 45
 * 
 * 1:1.75
 *                                                  4.5,      5, 5.6, 6.3, 7.1, 8, 9, 10, 11, 13, 14, 16, 18, 20, 22, 25, 29, 32, 36, 40,
 * 
 * 1:2.25
 *                                               4, 4.5,      5, 5.6, 6.3, 7.1, 8, 9, 10, 11, 13, 14, 16, 18, 20, 22, 25, 29, 32, 36,
 * 
 * 1:3.5
 *                                         3.8,  4, 4.5,      5, 5.6, 6.3, 7.1, 8, 9, 10, 11, 13, 14, 16, 18, 20, 22, 25, 29, 32, 36,
 * 
 * 1: 1.6
 *                                     3.5,      4, 4.5,      5, 5.6, 6.3, 7.1, 8, 9, 10, 11, 13, 14, 16, 18, 20, 22, 25, 29, 32,
 * 
 * tokina 11-16 f2.8
 *                           2.8, 3.2, 3.5,      4, 4.5,      5, 5.6, 6.3, 7.1, 8, 9, 10, 11, 13, 14, 16, 18, 20, 22
 * 
 * nikkor 35mm f1.8 dx
 *         1.8, 2, 2.2, 2.5, 2.8, 3.2, 3.5,      4, 4.5,      5, 5.6, 6.3, 7.1, 8, 9, 10, 11, 13, 14, 16, 18, 20, 22
*/

// The value expressed to SetDevicePropValue(PTP_DPC_FNumber)
// is a value from the intersection of the available values,
// multiplied by 100


uint16_t SetupHandler::m_apertures[maxApertures] = 
{
    560, 630, 710, 800, 900, 1000, 1100, 1300, 1400, 1600, 1800, 2000, 2200
};
// Note, to save lcd space, the displayed values are integer divided by 100
// & therefore  05, 06, 07, 08, 09, 10, 11, 13, 14, 16, 18, 20, 22 

SetupHandler::SetupHandler(MessagePump *_pump, uint32_t driveAmount, uint32_t frames)
    :
    IMessageHandler(_pump),
	m_driveAmount(driveAmount, 1, 9999, 4),
	m_numFrames(frames, 1, 999, 3),
	m_frameDelaySeconds(0, 0, 99, 2),
    m_lastFStop(16, 5, 22, 2),
	m_restoreFocus(false),
    m_lastFrameFstopInx(9) // f16
{
    menu[0] = "Amt";
    menu[1] = "Frm";
	menu[2] = "Dl";
    menu[3] = "Lf";
	menu[4] = "R";
};
SetupHandler::~SetupHandler()
{}

void SetupHandler::advanceCaret(uint8_t dir) // -1 = left, 1 right. All other values ignored
{
	unsigned char caretColumn = g_print->getCaretCol();
	if(dir == 0xff)
	{
        // move left
		switch(caretColumn)
		{
			case AmountFieldStartCol:		{ caretColumn = RestoreFocusFieldStartCol; break;}
			case FrameFieldStartCol:		{ caretColumn = AmountFieldEndCol; break;}
			case DelayFieldStartCol:		{ caretColumn = FrameFieldEndCol; break;}
            case LastFstopFieldStartCol:    { caretColumn = DelayFieldEndCol; break; }                
			case RestoreFocusFieldStartCol:	{ caretColumn = LastFstopFieldStartCol; break;}
			default:						{ --caretColumn; break;}				
		}
	}
	else if(dir == 1)
	{
        // move right
		switch(caretColumn)
		{
			case AmountFieldEndCol:			{ caretColumn = FrameFieldStartCol; break;}
			case FrameFieldEndCol:			{ caretColumn = DelayFieldStartCol; break;}
			case DelayFieldEndCol:			{ caretColumn = LastFstopFieldStartCol; break;}
            // no movement within the last fstop field (up down keys select preset values).
            case LastFstopFieldStartCol:    { caretColumn = RestoreFocusFieldStartCol; break;}                
            // no movement within the last fstop field (up down keys select preset values).                
			case RestoreFocusFieldStartCol:	{ caretColumn = AmountFieldStartCol; break;}
			default:						{ ++caretColumn; break;}
		}
	}
	g_print->setCursor(caretColumn, 1);
}

MsgResp SetupHandler::processMessage(Msg& msg)
{
    MsgResp rsp = eFail;
	if(eButtonActionPress != msg.m_type) return rsp;
	// the amount up or down some parameter will be changed
	unsigned char caretColumn = g_print->getCaretCol();
	int change = 0;
    // which button?
    switch(msg.m_code)
    {
        case eDown:
        {
			if((caretColumn <= AmountFieldEndCol) && (msg.m_type == eButtonActionPress))
			{
				// change amount entry
				change = -1000; // todo: consider a power of ten == AmountFieldEnd - AmountFieldStart + 1
				while(caretColumn-- != AmountFieldStartCol)
				{
					change /= 10;
				}
				m_driveAmount.changeVal(change);
				m_driveAmount.display(AmountFieldStartCol, AmountFieldRow);
			}
			else if((caretColumn >= FrameFieldStartCol) && (caretColumn <= FrameFieldEndCol))
			{
				change = -100;// todo: consider a power of ten == FrameFieldEnd - FrameFieldStart + 1
				while(caretColumn-- != FrameFieldStartCol)
				{
					change /= 10;
				}
				m_numFrames.changeVal(change);
				m_numFrames.display(FrameFieldStartCol, FrameFieldRow);
			}
			else if((caretColumn >= DelayFieldStartCol) && (caretColumn <= DelayFieldEndCol))
			{
				// todo: consider a power of ten == DelayFieldEnd - DelayFieldStart + 1
				change = -10;
				while(caretColumn-- != DelayFieldStartCol)
				{
					change /= 10;
				}
				m_frameDelaySeconds.changeVal(change);
				m_frameDelaySeconds.display(DelayFieldStartCol, DelayFieldRow);
			}
            else if(caretColumn == LastFstopFieldStartCol)
            {
                // wrap to top at 0
                if(m_lastFrameFstopInx == 0) (m_lastFrameFstopInx = maxApertures - 1);
                else --m_lastFrameFstopInx;
                m_lastFStop.setVal(m_apertures[m_lastFrameFstopInx] / 100);
                m_lastFStop.display(LastFstopFieldStartCol,LastFstopFieldRow );
            }                
			else if((caretColumn == RestoreFocusFieldStartCol))
			{
				updateRestoreFocusUI(-1);
			}
            break;
        }
        case eUp:
        {
			if((caretColumn <= AmountFieldEndCol) && (msg.m_type == eButtonActionPress))
			{
				// change amount entry
				change = 1000; // todo: consider a power of ten == AmountFieldEnd - AmountFieldStart + 1
				while(caretColumn-- != AmountFieldStartCol)
				{
					change /= 10;
				}
				m_driveAmount.changeVal(change);
				m_driveAmount.display(AmountFieldStartCol, AmountFieldRow);
			}
			else if((caretColumn >= FrameFieldStartCol) && (caretColumn <= FrameFieldEndCol))
			{
				change = 100;// todo: consider a power of ten == FrameFieldEnd - FrameFieldStart + 1
				while(caretColumn-- != FrameFieldStartCol)
				{
					change /= 10;
				}
				m_numFrames.changeVal(change);
				m_numFrames.display(FrameFieldStartCol, FrameFieldRow);
			}
			else if((caretColumn >= DelayFieldStartCol) && (caretColumn <= DelayFieldEndCol))
			{
				// todo: consider a power of ten == DelayFieldEnd - DelayFieldStart + 1
				change = 10;
				while(caretColumn-- != DelayFieldStartCol)
				{
					change /= 10;
				}
				m_frameDelaySeconds.changeVal(change);
				m_frameDelaySeconds.display(DelayFieldStartCol, DelayFieldRow);
			}
            else if(caretColumn == LastFstopFieldStartCol)
            {
                // wrap to 0 at top
                if(m_lastFrameFstopInx < maxApertures - 1)
                {
                    ++m_lastFrameFstopInx;
                }
                else
                {
                    m_lastFrameFstopInx = 0;
                }
                m_lastFStop.setVal(m_apertures[m_lastFrameFstopInx] / 100);
                m_lastFStop.display(LastFstopFieldStartCol,LastFstopFieldRow );
            }                
			else if(caretColumn == RestoreFocusFieldStartCol)
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
	            // write any settings that have changed to the eeprom.
	            uint16_t savedFocusAmt = eeprom_read_word(&ePromFocusAmount);
	            if(savedFocusAmt != m_driveAmount.getVal())
	            {
		            eeprom_write_word(&ePromFocusAmount, m_driveAmount.getVal());
	            }
				uint16_t savedFrameDelay = eeprom_read_word(&ePromFrameDelay);
				if(savedFrameDelay != m_frameDelaySeconds.getVal())
				{
					eeprom_write_word(&ePromFrameDelay, m_frameDelaySeconds.getVal());
				}
	            uint16_t savedNumFrames = eeprom_read_word(&ePromNumFrames);
	            if(savedNumFrames != m_numFrames.getVal())
	            {
		            eeprom_write_word(&ePromNumFrames, m_numFrames.getVal());
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
	printMenuItem(AmountFieldStartCol, 0/*row*/, 0);
	printMenuItem(FrameFieldStartCol, 0/*row*/,1);
	printMenuItem(DelayFieldStartCol, 0/*row*/,2);
    printMenuItem(LastFstopFieldStartCol, 0/*row*/, 3);
	printMenuItem(RestoreFocusFieldStartCol, 0/*row*/,4);
    m_driveAmount.display(AmountFieldStartCol, AmountFieldRow);
	m_numFrames.display(FrameFieldStartCol, FrameFieldRow);
    m_frameDelaySeconds.display(DelayFieldStartCol, DelayFieldRow);
    m_lastFStop.display(LastFstopFieldStartCol, LastFstopFieldRow);
	updateRestoreFocusUI(0);
	g_print->setCursor(0, 1);
}

void SetupHandler::updateRestoreFocusUI(int change)
{
	g_print->saveCursorLocation();
	g_print->setCursor(RestoreFocusFieldStartCol, 1);
	//g_print->print(F("   "));		
	//g_print->setCursor(RestoreFocusFieldStart, 1);
	if(0 != change) m_restoreFocus = !m_restoreFocus;
	g_print->print( m_restoreFocus ? F("Y") : F("N"));
	g_print->restoreCursorLocation();
}
