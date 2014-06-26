/*
 * Handlers.h
 *
 * Created: 4/26/2014 9:09:40 AM
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

#pragma once
#include "Msg.h"
#include "LcdImpl.h"
#include <stdint.h>

// Forward decls for message struct
class MessagePump;
class IMessageHandler;




class IMessageHandler
{
    // TODO: consider just using g_pump.
    MessagePump *m_pump;
protected:
    // Derived classes should write menu text strings
    const char *menu[5];
	void printMenuItem(uint8_t col, uint8_t row, uint8_t item)
	{
		g_print->setCursor(col, row);
		g_print->print(menu[item]);
	}
public:
    // Base class ctor just initializes OK on bottom line of menu[].
    IMessageHandler(MessagePump* pump)
		:
		m_pump(pump)
	{
		memset(menu, 0, sizeof(menu)); 
	}
    // Must be implemented in derived classes.
    virtual MsgResp processMessage(Msg&) = 0;
    // Sub classes must implement show() to display
	// their initial menu.
    virtual void show() = 0;
};



