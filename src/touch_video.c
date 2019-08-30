/*
	遍历视频目录并显示LCD屏幕上
	编译命令：arm-linux-gcc touch_video.c lcd_app.c -o touch_video -I./include -I./include/freetype2 -L./lib -lpthread -ljpeg -lfreetype

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
#include "example.c"

int *lcd=NULL;
int aaa=0,ii=0,jj=0;

struct doublelist *pp   = NULL;

struct doublelist *head = NULL;

//创建结构体
struct doublelist
{
	struct doublelist *prev;
	struct doublelist *next;
	char   movie[1024];
	char   data[1024];
};
//创建节点
struct doublelist *new_point()
{
	struct doublelist *new=(struct doublelist *)malloc(sizeof(struct doublelist));
	if(new==NULL)
	{
		perror("new is NULL");
		return NULL;
	}
	new->prev=new;
	new->next=new;
	return new;
}
//尾插
int tail_point(struct doublelist *h,char *name,char *movie_name)
{
	if(h==NULL)
	{
		printf("tail_point fail\n");
		return-1;
	}
	struct doublelist *new=new_point();
	struct doublelist *p=h;
	while(p->next!=h)
	{
		p=p->next;
	}
	p->next=new;
	new->next=h;
	
	new->prev=p;
	h->prev=new;
	
	strcpy(new->data,name);
	strcpy(new->movie,movie_name);
	return 1;
	
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
//查找需要的播放的jpg名对应的视频
char * movie_name_init(struct doublelist *h,char *name_data)
{
	struct doublelist *p=h->next;
	// printf("name_data===%s\n",name_data);
	if(name_data!=NULL)//查找需要的播放的jpg名对应的视频
	{
		while(p!=h)
		{
			if(strcmp(p->data,name_data)==0)
			{
				return p->movie;
			}
			p=p->next;
		}
	}
}
//遍历链表
void show_doublelist(struct doublelist *h,struct doublelist *ret,int k)
{
	int i=0;
	struct doublelist *tmp=ret;
	drw(800,480,0,0,0x99d9ea);//背景 

	if(k==0)  // 0 上一页
	{
		for(i=0;i<4;i++)
		{
			if(ret==h)
			{
				break;
			}
			else
			{
				ret=ret->prev;
			}
			
		}
		if(i==4)
		{
			pp=ret;
			tmp=ret;
		}
		
	}
	
	else if(k==1)
	{
		for(i=0;i<4;i++)
		{
			if(ret==h)
			{
				break;
			}
			else
			{
				ret=ret->next;
			}
			
		}
		if(i==4)
		{
			pp=ret;
			tmp=ret;
		}
		
	}

		
		for(i=0;i<4;i++)
		{
			
			if(tmp==h)
			{
				break;
			}
			if(i==0)
			{
				read_JPEG_file (tmp->data,90,90);
				
			}
			if(i==1)
			{
				read_JPEG_file (tmp->data,420,90);
				
			}
			if(i==2)
			{
				read_JPEG_file (tmp->data,90,280);
				
			}
			if(i==3)
			{
				read_JPEG_file (tmp->data,420,280);
				
			}
			tmp=tmp->next;
			
		}	
	
}

//处理目录下的文件选择.jpg
void chose_bmp(struct doublelist *h,char *names,char *gen)
{
	char buf[256]={0};
	char buf2[256]={0};
	char buf3[256]={0};
	char buf4[256]={0};
	
	sprintf(buf,"%s/%s",gen,names);
	if(strcmp((buf+strlen(buf)-4),".avi")==0)
	{
		aaa++;
		sprintf(buf2,"mplayer -ss 100 -noframedrop -nosound  -nolirc -nojoystick  -zoom -x 300 -y 150 -vo jpeg -frames 1 %s",buf);
		sprintf(buf3,"mv 00000001.jpg %d.jpg ",aaa);
		sprintf(buf4,"%d.jpg",aaa);
		printf("buf=%s\n",buf);
		system(buf2);
		system(buf3);
		tail_point(h,buf4,buf);//放入链表
		
	}
	if(strcmp((buf+strlen(buf)-4),".mp4")==0)
	{
		aaa++;
		sprintf(buf2,"mplayer -ss 100 -noframedrop -nosound  -nolirc -nojoystick  -zoom -x 300 -y 150 -vo jpeg -frames 1 %s",buf);
		sprintf(buf3,"mv 00000001.jpg %d.jpg ",aaa);
		sprintf(buf4,"%d.jpg",aaa);
		printf("buf=%s\n",buf);
		system(buf2);
		system(buf3);
		tail_point(h,buf4,buf);//放入链表
		
	}
}

//遍历目录
int read_dir(struct doublelist *h,char *dir)
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
			
		}
		
		
	}
	closedir(fd);
	
}

//线程  触摸线程
void *touch_tid(void *arg)
{
	
	// show_doublelist *h=head;
	struct input_event ts;
	int tsfd = open("/dev/input/event0",O_RDWR);
	if(tsfd < 0)
	{
		perror("");
		return;
	}	
	int x=0,y=0,xx=0,yy=0;
	
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
		if(ts.type == EV_KEY  && ts.code == BTN_TOUCH && ts.value == 1)
		{
			xx = x;
			yy = y;
		}
		if(ts.type == EV_KEY && ts.code == BTN_TOUCH && ts.value == 0)
		{
			char movie[256]={0};
			char name_jpg[256]={0};
			if(xx - x < -100)//上一页
			{ 
				printf("上一页\n");
				show_doublelist(head,pp,0);
				ii=ii-4;
				x=0;y=0,xx=0,yy=0;
			}
						
			if(xx - x > 100)//下一页
			{
				printf("下一页\n");
				
				show_doublelist(head,pp,1);
				ii=ii+4;
				x=0;y=0,xx=0,yy=0;
			}
			
			if(x>90&&y>90&&x<390&&y<240)
			{
				//printf("白\n");
				if((ii+1)>aaa)continue;
				else
					{
					jj=1;
					sprintf(name_jpg,"%d.jpg",1+ii);
					sprintf(movie,"./play %s",movie_name_init(head,name_jpg));
					printf("touch name_jpg %s \n",name_jpg);
					system(movie);
					
					
					drw(800,480,0,0,0x99d9ea);//背景 
					show_doublelist(head,pp,2);
					// 
					jj=0;
					}
				
				
			}
			
			if(x>420&&y>90&&x<720&&y<240)
			{
				//printf("红\n");
				if((ii+2)>aaa)continue;
				else
					{
					jj=1;
					sprintf(name_jpg,"%d.jpg",2+ii);
					sprintf(movie,"./play %s",movie_name_init(head,name_jpg));
					printf("touch name_jpg %s \n",name_jpg);
					system(movie);
					drw(800,480,0,0,0x99d9ea);//背景 
					
					show_doublelist(head,pp,2);
					
					jj=0;
					}
				

			}
			if(x>90&&y>280&&x<390&&y<430)
			{
				// printf("绿\n");
				if((ii+3)>aaa)continue;
				else
					{
					jj=1;
					sprintf(name_jpg,"%d.jpg",3+ii);
					sprintf(movie,"./play %s",movie_name_init(head,name_jpg));
					printf("touch name_jpg %s \n",name_jpg);
					system(movie);
					drw(800,480,0,0,0x99d9ea);//背景 
					
					show_doublelist(head,pp,2);


					jj=0;
					}
			}
			if(x>420&&y>280&&x<720&&y<430)
			{
				// printf("蓝\n");
				if((ii+4)>aaa)continue;
				else
					{
					jj=1;
					sprintf(name_jpg,"%d.jpg",4+ii);
					sprintf(movie,"./play %s",movie_name_init(head,name_jpg));
					printf("touch name_jpg %s \n",name_jpg);
					system(movie);
					drw(800,480,0,0,0x99d9ea);//背景 
					
					show_doublelist(head,pp,2);
					
					jj=0;
					}
				
				
			}
			
			
			x=0;y=0,xx=0,yy=0;
		}
	
	
	}
	close(tsfd);
	

}
//字体线程
void *show_top(void *top)
{
	get_lcd_info();//宽字符
	
	while(1)
	{
		if(jj==0)
		{
		int hd=0,dh=19;
		while(dh--)
		{
		//usleep(800);
			if(hd>28)//字体向左移动
			{
				while(1)
				{
					if(hd==0)break;
					if(jj==0)
					{
					hd-=4;
					usleep(10000);
					Lcd_Show_FreeType(lcd,L"多媒体播放器",64,0x99d9ea,1,0x99d9ea,70+hd+4,70);//白字
					Lcd_Show_FreeType(lcd,L"多媒体播放器",64,0x000000,0,0,70+hd,70);//黑字
					}		
				}	
			}	
			else//向右移动
			{		
					if(jj==0)
					{
					hd+=4;
					usleep(10000);
					Lcd_Show_FreeType(lcd,L"多媒体播放器",64,0x99d9ea,1,0x99d9ea,70+hd-4,70);//白字
					Lcd_Show_FreeType(lcd,L"多媒体播放器",64,0x000000,0,0,70+hd,70);//黑字
					}
			}	
		}

		}		
	}
	
	
	
}


//线程篮
int tid_init()
{
	pthread_t thread=0;
	pthread_t thread2=0;
	pthread_create(&thread2,NULL,show_top,NULL);
	pthread_create(&thread,NULL,touch_tid,NULL);//传递给任务函数的参数
}

int main (int argc,char *argv[])
{
	int a=150,b=300;
	if(argc!=2)
	{
		perror("xxx  xxx");
		return -1;
	}	
	//打开显示屏
	int fd = open("/dev/fb0",O_RDWR);
	if(fd<0)
	{
		perror("fb0");
		return -1;
	}
	
	lcd=mmap(NULL,800*480*4,PROT_READ|PROT_WRITE,MAP_SHARED,fd,0);//内存映射
	head=new_point();
	
	tid_init();//	触摸线程
	read_dir(head,argv[1]);	
	pp=head->next;
	drw(800,480,0,0,0x99d9ea);//背景 

	/* drw(b,a,90,90,0xffffff);//白
	drw(b,a,420,90,0xff0000);//红
	drw(b,a,90,280,0x00ff00);//绿
	drw(b,a,420,280,0x0000ff);//蓝 
	read_JPEG_file ("./00000001.jpg" ,90,100); */
	
	show_doublelist(head,pp,2);
	printf("aaa=%d\n",aaa);
	while(1)
	{
		if(ii<0)
		{
			ii=0;
		}
		 if(ii>=aaa)
		{
			ii=aaa-(aaa%4);
		} 
		
		
	}
	
}