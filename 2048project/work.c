#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/mman.h>
#include <math.h>
#include <time.h>
#include <stdlib.h>
#include <linux/input.h>
#include <string.h>
#include <pthread.h>
#include "lcd.h"
#include "bmp.h"
#include "random_num.h"

#define LCD_HEIGHT 480
#define LCD_WIDTH 800
#define ITEM_NUM  4   // 创建4行4列的矩阵
#define ITEM_WIDTH  100     //  图片宽100
#define ITEM_HEIGHT 100   //    图片高100
#define BLACK_LINE 5
#define MATRIX_X0 (LCD_WIDTH - (ITEM_WIDTH + BLACK_LINE)*ITEM_NUM)/2
#define MATRIX_Y0 (LCD_HEIGHT - (ITEM_HEIGHT + BLACK_LINE)*ITEM_NUM)/2
#define MOVE_UP 1
#define MOVE_DOWN 2
#define MOVE_LEFT 3
#define MOVE_RIGHT 4

int matrix_2048[4][4];
int matrix_back[4][4];
int flag, flag_degree, flag_win;
long long sum;

void print_2048(){
	int i, j;
	for( i = 0 ; i < ITEM_NUM ; i ++ )
	{
		for( j = 0 ; j < ITEM_NUM ; j ++ )
			printf("%d\t",matrix_2048[i][j]);
		printf("\n");
	}
}

int get_zeronum(){
	int i, j, n = 0;
	for( i = 0 ; i < ITEM_NUM ; i ++ )
		for( j = 0 ; j < ITEM_NUM ; j ++ )
			if( matrix_2048[i][j] == 0 )
				n ++;
	return n;
}

void set_rand_num(){
	int zero_Num = get_zeronum();
	int n = 0;
	int pos = rand() % zero_Num; // pos为随机出现2的位置   pos -> [0,zero_Num) 整数
	int i, j;
	for( i = 0 ; i < ITEM_NUM ; i ++ )
		for( j = 0 ; j < ITEM_NUM ; j ++ )
			sum += matrix_2048[i][j];

	for( i = 0 ; i < ITEM_NUM ; i ++ )
		for( j = 0 ; j < ITEM_NUM ; j ++ )
			if( matrix_2048[i][j] == 0 ){
				if( n == pos ) {
					if( sum >=    2200 ) {
						int res = rand()%10000;
						if( res == 9999 ) {
							matrix_2048[i][j] = 16384;
							return;
						}
					}
					int res = rand()%3;
					if( res < 2 ) matrix_2048[i][j] = 2;
					else matrix_2048[i][j] = 4;
					return;
				}
				else n++;
			}
}

void LCD_draw_matrix(){
	int i, j;
	int x0, y0;
	for( i = 0 ; i < ITEM_NUM ; i ++ ){
		for( j = 0 ; j < ITEM_NUM ; j ++ ){
			x0 = MATRIX_X0 + ( ITEM_WIDTH + BLACK_LINE ) * j + 5;
			y0 = MATRIX_Y0 + ( ITEM_HEIGHT + BLACK_LINE ) * i + 5;
			if( matrix_2048[i][j] == 0 ) {
				int k, z;
				for( k = 0 ; k < ITEM_WIDTH ; k ++ )
					for( z = 0 ; z < ITEM_HEIGHT ; z ++ )
						display_point( x0 + k , y0 + z , 0x4682b4 );
			}
			else{
				// 加载那个数字的图片 
				char pathname[32];
				sprintf( pathname , "%d.bmp" , matrix_2048[i][j] );
				show_picture( pathname , x0 , y0 );
			}
		}
	}
}

