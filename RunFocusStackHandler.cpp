/*
 * RunFocusStackHandler.cpp
 *
 * Created: 4/30/2014 10:57:24 AM
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

#include "RunFocusStackHandler.h"
#include "SetupHandler.h"
#include "NikType003.h"
#include "LcdImpl.h"
#include <WString.h>
extern IMessageHandler *g_pMain;
extern SetupHandler *g_pSetup;
extern NikType003  nk3;

#define FrameNumberPos 4
#define StatusPos 8
#define DelayPos 6

RunFocusStackHandler::RunFocusStackHandler(MessagePump *_pump)
    :
    IMessageHandler(_pump)
{
    menu[0] = "Frm";
}

RunFocusStackHandler::~RunFocusStackHandler()
{
}

MsgResp RunFocusStackHandler::processMessage(Msg& msg)
{
    MsgResp rsp = eFail;
    if(eButtonActionPress == msg.m_type)
    {
        nk3.cancelFocusStack();
        msg.m_nextHandler = g_pMain;
        rsp = eSuccess;
    }
    return rsp;
}
void RunFocusStackHandler::show()
{
	g_print->clear();
	g_print->setCursor(0, 0);
	g_print->print(menu[0]);
	showCaret(false);
	nk3.startFocusStack();
}
void RunFocusStackHandler::reportStatus(const __FlashStringHelper* msg)
{
	g_print->setCursor(StatusPos, 0);
	g_print->print(F("       "));
	g_print->setCursor(StatusPos, 0);
	g_print->print(msg);
	g_print->setCursor(0, 1);
	g_print->print(F("                "));
}
void RunFocusStackHandler::reportFrame(uint16_t frame)
{
	g_print->setCursor(FrameNumberPos, 0);
	g_print->print(F("    "));
	g_print->setCursor(FrameNumberPos, 0);
	g_print->print(frame);
}
void RunFocusStackHandler::reportDelay(uint32_t curMilliseconds)
{
	if((curMilliseconds - getLastUpdateTime()) > 1000)
	{
		uint32_t seconds = ((double)curMilliseconds - (double)nk3.getTimeLastCaptureStart()) / 1000;
		resetLastUpdateTime(curMilliseconds);
		g_print->setCursor(0, 1);
		if(seconds > 0)
		{
			g_print->print(F("Delay"));
			g_print->setCursor(DelayPos, 1);
			g_print->print(F("    "));
			g_print->setCursor(DelayPos, 1);
			g_print->print((int)g_pSetup->getFrameDelayMilliseconds() / 1000 - seconds);	
		}
		else
		{
			g_print->setCursor(DelayPos, 1);
			g_print->print(F("        "));
		}
	}
}
unsigned long RunFocusStackHandler::getLastUpdateTime() const { return m_lastUpdateTime;}
void RunFocusStackHandler::resetLastUpdateTime(uint32_t curMilliseconds) { m_lastUpdateTime = curMilliseconds; }