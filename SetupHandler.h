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
#include "DisplayableParameter.h"
#include "Sketch.h"

// column start/end for setup field
#define AmountFieldStartCol 0
#define AmountFieldRow 1
#define AmountFieldEnd   3
#define FrameFieldStart  5
#define FrameFieldEnd    7
#define FrameFieldRow	 1
#define DelayFieldStart  9
#define DelayFieldEnd	 10
#define DelayFieldRow	1
#define RestoreFocusFieldStart 13
#define RestoreFocusFieldEnd 13


class SetupHandler : public IMessageHandler
{
	DisplayableParameter m_driveAmount;
	DisplayableParameter m_numFrames;
	DisplayableParameter m_frameDelaySeconds;
	bool    m_restoreFocus;
	void updateRestoreFocusUI(int change = 0);
	void advanceCaret(uint8_t dir); // -1 = left, 1 right. All other values ignored
 public:
    SetupHandler(MessagePump *_pump, uint32_t driveAmount, uint32_t frames);
    uint32_t getDriveAmount() const { return m_driveAmount.getVal();}
    void setDriveAmount(uint32_t driveAmount) { m_driveAmount.setVal(driveAmount);}
    uint32_t getNumFrames() const { return m_numFrames.getVal(); }
    void setNumFrames(uint32_t numFrames) { m_numFrames.setVal(numFrames);}
	uint32_t getFrameDelayMilliseconds() const { return m_frameDelaySeconds.getVal() * 1000; }
	void setFrameDelaySeconds(uint32_t delay) { m_frameDelaySeconds.setVal(delay); }
	
	bool getRestoreFocus() const { return m_restoreFocus; }
	void setRestoreFocus(bool restore) { m_restoreFocus = restore; }
	
	
    ~SetupHandler();
    MsgResp processMessage(Msg& msg);
    virtual void show();
	friend class ManualHandler; // allow access to updateDriveAmountUI
};



