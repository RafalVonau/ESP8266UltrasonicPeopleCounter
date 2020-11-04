
//===========================================================================================

#define BAT_0_drawH(x,y,w) TFT_drawPixmapH(x, y, w, BAT_0_IMAGE_H,BAT_0_IMAGE)
#define BAT_1_drawH(x,y,w) TFT_drawPixmapH(x, y, w, BAT_1_IMAGE_H,BAT_1_IMAGE)

//===========================================================================================

void Bat::draw()
{
	int x, p, r;
	if (m_needUpdate == 0) return;
	m_needUpdate = 0;
	
	x = m_x;
	/* Oryginal battery size = 38x18px , start = 8px, end = 4px, middle = 26 + 4 = 30 levels*/
	
	if ((m_val == BAT_MAX) || (m_val == BAT_MAX-1)) {
		/* Draw full battery */
		BAT_START_1_draw(x,m_y);x += BAT_START_1_IMAGE_W;
		BAT_1_drawH(x,m_y, 26);x += 26;
		BAT_END_1_draw(x,m_y);
	} else if ((m_val == 0) || (m_val == 1)) {
		/* Draw empty battery */
		BAT_START_0_draw(x,m_y);x += BAT_START_0_IMAGE_W;
		BAT_0_drawH(x,m_y, 26);x += 26;
		BAT_END_0_draw(x,m_y);
	} else {
		/* Draw some proggress */
		p = 25 - m_val;
		if (p<0) p = 0;
		r = 26 - p;
		BAT_START_0_draw(x,m_y);x += BAT_START_0_IMAGE_W;
		if (p>0) {BAT_0_drawH(x, m_y, p); x += p;}
		if (r>1) {BAT_ROUND_draw(x, m_y); x+=2; r-=2;}
		if (r>0) {BAT_1_drawH(x,m_y, r); x += r;}
		BAT_END_1_draw(x,m_y);
	}
}
//===========================================================================================