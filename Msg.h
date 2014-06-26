/*
 * Msg.h
 *
 * Created: 6/26/2014 10:41:44 AM
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
 
 class IMessageHandler;
 
 // Globally unique message types. This should
 // tell us what 'happened', not where the message
 // came from.
 enum MsgType
 {
	 eButtonActionPress		= 0x00,
	 eButtonActionRelease	= 0x01,
	 eButtonActionHoldShort  = 0x02,
	 eButtonActionHoldMedium = 0x03,
	 eButtonActionHoldLong   = 0x04,
	 eButtonActionNone	    = 0xff
 };

 // Globally unique message codes. This should
 // tell us where a message originated, eg which
 // button, not what 'happened'
 enum MsgCode
 {
	 eLeft,
	 eRight,
	 eUp,
	 eDown,
	 eSelect,
	 eUnknown
 };

 // Return codes from IMessageHandler::processMessage.
 enum MsgResp
 {
	 eSuccess,
	 eFail,
 };

// Msg is dispatched from the loop function, and possibly
// other places to the MessagePump.
struct Msg
{
	// press, release,
	MsgType m_type;
	// eg, which button was pressed.
	const MsgCode m_code;
	// Deposit the value of next handler here before returning from
	// IMessageHandler::processMessage to set make a new handler.
	// Otherwise, leave it null. Depositing this pointer will force
	// a complete refresh of the LCD.
	IMessageHandler *m_nextHandler;
	Msg(MsgType _type, MsgCode _code) : m_type(_type), m_code(_code), m_nextHandler(0){}
};
