#ifndef __LCD_H__
#define __LCD_H__


//lcd屏幕的初始化
int lcd_init();

//在任意的点上 显示任意的一个颜色
void display_point(int x, int y, int color);

//将屏幕变为纯色 
void show_a_pure_color( int color );

//关闭屏幕 
int lcd_close();

#endif
