/*
 * NikType003State.cpp
 *
 * Created: 5/7/2014 8:56:43 AM
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

#include "NikType003State.h"
#include "NikType003.h"
#include "LcdImpl.h"
#include "MessagePump.h"
#include "MainMenu.h"
extern MainMenuHandler *g_pMain;
extern MessagePump g_pump;


NikType003StateHandler::NikType003StateHandler()
    : stateConnected(stInitial)
{}

// Tells NikType003 that it's disconnected.
// Setup menu will function, but stack will be unavailable.
void NikType003StateHandler::OnDeviceDisconnectedState(PTP *ptp)
{
    if ((stateConnected == stConnected || stateConnected == stInitial))
    {
        stateConnected = stDisconnected;
        NikType003 *nk3 = reinterpret_cast<NikType003*>(ptp);
        nk3->setIsConnected(false);
		//Serial.print("disconnected camera");
        g_pump.setNextHandler(g_pMain);
    }
}
// Tells NikType003 that it's connected
// Both setup & stack will function.
void NikType003StateHandler::OnDeviceInitializedState(PTP *ptp)
{
    if ((stateConnected == stDisconnected || stateConnected == stInitial))
    {
        stateConnected = stConnected;
        NikType003 *nk3 = reinterpret_cast<NikType003*>(ptp);
        nk3->setIsConnected(true);
		//Serial.print("connected camera");
        g_pump.setNextHandler(g_pMain);
    }
}
