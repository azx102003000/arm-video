// 编译命令 ：arm-linux-gcc play.c -o play -lpthread


#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <string.h>
#include <sys/types.h>
#include <linux/input.h>
int *lcd =NULL;

int aa=0;

//画矩形函数
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

//线程执行
void *func(void *zifu)
{
	//创建触摸屏设备
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
			//音量加
			if(x<147&&y<480&&x>0&&y>430)
			{
				printf("音量加\n");
				
				system("echo volume +1 > /pipe");
			}
			//快退
			if(x<280&&y<480&&x>155&&y>430)
			{
				printf("快退\n");
				if(aa==1)
				{
					aa=0;
					system("echo seek -10 > /pipe");
					system("echo pause > /pipe ");
					aa=1;
					
				}
				else
				{
					system("echo seek -10 > /pipe");
				}
				
			}
			//播放/暂停
			if(x<400&&y<480&&x>285&&y>430)
			{
				printf("播放/暂停\n");
				aa++;
				if(aa==1)//暂停
				{
					system("echo pause > /pipe ");
				}
				if(aa>1)//播放
				{
					system("echo pause > /pipe ");
					aa=0;
				}
					
				
			}
			//快进
			if(x<537&&y<480&&x>400&&y>430)
			{
				printf("快进\n");
				if(aa==1)
				{
					aa=0;
					system("echo seek +10 > /pipe");
					system("echo pause > /pipe ");
					aa=1;
				}
				else
				{
					system("echo seek +10 > /pipe");
				}
			}
			//音量减
			if(x<680&&y<480&&x>537&&y>430)
			{
				printf("音量减\n");
				system("echo volume -1 > /pipe");
			}
			//返回
			if(x<800&&y<480&&x>680&&y>430)
			{
				printf("返回\n");
				system("killall mplayer");
				system("killall play");
				
			}
			x=0,y=0;
		}
		 
	}
}



	


//显示bmp	
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
	
	printf("chang=%d\tkuan=%d\n",chang,kuan);
	
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

//创建线程
int xiancheng()
{
	pthread_t tid=0;
	pthread_create(&tid,NULL,func,NULL);

}
	
int main(int argc,char *argv[])
{
	if(argc!=2)
	{
		perror("./xxx xxx");
		return -1;
	}
	 //对LCD设备进行映射
	int  fd = open("/dev/fb0",O_RDWR);	
	if(fd < 0)
	{
		printf("open lcd fail\n");
	}
	
	//对LCD设备进行映射操作
    lcd = mmap(NULL,800*480*4,PROT_READ|PROT_WRITE,MAP_SHARED,fd,0);
	//刷黑操作
	drw(800,480,0,0,0x000000);
	
	//控制界面
	show_bmp("./photos/ctl.bmp",0,430);

	
	xiancheng();
	
	//创建一个进程播放视屏 
	int pid = fork();
		if(pid == 0)
		{
			char tmp[256]={0};
			sprintf(tmp,"mplayer -slave -quiet -input  file=/pipe -zoom -x 800 -y 420  %s",argv[1]);
			int pd = open("/pipe2",O_RDWR);
				if(pd < 0)
				{
					perror("open fail\n");
					exit(0);
				}
			//文件重定向
			dup2(pd,1); 
			
			//播放视频
			printf("正在播放\n");
			system(tmp);
			printf("播放结束\n");
			return;
		}
	
		if(pid > 0)
		{
			int pd = open("/pipe2",O_RDWR);
			if(pd < 0)
			{
				perror("open fail\n");
				exit(0);
			}
			
			//不断写入获取百分比
			 while(1)
			{
				
				//写入信息到管道中
				if(aa!=1)//暂停
				{
					
					system("echo get_percent_pos > /pipe");
					sleep(1);

					//读取管道中的数据到buf中 
					char buf[1024]={0};
					read(pd,buf,1024);		
					//根据百分比绘制进度条
					int num=0;
					sscanf(buf,"ANS_PERCENT_POSITION=%d",&num);
					drw(num*8,10,0,420,0x0000ff); //绘制进度条  blue
					drw(800-(num*8),10,num*8,420,0xffffff);
					
					if(num==100)
					{
						system("killall mplayer");
						system("killall play");
					}
					 
				}
  
			} 
		}
	
	
	
	
}