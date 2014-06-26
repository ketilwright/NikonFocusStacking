/*
 * DisplayableParameter.h
 *
 * Created: 6/26/2014 08:08:34 AM
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

class DisplayableParameter
{
	int32_t m_curVal;
	const int32_t m_minVal, m_maxVal;
	const uint8_t m_fieldWidth;
	void clamp()
	{
		if(m_curVal < m_minVal) m_curVal = m_minVal;
		if(m_curVal > m_maxVal) m_curVal = m_maxVal;
	}
public:
	DisplayableParameter(int32_t curVal, int32_t minVal, int32_t maxVal, uint8_t fieldWidth)
		:
		m_curVal(curVal),
		m_minVal(minVal),
		m_maxVal(maxVal),
		m_fieldWidth(fieldWidth)
	{}
	void changeVal(int32_t delta)
	{
		m_curVal += delta;
		clamp();
	}
	
	void display(uint8_t col, uint8_t row) const
	{
		// determine number of digits required
		uint8_t digits = 0;
		uint32_t val = m_curVal;
		while(val > 0)
		{
			val /= 10;
			++digits;
		}
		// save cursor, print in the requested location, padding with
		// zeros, and restore original cursor location.
		g_print->saveCursorLocation();
		g_print->setCursor(col, row);
		for(uint8_t z = 0; z < m_fieldWidth - digits; z++) g_print->print(F("0"));
		g_print->print(m_curVal);
		g_print->restoreCursorLocation();
	}
	int32_t getVal() const { return m_curVal; }
	void setVal(int32_t val) { m_curVal = val; clamp(); }		
};