// 移动 
int get_movement(){
	int fd = open("/dev/input/event0", O_RDONLY);
	int res;
	if (fd == -1) {
		printf("open /dev/event0 failed\n");
		return -1;
	}
	int x1 = -1, y1 = -1; // 接触时候的坐标点
	int x2, y2; // 离开后的坐标点
	struct input_event ev;
	while( 1 ){
		res = read( fd , &ev, sizeof(ev) );
		if( res != sizeof(ev) ) continue;	
		if( ev.type == EV_ABS && ev.code == ABS_X ) {
			if( x1 == -1 ) x1 = ev.value;
			x2 = ev.value;
		} 
		if (ev.type == EV_ABS && ev.code == ABS_Y) {
			if (y1 == -1) y1 = ev.value;
			y2 = ev.value;
		}
		if( (ev.type == EV_ABS && ev.code == ABS_PRESSURE && ev.value == 0 ) || 
		    ( ev.type == EV_KEY && ev.code == BTN_TOUCH && ev.value == 0 ) ) {
		    
		    
		    printf("x2 = %d\ty2 = %d\n",x2,y2);
			printf("flag = %d\n",flag);
			printf("\n\n\n\n");
			if( flag ) return 0;
			printf("x1 = %d\ty1 = %d\nx2 = %d\ty2 = %d\n",x1,y1,x2,y2);
			int opposite_x = abs(x2-x1); // 左右 
			int opposite_y = abs(y2-y1); // 上下 
			printf("opposite_x = %d\topposite_y = %d\n",opposite_x,opposite_y);
			
		    if( x2 >= 830 && y2 < 80 && opposite_x <= 40 && opposite_y <= 32 ) {
		    	flag = 1;
		    	return 0;
			}
			if( opposite_x <= 40 && opposite_y <= 32 ) {
				x1 = -1;
				y1 = -1;
				continue;
			}
			if( opposite_x > 2 * opposite_y ){ // 左右移动 
				if( x2 > x1 ){ // 方块向右边移动 
					close(fd);
					return MOVE_RIGHT;
				}
				else{ // 方块向左边移动 
					close(fd);
					return MOVE_LEFT;
				}
			} 
			else if( opposite_x < 2* opposite_y ){ // 上下移动 
				if( y2 > y1 ){ // 方块向下移动 
					close(fd);
					return MOVE_DOWN;
				}
				else{ // 方块向上移动 
					close(fd);
					return MOVE_UP;
				}
			}
			else x1 = -1, y1 = -1;
		}
	}
	close(fd);
}

// 向上移动
void move_up(){
	int i, j;
	int x, y;
	for (i = 0; i < ITEM_NUM; i++) {
		for (x = 0; x < ITEM_NUM; ) {
			if (matrix_2048[x][i] != 0) {
				for (y = x + 1; y < ITEM_NUM; y++) {
					if (matrix_2048[y][i] != 0) {
						if (matrix_2048[x][i] == matrix_2048[y][i]) {
							matrix_2048[x][i] += matrix_2048[y][i];
							matrix_2048[y][i] = 0;
							x = y + 1;
							break;
						}
						else x = y;
					}
				}
				if (y >= ITEM_NUM) break;
			}
			else x++;
		}       
		x = 0;
		for (y = 0; y < ITEM_NUM; y++) {
			if (matrix_2048[y][i] != 0) {       
				if (x != y) {
					matrix_2048[x][i] = matrix_2048[y][i];
					matrix_2048[y][i] = 0;
				}
				x++;
			}
		}
	}
}

// 向下移动
void move_down(){
	int i, j;
	int x, y;
	for (i = 0; i < ITEM_NUM; i++) {
		for (x = ITEM_NUM; x >=0; ) {
			if (matrix_2048[x][i] != 0) {
				for (y = x - 1; y >= 0; y--) {
					if (matrix_2048[y][i] != 0) {
						if (matrix_2048[x][i] == matrix_2048[y][i]) {
							matrix_2048[x][i] += matrix_2048[y][i];
							matrix_2048[y][i] = 0;
							x = y -1;
							break;
						}
						else x = y;
					}
				}
				if (y<0) break;
			}
			else  x--;
		}
		x = ITEM_NUM-1;
		for (y = ITEM_NUM-1 ; y >= 0; y--) {
			if (matrix_2048[y][i] != 0) {
				if (x != y) {
					matrix_2048[x][i] = matrix_2048[y][i];
					matrix_2048[y][i] = 0;
				}
				x--;
			}
		}
	}
}

// 向左移动
void move_left(){
	int i, j;
	int x, y;
	for (i = 0; i < ITEM_NUM; i++) {
		for (x = 0; x < ITEM_NUM; ) {
			if (matrix_2048[i][x] != 0) {
				for (y = x + 1; y < ITEM_NUM; y++) {
					if (matrix_2048[i][y] != 0) {
						if (matrix_2048[i][x] == matrix_2048[i][y]) {
							matrix_2048[i][x] += matrix_2048[i][y];
							matrix_2048[i][y] = 0;
							x = y + 1;
							break;
						}
						else {
							x = y;
						}
					}
				}
				if (y >= ITEM_NUM) break;
			}
			else x++;
		}
		x = 0;
		for (y = 0 ; y < ITEM_NUM; y++) {
			if (matrix_2048[i][y] != 0) {
				if (x != y) {
					matrix_2048[i][x] = matrix_2048[i][y];
					matrix_2048[i][y] = 0;
				}
				x++;
			}
		}
    }
}

