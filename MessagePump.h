/*
 * MessagePump.h
 *
 * Created: 4/26/2014 8:37:29 AM
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
#include "Sketch.h"

// Forward decls.
class IMessageHandler;
class Msg;

// MessagePump dispatches Msg structures, usually from the main
// loop function, and hands them to an IMessageHandler subclass's
// processMessage override function for handling.

class MessagePump
{
    // Currently active message handler object. When NULL, no
    // action is taken.
	IMessageHandler *m_currentHandler;
public:
    // Construct with a handler, or NULL.
	MessagePump(IMessageHandler *handler);
    // Makes the next Handler active, calling show() on it
    // and making it the recipient of subsequent Msg objects
    // passed to dispatch()
	void setNextHandler(IMessageHandler *next);
    // If there is a current handler, calls processMessage
    // on it, passing msg.
	uint16_t dispatch(Msg& msg);
};

