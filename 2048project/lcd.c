#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/mman.h>

#define MaxSize 800*480

int *plcd = NULL;
int lcd_fd;

//lcd屏幕的初始化
int lcd_init(){
	//打开屏幕
	lcd_fd = open( "/dev/fb0" , O_RDWR );
	if( lcd_fd == -1 ){
		perror("open failed!");
		return -1; 
	}
	//内存映射
	plcd = mmap( NULL , MaxSize*4 , PROT_READ | PROT_WRITE , MAP_SHARED , lcd_fd , 0 );
	return 0;
}

//在任意的点上 显示任意的一个颜色
void display_point(int x, int y, int color)
{
	if( x >= 0 && x < 800 && y >= 0 && y < 480 )
		*(plcd + x + y*800) = color;
}

//将屏幕变为纯色 
void show_a_pure_color( int color )
{
	int i, j;
	for( i = 0 ; i < 480 ; i++ )
		for( j = 0 ; j < 800 ; j++ )
			display_point(j,i,color);
}

//关闭屏幕 
int lcd_close()
{
	//关闭文件 
	close(lcd_fd);
	//解除映射
	int res = munmap( plcd , MaxSize*4 );
	if( res == -1 ){
		perror("Removal failed!");
		return -1;
	}
	return 0;
}

