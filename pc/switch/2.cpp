
void Switch::draw()
{
	if (m_needUpdate == 0) return;
	m_needUpdate = 0;
	switch (m_val) {
		case 0: {NIE_draw(m_x,m_y);} break;
		default: {TAK_draw(m_x,m_y);} break;
	}
}
//===========================================================================================