// 向右移动
void move_right(){
	int i, j;
	int x, y;
	for (i = 0; i < ITEM_NUM; i++) {
		for (x = ITEM_NUM; x >0; ) {
			if (matrix_2048[i][x] != 0) {
				for (y = x-1; y>=0; y--) {
					if (matrix_2048[i][y] != 0) {
						if (matrix_2048[i][x] == matrix_2048[i][y]) {
							matrix_2048[i][x] += matrix_2048[i][y];
							matrix_2048[i][y] = 0;
							x = y - 1;
							break;
						}
						else x = y;
					}
				}
				if (y<0) break;
			}
			else x--;
		}
		x = ITEM_NUM-1;
		for (y = ITEM_NUM-1 ; y >= 0; y--) {
			if (matrix_2048[i][y] != 0) {
				if (x != y) {
					matrix_2048[i][x] = matrix_2048[i][y];
					matrix_2048[i][y] = 0;
				}
				x--;
			}
		}
	}
}

// 滑动变化 
void change_matrix( int mv ){
	switch(mv){
		case MOVE_UP : move_up();break;
		case MOVE_DOWN : move_down();break;
		case MOVE_LEFT : move_left();break;
		case MOVE_RIGHT : move_right();break;
		default: break; 
	}
}

// 游戏结束 需要判断是否能继续滑动 
int is_gameover(){
	int x, y;
	for( x = 0 ; x < ITEM_NUM ; x ++ )
		for( y = 0 ; y < ITEM_NUM ; y ++ ){
			if( matrix_2048[x][y] == 0 ) return 0;
			if(x<ITEM_NUM-1 && matrix_2048[x][y]==matrix_2048[x+1][y]) return 0;
			if(y<ITEM_NUM-1 && matrix_2048[x][y]==matrix_2048[x][y+1]) return 0;
		}
	for( y = LCD_HEIGHT ; y ; y -= 10 )
		show_picture("gameover.bmp",0,y);
	return 1;
}

int start(){
	int fd = open("/dev/input/event0", O_RDONLY);
	int res;
	if (fd == -1) {
		printf("open /dev/event0 failed\n");
		return -1;
	}
	int x1 = -1, y1 = -1; // 接触时候的坐标点
	int x2, y2; // 离开后的坐标点
	struct input_event ev;
	while( 1 ){
		res = read( fd , &ev, sizeof(ev) );
		if( res != sizeof(ev) ) continue;	
		if( ev.type == EV_ABS && ev.code == ABS_X ) {
			if( x1 == -1 ) x1 = ev.value;
			x2 = ev.value;
		} 
		if (ev.type == EV_ABS && ev.code == ABS_Y) {
			if (y1 == -1) y1 = ev.value;
			y2 = ev.value;
		}
		if( (ev.type == EV_ABS && ev.code == ABS_PRESSURE && ev.value == 0 ) || 
		    ( ev.type == EV_KEY && ev.code == BTN_TOUCH && ev.value == 0 ) ) {
		    if( x2 >= 30 && x2 <= 180 && y2 >= 30 && y2 <= 230 )
		    	flag_degree = 1;
		    else if( x2 >= 210 && x2 <= 360 && y2 >= 30 && y2 <= 230 )
		    	flag_degree = 2;
		    else if( x2 >= 410 && x2 <= 770 && y2 >= 30 && y2 <= 410 )
		    	flag_degree = 3;
		    else continue;
			close(fd);
		    return 1;
		}
	}
}

int again(){
	int fd = open("/dev/input/event0", O_RDONLY);
	int res;
	if (fd == -1) {
		printf("open /dev/event0 failed\n");
		return -1;
	}
	int x1 = -1, y1 = -1; // 接触时候的坐标点
	int x2, y2; // 离开后的坐标点
	struct input_event ev;
	while( 1 ){
		res = read( fd , &ev, sizeof(ev) );
		if( res != sizeof(ev) ) continue;	
		if( ev.type == EV_ABS && ev.code == ABS_X ) {
			if( x1 == -1 ) x1 = ev.value;
			x2 = ev.value;
		} 
		if (ev.type == EV_ABS && ev.code == ABS_Y) {
			if (y1 == -1) y1 = ev.value;
			y2 = ev.value;
		}
		if( (ev.type == EV_ABS && ev.code == ABS_PRESSURE && ev.value == 0 ) || 
		    ( ev.type == EV_KEY && ev.code == BTN_TOUCH && ev.value == 0 ) ) {
		    if( flag_win ){
		    	if( x2 >= 610 && y2 >= 310 ){
			    	close(fd);
			    	return 1;
			    }
			    else continue;
			}
		    else if( x2 >= 60 && y2 >= 120 && x2 <= 540 && y2 <= 250 )
		    {
				close(fd);
		    	return 1;
			}
			else continue;
		}
	}
}

