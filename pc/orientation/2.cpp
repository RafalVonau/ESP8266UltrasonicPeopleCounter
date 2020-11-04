
void Orientation::draw()
{
	if (m_needUpdate == 0) return;
	m_needUpdate = 0;
	switch (m_val) {
		case 0: {NORMALNYZAZNACZONY_draw(m_x,m_y); ODBITYODZNACZONY_draw((m_x+73), m_y);} break;
		default: {NORMALNYODZNACZONY_draw(m_x,m_y); ODBITYZAZNACZONY_draw((m_x+73), m_y);} break;
	}
}
//===========================================================================================

void Orientation::apply()
{
	if (m_val == 0) TFT_setRotation(1); else TFT_setRotation(3);
}
//===========================================================================================