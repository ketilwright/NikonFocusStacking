/* 
* ModelMenu.h
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


#pragma once

#include "MessageHandler.h"
#include "DisplayableParameter.h"

class ManualHandler : public IMessageHandler
{
	void advanceCaret(uint8_t dir);
	void focus(uint8_t dir);
    DisplayableParameter m_pos;
public:
	ManualHandler(MessagePump *_pump);
    MsgResp processMessage(Msg& msg);
    virtual void show();
}; 

