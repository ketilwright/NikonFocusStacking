/*
 * MainMenu.cpp
 *
 * Created: 4/30/2014 9:50:19 AM
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

#include "MessageHandler.h"
#include "MainMenu.h"
#include "SetupHandler.h"
#include "RunFocusStackHandler.h"

#include "NikType003.h"
#include "LcdImpl.h"
#include "Sketch.h"
extern RunFocusStackHandler *g_pRunStack;
extern SetupHandler *g_pSetup;
extern NikType003 nk3;

#define SetupMenuPos 0
#define RunMenuPos 8

// Main menu ctor. Sets up the text strings
// to display.
MainMenuHandler::MainMenuHandler(MessagePump *_pump)
    :
    IMessageHandler(_pump)
{
    menu[0] = "Setup";
    menu[1] = "RunStack";
};
MainMenuHandler::~MainMenuHandler()
{}

// Listens for button press Msgs dispatched from loop().
MsgResp MainMenuHandler::processMessage(Msg& msg)
{
    MsgResp rsp = eFail;
    if(eButtonActionPress == msg.m_type)
    {
        // up and down buttons move the caret.
        // select button invokes Setup or runStack
        // handling.
        switch(msg.m_code)
        {
            case eLeft:     
			{ 
				switch(getCaretCol())
				{
					case SetupMenuPos:
					{
						if(nk3.isConnected()) moveCaret(RunMenuPos, 0);
						break;
					}
					case RunMenuPos:
					{
						moveCaret(SetupMenuPos, 0);
						break;
					}
					default:break;
				}
				break;
			}
            case eRight:
			{
				switch(getCaretCol())
				{
					case SetupMenuPos: 
					{
						if(nk3.isConnected()) moveCaret(RunMenuPos, 0);
						break;
					}
					case RunMenuPos:
					{
						moveCaret(SetupMenuPos, 0);
						break;
					}
					default:break;
				}
				break;
			}
            case eSelect:
            {
                switch(m_caretCol)
                {
                    case 0:
                    {
                        msg.m_nextHandler = g_pSetup;
                        rsp = eSuccess;
                        break;
                    }
                    case 8:
                    {
                        if(nk3.isConnected())
                        {
                            msg.m_nextHandler = g_pRunStack;
                            rsp = eSuccess;
                        }
                        break;
                    }
                    case 3:
                    {
                        // on/off?
                        // TODO: turn display off/on
                        // how far to sleep can we go
                        // and still wake?
                        break;
                    }
                    default: { break; }
                }
                break;
            }
			case eUp:     { break; }
			case eDown:    { break; }
            default : { break; }
        }
    }
    return rsp;
}
// Show the main menu, the camera type & connection state.
extern bool g_usbOK;
void MainMenuHandler::show()
{
	if(!g_usbOK) return;
    IMessageHandler::show();
	moveCaret(0, 0);
	showCaret(true);
}





