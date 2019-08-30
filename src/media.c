/*
编译命令：arm-linux-gcc media.c lcd_app.c -o media -I./include -I./include/freetype2 -L./lib -ljpeg -lfreetype -lpthread

													多媒体播放器

*/
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <linux/input.h> //输入模型的定义

#include <unistd.h>
#include <sys/mman.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <jpeglib.h>
#include <setjmp.h>
#include "lcd_app.h"
#include <pthread.h>

int *lcd=NULL;
//主界面
void touch_music_media()
{
	show_bmp("./photos/main.bmp",0,0);
	
	Lcd_Show_FreeType(lcd,L"多媒体播放器",64,0xffffff,1,0xffaec9,150,90);
	Lcd_Show_FreeType(lcd,L"音乐",64,0xffffff,0,0xffaec9,112,271);
	Lcd_Show_FreeType(lcd,L"播放",64,0xffffff,0,0xffaec9,112,335);
	
	
	Lcd_Show_FreeType(lcd,L"视频",64,0xffffff,0,0xffaec9,372,271);
	Lcd_Show_FreeType(lcd,L"播放",64,0xffffff,0,0xffaec9,372,335);
}
//线程执行触摸屏
void *touch_tid (void *arg)
{
	//打开触摸屏设备 
	struct input_event ts;
	int tsfd = open("/dev/input/event0",O_RDWR);
	if(tsfd < 0)
	{
		perror("");
		return;
	}	
	int x=0,y=0;
	while(1)
	{
		read(tsfd,&ts,sizeof(ts));
		if(ts.type == EV_ABS && ts.code == ABS_X)
		{
			x = ts.value;//输出X轴的坐标值
		}
		if(ts.type == EV_ABS && ts.code == ABS_Y)
		{
			y = ts.value;//输出X轴的坐标值 
		}

		if(ts.type == EV_KEY && ts.code == BTN_TOUCH && ts.value == 0)
		{
			if(x>101&&y>191&&x<247&&y<363)
			{
				printf("音乐播放\n");
			}
			
			if(x>366&&y>194&&x<514&&y<358)
			{
				printf("视频播放\n");
				system("./touch_video ./");
				touch_music_media();
				
			}
			
			x=0;y=0;
		}
	}
	close(tsfd);
}


//画矩形
void drw(int x,int y,int px,int py,int color)
{
	int i=0,j=0;
	for(j=0;j<y;j++)
	{
		for(i=0;i<x;i++)
		{
			if(j+py < 480 && i+px < 800)
			{
				*(lcd +(j+py)*800+(i+px)) = color;
			}
		
		}
	}
}
//bmp
int show_bmp(char *bmp_a,int px,int py)
{
	int chang,kuan;
	int bmp_fd=open(bmp_a,O_RDWR);
	if(bmp_fd<0)
	{
		perror("NO TIMG.BMP");
		return-1;
	}
	
	lseek(bmp_fd,18,SEEK_SET);//偏移到18个
	read(bmp_fd,&chang,4);
	read(bmp_fd,&kuan,4);
	
	// printf("chang=%d\tkuan=%d\n",chang,kuan);
	
	lseek(bmp_fd,54,SEEK_SET);//偏移到54个
	int buf[chang][kuan];
	
	char tmp_buf[chang*kuan*3];//24位色 定义800*400*3的数组
	read(bmp_fd,tmp_buf,sizeof(tmp_buf));
	
	unsigned char r=0,g=0,b=0,a=0,*p=tmp_buf;
	int color =0,x=0,y=0;//无符号整形减到-1还是正数

	for(y=kuan-1;y>=0;y--)
	{
		for(x=0;x<chang;x++)
		{
			if(kuan>480&&chang>800)
			{
					continue;
			}
			b =*p++;g =*p++;r =*p++;
			
			color=a<<24|r<<16|g<<8|b;
			
			buf[x][y] = color;
			*(lcd+(y+py)*800+(x+px))=buf[x][y];
		}
		
	}
	
	close(bmp_fd);
	
	//munmap(lcd_a,800*480*4);
	return 1;
}
//遍历目录
/* int read_dir(struct doublelist *h,char *dir)
{
	DIR *fd=opendir(dir);
	if(fd==NULL)
	{
		perror("opendir dir");
		return -1;
	}
	while(1)
	{
		
		struct dirent *p=readdir(fd);
		if(p==NULL)
		{
			break;
		}
		if(p->d_name[0]=='.')//读到‘.’目录文件就跳过
		{
			continue;
		}
		if(p->d_type==DT_DIR)//读到目录文件，递归
		{
			char buf[256]={0};
			sprintf(buf,"%s/%s",dir,p->d_name);//拼接
			read_dir(h,buf);
		}
		if(p->d_type==DT_REG)//读到普通文件
		{
			chose_bmp(h,p->d_name,dir);//选择图片
			//printf(" %s********************\n",p->d_name);
		}
		
		
	}
	closedir(fd);
	
} */
//启动界面
void start_init()
{
	wchar_t *buf[]={L"多媒体播放器",L"正在启动中哦",L"加载好啦！！！"};
	int i=0;
	for(i=0;i<3;i++)
	{
		drw(800,480,0,0,0xffaec9);
	
		Lcd_Show_FreeType(lcd,buf[i],64,0xffffff,1,0xffaec9,90,100);
		sleep(1);
	}
	
}

//线程
int tid_init()
{
	pthread_t thread=0;
	pthread_create(&thread,NULL,touch_tid,NULL);//传递给任务函数的参数
}



//多媒体播放器
int main (int argc,char *argv[])
{
	

	//打开LCD设备
	int fd=open("/dev/fb0",O_RDWR);
	if(fd<0)
	{
		perror("");
		return -1;
	}
	get_lcd_info();
	lcd = mmap(NULL,800*480*4,PROT_READ|PROT_WRITE,MAP_SHARED,fd,0);
	// start_init();//启动界面
	
	touch_music_media();
	tid_init();
	while(1);
	
	
	
	
	close(fd);
}