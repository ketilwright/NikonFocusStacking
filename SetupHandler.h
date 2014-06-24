/*
 * SetupHandler.h
 *
 * Created: 4/29/2014 11:38:34 AM
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

#pragma once

#include "MessageHandler.h"
#include "Sketch.h"
class SetupHandler : public IMessageHandler
{
    int32_t m_driveAmount;
    int32_t m_numFrames;
	int32_t m_frameDelaySeconds;
	bool    m_restoreFocus;
    void updateDriveAmountUI(int change = 0);
    void updateFramesUI(int change = 0);
	void updateFrameDelayUI(int change = 0);
	void updateRestoreFocusUI(int change = 0);
	void advanceCaret(uint8_t dir); // -1 = left, 1 right. All other values ignored
 public:
    SetupHandler(MessagePump *_pump, uint32_t driveAmount, uint32_t frames);
    uint32_t getDriveAmount() const { return m_driveAmount;}
    void setDriveAmount(uint32_t driveAmount) { m_driveAmount = driveAmount;}
    uint32_t getNumFrames() const { return m_numFrames; }
    void setNumFrames(uint32_t numFrames) { m_numFrames = numFrames;}
	uint32_t getFrameDelayMilliseconds() const { return m_frameDelaySeconds * 1000; }
	void setFrameDelaySeconds(uint32_t delay) { m_frameDelaySeconds = delay; }
	
	bool getRestoreFocus() const { return m_restoreFocus; }
	void setRestoreFocus(bool restore) { m_restoreFocus = restore; }
	
	
    ~SetupHandler();
    MsgResp processMessage(Msg& msg);
    virtual void show();
};



