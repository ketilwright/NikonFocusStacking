/*
 * MessagePump.cpp
 *
 * Created: 4/29/2014 10:34:28 AM
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

#include "MessagePump.h"
#include "MessageHandler.h"

// Construct with a handler, or NULL.
MessagePump::MessagePump(IMessageHandler *handler)
    :
    m_currentHandler(handler)
{}

// Shows next IMessageHandler to the user, and makes
// it the handler of subsequent Msgs.
void MessagePump::setNextHandler(IMessageHandler *next)
{
    m_currentHandler = next;
    m_currentHandler->show();
}

// If there is a current handler, calls processMessage
// on it, passing msg.
uint16_t MessagePump::dispatch(Msg& msg)
{
    uint16_t rc = eFail;
    if(m_currentHandler)
    {
        uint16_t rc = m_currentHandler->processMessage(msg);
        switch(rc)
        {
            case eSuccess: // todo: why aren't we looking for eSetNextHandler?
            {
                if(msg.m_nextHandler)
                {
                    m_currentHandler = msg.m_nextHandler;
                    m_currentHandler->show();
                }
                break;
            }
            default:
            {
                // TODO: optionally beep() or do something more informative on eFail.
                break;
            }
        }
    }
    // Make sure that there can be no chance of a Msg
    // retaining a stale handler across successive loops().
    msg.m_nextHandler = NULL;
    return rc;
}