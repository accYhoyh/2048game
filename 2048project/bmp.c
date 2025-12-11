#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/mman.h>
#include <math.h>
#include <stdlib.h>
#include "lcd.h"

void show_picture(char * pathname ,int x ,int y)
{
	int fd = open(pathname,O_RDONLY);
	if(fd == -1)
	{
		perror("open error\n");
		return ;
	}

	int width,height;
	short depth;
	unsigned char buf[4] ;
	//读取宽度
	lseek(fd,0x12,SEEK_SET);
	read(fd,buf,4);
	width = buf[3]<<24 | buf[2]<< 16 | buf[1] << 8 | buf[0];
	//读取高度
	read(fd,buf,4);
	height  = buf[3]<<24 | buf[2]<< 16 | buf[1] << 8 | buf[0];
	//读取色深
	lseek(fd,0x1c,SEEK_SET);
	read(fd,buf,2);
	depth = buf[1] << 8  | buf[0];
	//像素数组 
	int line_valid_bytes = abs(width) * depth / 8 ; //一行本有的有效字节
	int laizi = 0; //填充字节， 文件大小为 54 + wedth*height + height*n 为4的倍数 
	if( (line_valid_bytes % 4) !=0   ) laizi =  4 - line_valid_bytes%4;
	int line_bytes = line_valid_bytes + laizi; //一行所有的字节数
	int total_bytes = line_bytes * abs(height); //整个像素数组的大小
	unsigned char * p1  = malloc(total_bytes); // 获取动态数组
	
	// 像素为54字节之后，所以调到54读完
	lseek(fd,54,SEEK_SET);
	read(fd,p1,total_bytes);
	
	// 画点，画图
	unsigned char a ,r ,g, b ;
	int i = 0;//用来做指针运动的
	int x0=0,y0=0; //用来循环计数
	int color;
	for( y0 = 0 ; y0 < abs(height) ; y0 ++ ) { // 列 
		for( x0 = 0 ; x0 < abs(width) ; x0 ++ ) { // 行 
			//一字节一字节读入RGBA
			// 读取后，图片顺序会反过来，需要调整
			b = p1[i++];
			g = p1[i++];
			r = p1[i++];
			if(depth == 32)
			{
				a=p1[i++];
			}
			if(depth == 24)
			{
				a = 0;
			}
			color = a << 24 | r << 16 | g << 8 | b ;
			display_point(width>0?x+x0:abs(width)+x-1-x0, height>0? y+height-1-y0 : y+y0,color);
			
			
		}
		// 一行弄完需要进行填充过滤 
		i = i +laizi;
	}
	// 释放指针 
	free(p1);
	close(fd);
}
