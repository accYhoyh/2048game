#ifndef __BMP_H__
#define __BMP_H__

/*
* 00-01 文件标识，为字母ASCII码"BM"					2byte 
* 02-05 文件大小									4byte 
* 06-09 位图文件保留字，必须为0 					4byte
* 0A-0D 文件开始到位图数据开始之间的偏移量			4byte
* 0E-11 图像描述信息块的大小，常为28H				4byte
* 12-15 图片高度									4byte 
* 16-19 图片宽度 									4byte 
* 1A-1B 图像plane总数，恒为 1						2byte
* 1C-1D 记录颜色的位数								2byte
* 1E-21 数据压缩方式								2byte
* 22-25 图像区数据大小，必须为4的倍数				4byte
* 26-29 水平像素点个数（在设备无关位图中，00H）		4byte 
* 2A-2D 垂直像素点个数 （在设备无关位图中，00H）	4byte
* 2E-31 图像所用颜色数（不用，固定为0）				4byte
* 32-35 重要颜色数（不用，固定为0）					4byte 
*
*/

void show_picture( char* pathname , int x , int y );


#endif
