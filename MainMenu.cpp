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
#include "ManualHandler.h"
#include "RunFocusStackHandler.h"

#include "NikType003.h"
#include "LcdImpl.h"
#include "Sketch.h"
extern RunFocusStackHandler *g_pRunStack;
extern SetupHandler			*g_pSetup;
extern ManualHandler		*g_pManual;
extern NikType003 nk3;

#define SetupMenuPosCol 0
#define SetupMenuPosRow 0
#define ModelMenuPosCol 8
#define ModelMenuPosRow 0
#define RunMenuPosCol 0
#define RunMenuPosRow 1

// Main menu ctor. Sets up the text strings
// to display.
MainMenuHandler::MainMenuHandler(MessagePump *_pump)
    :
    IMessageHandler(_pump)
{
    menu[0] = "Setup";
	menu[1] = "Test";
    menu[2] = "RunStack";
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
				advanceCaret(0xff);
				rsp = eSuccess;
				break;
			}
            case eRight:
			{
				advanceCaret(1);
				rsp = eSuccess;
				break;
			}
            case eSelect:
            {
				if(g_print->getCaretCol() == SetupMenuPosCol && g_print->getCaretRow() == SetupMenuPosRow)
				{
					msg.m_nextHandler = g_pSetup;
					rsp = eSuccess;
				}
				else if(g_print->getCaretCol() == ModelMenuPosCol && g_print->getCaretRow() == ModelMenuPosRow)
				{
					msg.m_nextHandler = g_pManual;
					rsp = eSuccess;
				}
				else if(g_print->getCaretCol() == RunMenuPosCol && g_print->getCaretRow() == RunMenuPosRow)
				{
					msg.m_nextHandler = g_pRunStack;
					rsp = eSuccess;
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
	g_print->clear();
	g_print->setCursor(SetupMenuPosCol, SetupMenuPosRow);
	g_print->print(menu[0]);
	g_print->setCursor(ModelMenuPosCol, ModelMenuPosRow);
	g_print->print(menu[1]);
	g_print->setCursor(RunMenuPosCol, RunMenuPosRow);
	g_print->print(menu[2]);
	g_print->setCursor(0, 0);
}

void MainMenuHandler::advanceCaret(uint8_t dir) // -1 = left, 1 right. All other values ignored
{
	if(0xff == dir)
	{
		// left
		if(g_print->getCaretCol() == SetupMenuPosCol && g_print->getCaretRow() == SetupMenuPosRow)
		{
			g_print->setCursor(RunMenuPosCol, RunMenuPosRow);
		}
		else if(g_print->getCaretCol() == ModelMenuPosCol && g_print->getCaretRow() == ModelMenuPosRow)
		{
			g_print->setCursor(SetupMenuPosCol, SetupMenuPosRow);
		}
		else if(g_print->getCaretCol() == RunMenuPosCol && g_print->getCaretRow() == RunMenuPosRow)
		{
			g_print->setCursor(ModelMenuPosCol, ModelMenuPosRow);
		}
	}
	else if(1 == dir)
	{
		// right
		if(g_print->getCaretCol() == SetupMenuPosCol && g_print->getCaretRow() == SetupMenuPosRow)
		{
			g_print->setCursor(ModelMenuPosCol, ModelMenuPosRow);
		}
		else if(g_print->getCaretCol() == ModelMenuPosCol && g_print->getCaretRow() == ModelMenuPosRow)
		{
			g_print->setCursor(RunMenuPosCol, RunMenuPosRow);
		}
		else if(g_print->getCaretCol() == RunMenuPosCol && g_print->getCaretRow() == RunMenuPosRow)
		{
			g_print->setCursor(SetupMenuPosCol, SetupMenuPosRow);
		}
	}
}




