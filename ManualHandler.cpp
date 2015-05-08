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

// "Pos"
#define FocusPosMenuItemCol 5
#define FocusPosMenuItemRow 0
// current value of m_pos
#define FocusPosIndicatorCol 9
#define FocusPosIndicatorRow 0

// toggles on/off live view, when
// lv is on up/down keys move focus
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
	IMessageHandler(_pump),
    //m_pos(0, -2147483648, 2147483647, 7)
    m_pos(0, -1000000, 1000000, 7, false) // false -> don't pad zeros
{
	menu[0] = "Amt";
    menu[1] = "Pos";
    menu[2] = "Test";
    menu[3] = "Done";
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
                // our amt field is in the same screen pos the setup
                // handler, which already knows how to evaluate the 
                // msg.
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
                // our amt field is in the same screen pos the setup
                // handler, which already knows how to evaluate the
                // msg.
				rsp = g_pSetup->processMessage(msg);
			}
			break; 
		}
		case eSelect:
		{
			if((TestMenuItemCol == col) && (TestMenuItemRow == row))
			{
				if(PTP_RC_OK == nk3.waitForReady(100))
				{
					nk3.enableLiveView(!nk3.isLiveViewEnabled());
				}
			}
			else if((DoneMenuItemCol == col) && (DoneMenuItemRow == row))
			{
				if(nk3.isLiveViewEnabled())
				{
					nk3.enableLiveView(false);
				}
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
		if((AmountFieldStartCol == col) && (AmountFieldRow == row))
		{
			// wrap around backwards from beginning of amount field -> Done menu ite,
			g_print->setCursor(DoneMenuItemCol, DoneMenuItemRow);
		}
		else if((col > AmountFieldStartCol) && (col <= AmountFieldEndCol) && (AmountFieldRow == row))
		{
			// move within the amount field
			g_print->setCursor(col - 1, row);
		}
		else if((TestMenuItemCol == col) && (TestMenuItemRow == row))
		{
			// move from Test menu to end of amount field
			g_print->setCursor(AmountFieldEndCol, AmountFieldRow);
		}
		else if((DoneMenuItemCol == col) && (DoneMenuItemRow == row))
		{
			// move from Done item -> Test item
			g_print->setCursor(TestMenuItemCol, TestMenuItemRow);
		}
		
	}
	else if(1 == dir) // right
	{
		if((col >= AmountFieldStartCol) && (col < AmountFieldEndCol) && (AmountFieldRow == row))
		{
			// move within the amount field
			g_print->setCursor(col + 1, AmountFieldRow);
		}
		else if((AmountFieldEndCol == col) && (AmountFieldRow == row))
		{
			// move from end of amount field to done
			g_print->setCursor(TestMenuItemCol, TestMenuItemRow);
		}
		else if((TestMenuItemCol == col) && (TestMenuItemRow == row))
		{
			// move from test -> done
			g_print->setCursor(DoneMenuItemCol, DoneMenuItemRow);
		}
		else if((DoneMenuItemCol == col) && (DoneMenuItemRow == row))
		{
			// wrap around from Done -> to start of amount field
			g_print->setCursor(AmountFieldStartCol, AmountFieldRow);
		}
	}
}

