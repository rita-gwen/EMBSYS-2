


void LcdTouchTask(void* pdata)
{
    char buf[BUFSIZE];
    lcdCtrl = initLcd();
    PrintWithBuf(buf, BUFSIZE, "Initializing FT6206 touchscreen controller\n");
    
    touchCtrl = initTouch();

    int currentcolor = ILI9341_RED;
    
    while (1) { 
        
        // We are doing event waiting here instead of polling
        touchCtrl->waitForTouch();
        TS_Point rawPoint;
       
        //Retrieve a point  
        rawPoint = touchCtrl->getPoint();

        if (rawPoint.x == 0 && rawPoint.y == 0)
        {
            continue; // usually spurious, so ignore
        }
        
        // transform touch orientation to screen orientation.
        TS_Point p = TS_Point();
        p.x = MapTouchToScreen(rawPoint.x, 0, ILI9341_TFTWIDTH, ILI9341_TFTWIDTH, 0);
        p.y = MapTouchToScreen(rawPoint.y, 0, ILI9341_TFTHEIGHT, ILI9341_TFTHEIGHT, 0);
        
        lcdCtrl->fillCircle(p.x, p.y, PENRADIUS, currentcolor);
        DispatchUIEvent(&p);
    }
}