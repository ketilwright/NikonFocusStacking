/* 
* ModelMenu.cpp
*
* Created: 6/24/2014 8:48:23 AM
* Author: Ketil Wright
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


#include "ManualHandler.h"
#include "LcdImpl.h"
#include "NikType003.h"
#include "SetupHandler.h"

#define MarkStartCol 0
#define MarkStartRow 0
#define MarkEndCol 8
#define MarkEndRow 0
#define AmountCol 0
#define AmountRow 1
#define AmountFieldStart 4
#define AmountFieldEnd 7

extern IMessageHandler *g_pMain;
extern SetupHandler *g_pSetup;
extern NikType003 nk3;
// default constructor
ManualHandler::ManualHandler(MessagePump *_pump)
	:
	IMessageHandler(_pump)
{
	menu[0] = "Start";
	menu[1] = "End";
	menu[2] = "Amt";
} 

MsgResp ManualHandler::processMessage(Msg& msg)
{
	MsgResp rsp = eSuccess;
	if(eButtonActionPress != msg.m_type) return rsp;
	
	switch(msg.m_code)
	{
		case eLeft:
		{
			advanceCaret(0xff);
			break;
		}
		case eRight:
		{
			advanceCaret(1);
			break;
		}
		case eDown:
		{
			// advance focus backward
			if(PTP_RC_OK == nk3.enableLiveView(true))
			{
				if(PTP_RC_OK == nk3.waitForReady(1000))
				{
					if(PTP_RC_OK == nk3.moveFocus(1, g_pSetup->getDriveAmount()))
					{
						if(PTP_RC_OK == nk3.waitForReady(1000))
						{
							nk3.enableLiveView(false);
						}
					}
				}
			}
			break;
		}
		case eUp:
		{
			// advance focus forward
			if(PTP_RC_OK == nk3.enableLiveView(true))
			{
				if(PTP_RC_OK == nk3.waitForReady(1000))
				{
					if(PTP_RC_OK == nk3.moveFocus(2, g_pSetup->getDriveAmount()))
					{
						if(PTP_RC_OK == nk3.waitForReady(1000))
						{
							nk3.enableLiveView(false);
						}
					}
				}
			}
			break; 
		}
		case eSelect:
		{
			msg.m_nextHandler = g_pMain;
			break;
		}
		default:
		{
			rsp = eFail;
			break;
		}
	}
	return rsp;
	
}

void ManualHandler::advanceCaret(uint8_t dir)
{
	if(0xff == dir)
	{
		// left
		if(getCaretCol() == MarkStartCol && getCaretRow() == MarkStartRow)
		{
			// currently on mark start, move to end of amount field
			moveCaret(AmountCol, AmountFieldEnd);
		}
		else if(getCaretCol() == MarkEndCol && getCaretRow() == MarkEndRow)
		{
			// currently on mark end, move to mark start field
			moveCaret(MarkStartCol, MarkStartRow);
		}
		else if(getCaretCol() == AmountFieldStart && getCaretRow() == AmountRow)
		{
			// currently at start of amount field, move to mark end field
			moveCaret(MarkEndCol, MarkEndRow);
		}
		else if(getCaretRow() == AmountRow && getCaretCol() > AmountFieldStart && getCaretCol() <= AmountFieldEnd)
		{
			// move caret within amount field
			moveCaret(getCaretCol() - 1, AmountRow);
		}
	}
	else if(1 == dir)
	{
		// right
		if(getCaretCol() == MarkStartCol && getCaretRow() == MarkStartRow)
		{
			// currently on mark start, move to mark end field
			moveCaret(MarkEndCol, MarkEndRow);
		}
		else if(getCaretCol() == MarkEndCol && getCaretRow() == MarkEndRow)
		{
			// currently on mark end, move to start of amount field
			moveCaret(AmountFieldStart, AmountRow);
		}
		else if(getCaretCol() == AmountFieldEnd && getCaretRow() == AmountRow)
		{
			// currently at the end of the amount field, move to the
			// mark start field
			moveCaret(MarkStartCol, MarkStartRow);
		}
		else if(getCaretRow() == AmountRow && getCaretCol() >= AmountFieldStart && getCaretCol() < AmountFieldEnd)
		{
			// move caret within the amount field
			moveCaret(getCaretCol() + 1, AmountRow);
		}
	}
}

void ManualHandler::show()
{
	g_print->clear();
	printMenuItem(MarkStartCol, MarkStartRow, 0);
	printMenuItem(MarkEndCol, MarkEndRow, 1);
	printMenuItem(AmountCol, AmountRow, 2);
	
	moveCaret(MarkStartCol, MarkStartRow);
	showCaret(true);
	
}