void* music_play(){
	while( 1 ){
		system("madplay Q normal_music.mp3 &");
		sleep(30);
		system("killall madplay");
	}
}

void score(){
	int i, j;
	int sum = 0;
	for( i = 0 ; i < ITEM_NUM ; i ++ )
		for( j = 0 ; j < ITEM_NUM ; j ++ )
		{
			sum += matrix_2048[i][j];
			if( flag_degree == 1 && matrix_2048[i][j] >= 128 )
				flag_win = 1;
			else if( flag_degree == 2 && matrix_2048[i][j] >= 2048 )
				flag_win = 1;
			else if( flag_degree == 3 && matrix_2048[i][j] >= 16384 )
				flag_win = 1;
		}
	int n = 0, t = sum;
	int wei[10] = {0};
	i = 1;
	if( t == 0 ) n = 1;
	else 
		while( t ){
			wei[i++] = t%10;
			t/=10;
			n++;
		}
	char score_pathname[32];
	if( n == 1 ){
		sprintf( score_pathname , "sum_%d.bmp" , wei[1] );
		show_picture( score_pathname , 80 , 250 );
	}
	else if( n == 2 ){
		sprintf( score_pathname , "sum_%d.bmp" , wei[2] );
		show_picture( score_pathname , 70 , 250 );
		sprintf( score_pathname , "sum_%d.bmp" , wei[1] );
		show_picture( score_pathname , 90 , 250 );
	}
	else if( n == 3 ){
		sprintf( score_pathname , "sum_%d.bmp" , wei[3] );
		show_picture( score_pathname , 60 , 250 );
		sprintf( score_pathname , "sum_%d.bmp" , wei[2] );
		show_picture( score_pathname , 80 , 250 );
		sprintf( score_pathname , "sum_%d.bmp" , wei[1] );
		show_picture( score_pathname , 100 , 250 );
	}
	else if( n == 4 ){
		sprintf( score_pathname , "sum_%d.bmp" , wei[4] );
		show_picture( score_pathname , 50 , 250 );
		sprintf( score_pathname , "sum_%d.bmp" , wei[3] );
		show_picture( score_pathname , 70 , 250 );
		sprintf( score_pathname , "sum_%d.bmp" , wei[2] );
		show_picture( score_pathname , 90 , 250 );
		sprintf( score_pathname , "sum_%d.bmp" , wei[1] );
		show_picture( score_pathname , 110 , 250 );
	}
	else if( n == 5 ){
		sprintf( score_pathname , "sum_%d.bmp" , wei[5] );
		show_picture( score_pathname , 40 , 250 );
		sprintf( score_pathname , "sum_%d.bmp" , wei[4] );
		show_picture( score_pathname , 60 , 250 );
		sprintf( score_pathname , "sum_%d.bmp" , wei[3] );
		show_picture( score_pathname , 80 , 250 );
		sprintf( score_pathname , "sum_%d.bmp" , wei[2] );
		show_picture( score_pathname , 100 , 250 );
		sprintf( score_pathname , "sum_%d.bmp" , wei[1] );
		show_picture( score_pathname , 120 , 250 );
	}
	
}

int main( int argc , char *argv[] ){
	srand(time(NULL));
	pthread_t tid;
	pthread_create(&tid,NULL,music_play,NULL);
	lcd_init();
	//show_a_pure_color(0x555555);
	while(1)
	{
		show_picture("start.bmp",0,0);
		show_picture("normal.bmp",30,30);
		show_picture("hard.bmp",210,30);
		show_picture("hell.bmp",410,30);
		if( start() ){
			while( 1 ) {
				flag = 0;
				flag_win = 0;
				memset(matrix_2048,0,sizeof(matrix_2048));
				show_picture("back_ground.bmp",0,0); 
				set_rand_num();
				LCD_draw_matrix(); 
				// 游戏运行 
				
				show_picture("score_background.bmp",30,200);
				score();
				while(1){
					show_picture("again.bmp",640,0);
					int mv = get_movement();
					change_matrix(mv);
					set_rand_num();
					show_picture("score_background.bmp",30,200);
					score();
					if( flag ) break;
					LCD_draw_matrix();
					print_2048();
					if( flag_win ) {
						int y;
						for( y = LCD_HEIGHT ; y >= 0 ; y -= 10 )
							show_picture("win.bmp",0,y);
						break;
					}
					if( is_gameover() ) break;
				}
				if( flag ) break;
				if( again() ) break;
			}
		}
	}
	lcd_close();
	return 0;
}
