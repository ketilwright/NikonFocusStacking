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

// amount menu indicator in row 0
#define AmountMenuItemCol 0
#define AmountMenuItemRow 0
// amount field in row 1
#define AmountFieldStartCol 0
#define AmountFieldEndCol 3
#define AmountFieldRow 1

#define MarkStartMenuItemCol 5
#define MarkStartMenuItemRow 0

#define MarkEndMenuItemCol 13
#define MarkEndMenuItemRow 0

#define TestMenuItemCol 5
#define TestMenuItemRow 1

#define DoneMenuItemCol 12
#define DoneMenuItemRow 1

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
	menu[3] = "Test";
	menu[4] = "Done";
} 

MsgResp ManualHandler::processMessage(Msg& msg)
{
	if(eButtonActionPress != msg.m_type) return eFail;
	MsgResp rsp = eSuccess;
	uint8_t col = g_print->getCaretCol();
	uint8_t row = g_print->getCaretRow();
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
			if((TestMenuItemCol == col) && (TestMenuItemRow == row))
			{
				focus(1); // backward	
			}
			else if((col <= AmountFieldEndCol) && (AmountFieldRow == row))
			{
				rsp = g_pSetup->processMessage(msg);
			}
			break;
		}
		case eUp:
		{
			if((TestMenuItemCol == col) && (TestMenuItemRow == row))
			{
				focus(2); // forward
			}
			else if((col <= AmountFieldEndCol) && (AmountFieldRow == row))
			{
				rsp = g_pSetup->processMessage(msg);
			}
			break; 
		}
		case eSelect:
		{
			if((MarkStartMenuItemCol == col) && (MarkStartMenuItemRow == row))
			{
				// mark start
			}
			else if((MarkEndMenuItemCol == col) && (MarkEndMenuItemRow == row))
			{
				// mark end
			}
			else if((DoneMenuItemCol == col) && (DoneMenuItemRow == row))
			{
				msg.m_nextHandler = g_pMain;
			}
			
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
	uint8_t col = g_print->getCaretCol();
	uint8_t row = g_print->getCaretRow();
	if(0xff == dir) // left
	{
		if((MarkStartMenuItemCol == col) && (MarkStartMenuItemRow == row))
		{
			// move from Start -> Done
			moveCaret(DoneMenuItemCol, DoneMenuItemRow);
		}
		else if((MarkEndMenuItemCol == col) && (MarkEndMenuItemRow == row))
		{
			// move from End -> Start
			moveCaret(MarkStartMenuItemCol, MarkStartMenuItemRow);
		}
		else if((AmountFieldStartCol == col) && (AmountFieldRow == row))
		{
			// move from beginning of amount field -> End
			moveCaret(MarkEndMenuItemCol, MarkEndMenuItemRow);
		}
		else if((col > AmountFieldStartCol) && (col <= AmountFieldEndCol) && (AmountFieldRow == row))
		{
			// move within the amount field
			moveCaret(col - 1, row);
		}
		else if((TestMenuItemCol == col) && (TestMenuItemRow == row))
		{
			// move from Test menu to end of amount field
			moveCaret(AmountFieldEndCol, AmountFieldRow);
		}
		else if((DoneMenuItemCol == col) && (DoneMenuItemRow == row))
		{
			// move from Done item -> Test item
			moveCaret(TestMenuItemCol, TestMenuItemRow);
		}
		
	}
	else if(1 == dir) // right
	{
		if((MarkStartMenuItemCol == col) && (MarkStartMenuItemRow == row))
		{
			// move from mark start -> mark end
			moveCaret(MarkEndMenuItemCol, MarkEndMenuItemRow);
		}
		else if((MarkEndMenuItemCol == col) && (MarkEndMenuItemRow == row))
		{
			// move from mark end -> amount field
			moveCaret(AmountFieldStartCol, AmountFieldRow);
		}
		else if((col >= AmountFieldStartCol) && (col < AmountFieldEndCol) && (AmountFieldRow == row))
		{
			// move within the amount field
			moveCaret(col + 1, AmountFieldRow);
		}
		else if((AmountFieldEndCol == col) && (AmountFieldRow == row))
		{
			// move from end of amount field to done
			moveCaret(TestMenuItemCol, TestMenuItemRow);
		}
		else if((TestMenuItemCol == col) && (TestMenuItemRow == row))
		{
			// move from test -> donw
			moveCaret(DoneMenuItemCol, DoneMenuItemRow);
		}
		else if((DoneMenuItemCol == col) && (DoneMenuItemRow == row))
		{
			// move from Done -> Mark Start
			moveCaret(MarkStartMenuItemCol, MarkStartMenuItemRow);
		}
	}
}

void ManualHandler::show()
{
	g_print->clear();
	printMenuItem(MarkStartMenuItemCol, MarkStartMenuItemRow, 0);
	printMenuItem(MarkEndMenuItemCol, MarkEndMenuItemRow, 1);
	printMenuItem(AmountMenuItemCol, AmountMenuItemRow, 2);
	printMenuItem(TestMenuItemCol, TestMenuItemRow, 3);
	printMenuItem(DoneMenuItemCol, DoneMenuItemRow, 4);
	g_pSetup->m_driveAmount.display(AmountFieldStartCol, AmountFieldRow);
	moveCaret(MarkStartMenuItemCol, MarkStartMenuItemRow);
}
void ManualHandler::focus(uint8_t dir)
{
	if(PTP_RC_OK == nk3.enableLiveView(true))
	{
		if(PTP_RC_OK == nk3.waitForReady(1000))
		{
			if(PTP_RC_OK == nk3.moveFocus(dir, g_pSetup->getDriveAmount()))
			{
				if(PTP_RC_OK == nk3.waitForReady(1000))
				{
					nk3.enableLiveView(false);
				}
			}
		}
	}
}