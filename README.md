# ARM-VIDEO多媒体播放器  
执行文件为  
touch_video  
play  

如果报错则需要加 /  
  
./touch_video ./  

编译命令：  
touch_video.c 的arm-Linux-gcc编译  
arm-linux-gcc touch_video.c lcd_app.c -o touch_video -I./include -I./include/freetype2 -L./lib -lpthread -ljpeg -lfreetype  

paly.c 的arm-Linux-gcc编译  
arm-linux-gcc play.c -o play -lpthread  
视频在video目录下  

如果打开失败则是没有创建有名管道  则需要在根目录下创建两个管道  
mkfifo pipe  
mkfifo pipe2  
如果显示段错误或者是无法显示字体 需要加载字体库simsun.ttc放入到 执行文件   的文件目录！！！  