void ManualHandler::show()
{
	g_print->clear();
	printMenuItem(AmountMenuItemCol, AmountMenuItemRow, 0);
    printMenuItem(FocusPosMenuItemCol, FocusPosMenuItemRow,1);
	printMenuItem(TestMenuItemCol, TestMenuItemRow, 2);
	printMenuItem(DoneMenuItemCol, DoneMenuItemRow, 3);
    m_pos.setVal(0);
    m_pos.display(FocusPosIndicatorCol, FocusPosIndicatorRow);
	g_pSetup->m_driveAmount.display(AmountFieldStartCol, AmountFieldRow);
	g_print->setCursor(AmountFieldStartCol, AmountFieldRow);
    nk3.waitForReady(1000);
}
void ManualHandler::focus(uint8_t dir)
{
    if(nk3.isLiveViewEnabled())
    {
        if(PTP_RC_OK == nk3.waitForReady(100))
        {
            int32_t amount = static_cast<int32_t>(g_pSetup->getDriveAmount());
            uint16_t retVal = nk3.moveFocus(dir, amount);
            switch(retVal)
            {
                case PTP_RC_OK:
                {
                    switch(dir)
                    {
                        case 1:
                        {
                            m_pos.changeVal(-amount);
                            break;
                        }
                        case 2:
                        {
                            m_pos.changeVal(amount);
                            break;                            
                        }
                    }
                    
                    break;
                }
                case NK_RC_InvalidStatus:
                {
                    m_pos.setVal(0);
                    m_pos.display(FocusPosIndicatorCol, FocusPosIndicatorRow);
                    g_print->saveCursorLocation();
                    g_print->setCursor(FocusPosIndicatorCol, FocusPosIndicatorRow);
                    g_print->print("chk cam");
                    g_print->restoreCursorLocation();
                    delay(1000);
                    break;
                }
                case NK_RC_MfDriveStepEnd:
                {
                    // TODO: flag this condition & refuse any further
                    // focus drive in the same direction.
                    // Note that a subsequent successful 
                    // call to nk3.MoveFocus is returning
                    // NK_RC_MfDriveStepInsufficiency (you can hear
                    // the focus motor move).
                    m_pos.setVal(0);
                    m_pos.display(FocusPosIndicatorCol, FocusPosIndicatorRow);
                    g_print->saveCursorLocation();
                    g_print->setCursor(FocusPosIndicatorCol, FocusPosIndicatorRow);
                    g_print->print("end");
                    g_print->restoreCursorLocation();
                    break;
                }
                case NK_RC_MfDriveStepInsufficiency:
                {
                     m_pos.setVal(0);
                     m_pos.display(FocusPosIndicatorCol, FocusPosIndicatorRow);
                     g_print->saveCursorLocation();
                     g_print->setCursor(FocusPosIndicatorCol, FocusPosIndicatorRow);
                     g_print->print("isf");
                     g_print->restoreCursorLocation();
                    break;
                }
                
            }
            m_pos.display(FocusPosIndicatorCol, FocusPosIndicatorRow);
            
        }            
    }
}
#if 0
void ManualHandler::focus(uint8_t dir)
{
    if(nk3.isLiveViewEnabled())
    {
        if(PTP_RC_OK == nk3.waitForReady(100))
        {
            int32_t amount = static_cast<int32_t>(g_pSetup->getDriveAmount());
            if(PTP_RC_OK == nk3.moveFocus(dir, g_pSetup->getDriveAmount()))
            {
                //Serial.print("moveFocus: direction: "); 
                //Serial.print(dir); 
                //Serial.print(" amount "); 
                //Serial.println(amount); 
                //
                bool proceed = false;
              //  bool updateDisplay = false;
                uint16_t devReadyResp = PTP_RC_DeviceBusy;
                while(!proceed)
                {
                    devReadyResp = nk3.Operation(NK_OC_DeviceReady, 0, NULL);
                    switch(devReadyResp)
                    {
                        case PTP_RC_OK:
                        {
                            //Serial.println("PTP_RC_OK"); 
                            proceed = true;
                //            updateDisplay = true;
                            break;
                        }
                        case NK_RC_MfDriveStepEnd:
                        {
                            //Serial.println("NK_RC_MfDriveStepEnd"); 
                            break;    
                        }
                        case PTP_RC_DeviceBusy:
                        {
                            //Serial.println("PTP_RC_DeviceBusy"); 
                            delay(100);
                            break;
                        }
                        case NK_RC_MfDriveStepInsufficiency:
                        {
                            //Serial.println("NK_RC_MfDriveStepInsufficiency"); 
                            if((nk3.enableLiveView(false) == PTP_RC_OK))
                            {
                                if((devReadyResp = nk3.waitForReady(1000)) == PTP_RC_OK)
                                {
                                    if((devReadyResp = nk3.enableLiveView(true)) == PTP_RC_OK)
                                    {
                                        if((devReadyResp = nk3.waitForReady(1000)) == PTP_RC_OK)
                                        {
                                            m_pos.setVal(0);
                                            m_pos.display(FocusPosIndicatorCol, FocusPosIndicatorRow);
                                            return;
                                        } //ready
                                    }  // moveFocus
                                } // ready
                            }
                            proceed = true;
                            break;
                        }
                        
                        
                    }
                }
                
                //Serial.println("MFDrive done"); 
                switch(devReadyResp)
                {
                    case PTP_RC_OK:
                    {
                        switch(dir)
                        {
                            case 1: // backward
                            {
                                m_pos.changeVal(-amount);
                                break;
                            }
                            case 2: // forward
                            {
                                m_pos.changeVal(amount);
                                break;
                            }
                        }
                        m_pos.display(FocusPosIndicatorCol, FocusPosIndicatorRow);
                        break;
                    }
                    case NK_RC_MfDriveStepInsufficiency:
                    {
                        // Indicates that the driving amount is insufficient
                        m_pos.setVal(0);
                        m_pos.display(FocusPosIndicatorCol, FocusPosIndicatorRow);
                        g_print->saveCursorLocation();
                        g_print->setCursor(FocusPosIndicatorCol, FocusPosIndicatorRow);
                        g_print->print("isf");
                        g_print->restoreCursorLocation();
                        
                        
                        break;
                    }
                }
                //nk3.waitForReady(100);
            }
        }
    }
}

void ManualHandler::focus(uint8_t dir)
{
	if(nk3.isLiveViewEnabled())
	{
		if(PTP_RC_OK == nk3.waitForReady(100))
		{
            int32_t amount = static_cast<int32_t>(g_pSetup->getDriveAmount());
			if(PTP_RC_OK == nk3.moveFocus(dir, g_pSetup->getDriveAmount()))
			{
                uint16_t retDevReady = nk3.Operation(NK_OC_DeviceReady, 0, NULL);
                switch(retDevReady)
                {
                    case PTP_RC_OK:
                    {
                        switch(dir)
                        {
                            case 1: // backward
                            {
                                m_pos.changeVal(-amount);
                                break;
                            }
                            case 2: // forward
                            {
                                m_pos.changeVal(amount);
                                break;
                            }
                        }
                        m_pos.display(FocusPosIndicatorCol, FocusPosIndicatorRow);
                        break;
                    }
                    case NK_RC_MfDriveStepEnd:
                    {
                        // Indicates that the MF driving reaches the termination.
                        g_print->saveCursorLocation();
                        g_print->setCursor(FocusPosIndicatorCol, FocusPosIndicatorRow);
                        g_print->print("end");
                        g_print->restoreCursorLocation();
                        break;
                    }
                    case NK_RC_MfDriveStepInsufficiency:
                    {
                        // Indicates that the driving amount is insufficient
                        g_print->saveCursorLocation();
                        g_print->setCursor(FocusPosIndicatorCol, FocusPosIndicatorRow);
                        g_print->print("insuf");
                        g_print->restoreCursorLocation();
                        break;
                    }
                }                
				//nk3.waitForReady(100);
			}	
		}
	}
}
#endif