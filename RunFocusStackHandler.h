/*
 * RunFocusStackHandler.h
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

#pragma once

class MessagePump;
class __FlashStringHelper;
#include "MessageHandler.h"

// Message handler for running the focus stack.
// The only action supported is to press eSelect
// to cancel the operation
class RunFocusStackHandler : public IMessageHandler
{
	unsigned long m_lastUpdateTime;
public:
	RunFocusStackHandler(MessagePump *_pump);
	~RunFocusStackHandler();
    MsgResp processMessage(Msg& msg);
    virtual void show();
	// UI update methods
	void reportStatus(const __FlashStringHelper* msg);
	void reportFrame(uint16_t frame);
	void reportDelay(uint32_t curMilliseconds);
	unsigned long getLastUpdateTime() const;
	void resetLastUpdateTime(uint32_t curMilliseconds);
};

