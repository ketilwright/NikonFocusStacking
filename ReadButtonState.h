/* 
* ReadButtonState.h
*
* Created: 5/24/2014 1:27:25 PM
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


// Adapt reading a button to a HW implementation
class ReadButtonState
{
public:
	virtual ~ReadButtonState() {};
	virtual bool isPressed(uint8_t button) = 0;
	virtual void setPinMode(uint8_t button, uint8_t mode) = 0;
};

