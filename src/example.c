
#include <stdio.h>
#include "jpeglib.h"
#include <setjmp.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>

extern JSAMPLE * image_buffer;	/* Points to large array of R,G,B-order data */
extern int image_height;	/* Number of rows in image */
extern int image_width;		/* Number of columns in image */

//外部全局变量
extern int *lcd;

//出错对象结构体
struct my_error_mgr {
  struct jpeg_error_mgr pub;	/* "public" fields */

  jmp_buf setjmp_buffer;	/* for return to caller */
};

typedef struct my_error_mgr * my_error_ptr;

//处理出错信息
METHODDEF(void)
my_error_exit (j_common_ptr cinfo)
{
  /* cinfo->err really points to a my_error_mgr struct, so coerce pointer */
  my_error_ptr myerr = (my_error_ptr) cinfo->err;

  /* Always display the message. */
  /* We could postpone this until after returning, if we chose. */
  (*cinfo->err->output_message) (cinfo);

  /* Return control to the setjmp point */
  longjmp(myerr->setjmp_buffer, 1);
}

//解码jpeg文件成功返回值1  失败返回0
GLOBAL(int)
read_JPEG_file (char * filename ,int px,int py)
{
   //定义jpeg解码对象结构体
  struct jpeg_decompress_struct cinfo;
 
  //定义jpeg出错对象
  struct my_error_mgr jerr;
  /* More stuff */
  FILE * infile;		/*源文件*/
  JSAMPARRAY buffer;		/*输出缓存区*/
  int row_stride;		/*输出缓冲区的宽度*/


		//打开需要解码的jpeg文件
  if ((infile = fopen(filename, "rb")) == NULL) {
    fprintf(stderr, "can't open %s\n", filename);
    return 0;
  }

	
	//初始化jpeg出错对象
  cinfo.err = jpeg_std_error(&jerr.pub);
  jerr.pub.error_exit = my_error_exit;

  if (setjmp(jerr.setjmp_buffer)) {
    jpeg_destroy_decompress(&cinfo);
    fclose(infile);
    return 0;
  }
  
  /*初始化jpeg的解码对象*/
  jpeg_create_decompress(&cinfo);

  /*把解码对象与源文件关联起来*/
  jpeg_stdio_src(&cinfo, infile);

  /*读取jpeg文件头，获取图像的长度和宽度 */
  (void) jpeg_read_header(&cinfo, TRUE);
 
   //开始解码
  (void) jpeg_start_decompress(&cinfo);
  
  //求出一行的像素的字节数
  row_stride = cinfo.output_width * cinfo.output_components;
  //分配堆空间
  buffer = (*cinfo.mem->alloc_sarray)
		((j_common_ptr) &cinfo, JPOOL_IMAGE, row_stride, 1);

		//每次解码只解一行
		unsigned char a=0;
		unsigned char r=0;
		unsigned char g=0;
		unsigned char b=0;
		unsigned int color=0;
		int  x=0,y=0;
	
	
		//printf("\n");
		printf("%s\n",filename);
		printf("jpg_kuan = %d jpg_gao=%d\n",cinfo.output_width,cinfo.output_height);
		//printf("jpg_gao  = %d\n",cinfo.output_height);
		//printf("jpg_lensize  = %d\n",cinfo.output_components*8);
		//printf("\n");
		
		
		
  while (cinfo.output_scanline < cinfo.output_height) {
	  
	  y = cinfo.output_scanline;
	  
		//读取一行解码后的数据放进缓冲区
    (void) jpeg_read_scanlines(&cinfo, buffer, 1);
    /* Assume put_scanline_someplace wants a pointer and sample count. */
    //put_scanline_someplace(buffer[0], row_stride);
	//解码后的像素数据是存放在buffer[0]中的  
	//重点 重点 重点！！！
	char  *p = buffer[0];  
	
		for(x=0;x<cinfo.output_width;x++)
		{
			//取像素点 
			if(cinfo.output_components == 4)
			{
				a = *p++;
			}
			else 
			{
				a = 0; 
			}
			
			r = *p++;
			g = *p++;
			b = *p++;
			//合并成32位色  
			color  = a << 24 | r << 16 | g << 8 | b;
			//大图满屏显示
			if(cinfo.output_width > 800 || cinfo.output_height > 480)
			{
				if(y < 480 && x < 800 && x > 0 && y >0)
				{
					//显示到lcd设备中
					*(lcd + y*800 + x) = color;
				}
				
			}
			else  //中间显示
			{		//求出中点坐标
					// int  ymid =  (480-cinfo.output_height)/2;
					// int  xmid =  (800-cinfo.output_width)/2;
					
					int tmpy = py+y;
					int tmpx = px+x;
					//防止越界
				if(tmpy < 480 && tmpx < 800 && tmpx > 0 && tmpy >0)
				{
					//显示到lcd设备中
					*(lcd + tmpy*800 + tmpx) = color;
				}
					
				
			}
			
			
			
		}
  }


	//解码结束  收尾处理
  (void) jpeg_finish_decompress(&cinfo);
  jpeg_destroy_decompress(&cinfo);
  fclose(infile);

  return 1;
}

