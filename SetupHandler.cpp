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


SetupHandler::SetupHandler(MessagePump *_pump, uint32_t driveAmount, uint32_t frames)
    :
    IMessageHandler(_pump),
	m_driveAmount(driveAmount, 1, 9999, 4),
	m_numFrames(frames, 1, 999, 3),
	m_frameDelaySeconds(0, 0, 999, 3),
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
	unsigned char caretColumn = g_print->getCaretCol();
	if(dir == 0xff)
	{
		switch(caretColumn)
		{
			case AmountFieldStartCol:		{ caretColumn = RestoreFocusFieldEnd; break;}
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
			case RestoreFocusFieldEnd:		{ caretColumn = AmountFieldStartCol; break;}
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
	unsigned char caretColumn = g_print->getCaretCol();
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
				while(caretColumn-- != AmountFieldStartCol)
				{
					change /= 10;
				}
				m_driveAmount.changeVal(change);
				m_driveAmount.display(AmountFieldStartCol, AmountFieldRow);
			}
			else if((caretColumn >= FrameFieldStart) && (caretColumn <= FrameFieldEnd))
			{
				change = -100;// todo: consider a power of ten == FrameFieldEnd - FrameFieldStart + 1
				while(caretColumn-- != FrameFieldStart)
				{
					change /= 10;
				}
				m_numFrames.changeVal(change);
				m_numFrames.display(FrameFieldStart, FrameFieldRow);
			}
			else if((caretColumn >= DelayFieldStart) && (caretColumn <= DelayFieldEnd))
			{
				// todo: consider a power of ten == DelayFieldEnd - DelayFieldStart + 1
				change = -100;
				while(caretColumn-- != DelayFieldStart)
				{
					change /= 10;
				}
				m_frameDelaySeconds.changeVal(change);
				m_frameDelaySeconds.display(DelayFieldStart, DelayFieldRow);
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
				while(caretColumn-- != AmountFieldStartCol)
				{
					change /= 10;
				}
				m_driveAmount.changeVal(change);
				m_driveAmount.display(AmountFieldStartCol, AmountFieldRow);
			}
			else if((caretColumn >= FrameFieldStart) && (caretColumn <= FrameFieldEnd))
			{
				change = 100;// todo: consider a power of ten == FrameFieldEnd - FrameFieldStart + 1
				while(caretColumn-- != FrameFieldStart)
				{
					change /= 10;
				}
				m_numFrames.changeVal(change);
				m_numFrames.display(FrameFieldStart, FrameFieldRow);
			}
			else if((caretColumn >= DelayFieldStart) && (caretColumn <= DelayFieldEnd))
			{
				// todo: consider a power of ten == DelayFieldEnd - DelayFieldStart + 1
				change = 100;
				while(caretColumn-- != DelayFieldStart)
				{
					change /= 10;
				}
				m_frameDelaySeconds.changeVal(change);
				m_frameDelaySeconds.display(DelayFieldStart, DelayFieldRow);
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
	printMenuItem(FrameFieldStart, 0/*row*/,1);
	printMenuItem(DelayFieldStart, 0/*row*/,2);
	printMenuItem(RestoreFocusFieldStart, 0/*row*/,3);
    m_driveAmount.display(AmountFieldStartCol, AmountFieldRow);
	m_numFrames.display(FrameFieldStart, FrameFieldRow);
    m_frameDelaySeconds.display(DelayFieldStart, DelayFieldRow);
	updateRestoreFocusUI(0);
	moveCaret(0, 1);
}

void SetupHandler::updateRestoreFocusUI(int change)
{
	g_print->saveCursorLocation();
	g_print->setCursor(RestoreFocusFieldStart, 1);
	g_print->print(F("   "));		
	g_print->setCursor(RestoreFocusFieldStart, 1);
	if(0 != change) m_restoreFocus = !m_restoreFocus;
	g_print->print( m_restoreFocus ? F("Yes") : F("No"));
	g_print->restoreCursorLocation();
}
