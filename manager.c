#define GTK_ENABLE_BROKEN
#include<gtk/gtk.h>
#include<stdio.h>
#include"dirent.h"
#include"unistd.h"
#include"sys/file.h"
#include"fcntl.h"
#include"string.h"
#include"time.h"
#include"math.h"
#include<stdlib.h>
#include<sys/time.h>
#include<gdk/gdk.h>

#define PMAX     100 //最大进程数目
enum//进程界面的枚举类型
{
	NAME_COLUMN,//进程名列
	PID_COLUMN,//进程PID列
	STATUS_COLUMN,//进程状态列
	CPU_COLUMN,//CPU信息列
	MEMORY_COLUMN,//内存信息列
	NP_COLUMNS
};

enum//磁盘界面的枚举类型
{
	MAJOR_COLUMN,//主设备号列
	MINOR_COLUMN,//从设备号列
	BLOCK_COLUMN,//块大小列
	NAME2_COLUMN,//块名列
	NB_COLUMNS
};

enum//内存界面的枚举类型
{
	NAME3_COLUMN,//项目名列
	SIZE_COLUMN,//占用大小列
	KB_COLUMN,//单位列
	NM_COLUMNS
};

void getModInfo(char store[], int i, char modName[], char memUse[],char times[]);
void set_modul_info();

GtkWidget *cpu_draw_area;
GtkWidget *memory_draw_area;
GtkWidget *swap_draw_area;
GtkWidget *temp_widget;
GtkWidget * clist_modul;
float Cpu,MM,cpu_rate,mem_rate,swap_rate;

int USER,NICE,SYSTEM,IDLE;

char *title[6]={" History "," MEMORY "," PROCESS "," DISKINFO "," SYSINFO ","About"};

gdouble fuser = 0;//先前CPU使用信息
gdouble ftotal = 0;//先前CPU使用总量
gdouble total = 0;//当前进程总数
gint s_total = 0;
gint r_total = 0;
gdouble pfuser[PMAX];
gdouble rate = 0;//cpu 使用率

GtkWidget *window;//主窗口
GtkWidget *notebook;//notebook控件
GtkWidget *vbox;
GtkWidget *hbox;
GtkWidget *temp_frame;
GtkWidget *temp_frame1;
GtkWidget *temp_label;

GtkWidget *label;
GtkWidget *mem_bar;
GtkWidget *cpu_bar;
GtkWidget *swap_bar;

GtkWidget *info_label;
GString   *info;
GtkWidget *status_bar0;
GtkWidget *mem_label;
GtkWidget *swap_label;

GtkWidget *scrolled_window;

GtkListStore *process_store;
GtkListStore *disk_store;
GtkListStore *mem_store;


GtkWidget *ptree_view;
GtkWidget *btree_view;
GtkWidget *mtree_view;

GtkCellRenderer *renderer;//用于显示tree view中的每个列标题
GtkCellRenderer *drenderer;
GtkCellRenderer *mrenderer;
GtkTreeViewColumn *column;//tree view控件列数
GtkTreeViewColumn *dcolumn;
GtkTreeViewColumn *mcolumn;
GtkWidget *prefresh_button,*pdelete_button; //刷新进程按钮和结束进程按钮
GtkWidget *main_vbox;


GtkWidget *table;

GtkWidget *table1[6];//定义表格

GtkWidget *frame[6];//定义框架

GtkWidget *frame0;

GtkWidget *button[6],*cpu_rate_label,*process_label,*mem_rate_label,*swap_rate_label,*time_label,*s_label,*r_label,*uptime_label;//定义标签

gint length;

GtkWidget *CreateMenuItem(GtkWidget *,char *);//创建菜单项函数

GtkWidget *CreateMenuFile(GtkWidget *);//创建菜单File函数

GtkWidget *CreateMenuShutDown(GtkWidget *);//创建菜单Shutdown函数

GtkWidget *CreateMenuHelp(GtkWidget *);//创建菜单Help函数

void show_dialog (gchar *, gchar *);
void get_cpu_info (GString *);
void get_os_info (GString *);
void draw_cpu_load_graph (void);
void draw_mem_load_graph (void);

gint cpu_rate_ava(void);//计算CPU使用率

gint mem_rate_ava(void);//计算内存使用率

gint swap_rate_ava(void);//计算交换区使用率

gint uptime(void);//计算系统开机时间和运行时间

gint process_num(void);//计算进程数

gint mem_timeout(void);//

gint pro_timeout(void);//

gint sys_time(void);//计算系统时间

void get_process_info (GtkListStore *);
void get_disk_info (GtkListStore *);
void prefresh_button_clicked (gpointer data);
void prefresh_button_clicked2 (gpointer data);
void cpu_refresh(gpointer);
void pdelete_button_clicked (gpointer data);
gboolean get_cpu_rate (gpointer data);

void notebook_cpu_init(void);//cpu页面初始化
void notebook_mem_init(void);//内存页面初始化
void notebook_pro_init(void);//进程页面初始化
void notebook_disk_init(void);//磁盘页面初始化
void notebook_sys_init(void);//系统页面初始化
void notebook_history_init(void);//history页面初始化
void notebook_modules_init(GtkWidget* notebook);//modules

int select_name(char name[]){
	int i;
	for(i=0;name[i]!=0;i++)
		if(isalpha(name[i])||name[i]=='.')
		return 0;
	return 1;
}

/*回调函数，退出窗口管理程序*/
void delete_event(GtkWidget *window,gpointer data){
	gtk_main_quit();
}

/*回调函数，重启*/
void restart(GtkWidget *window,gpointer data){
	system("reboot");
}

/*回调函数，注销*/
void logout(GtkWidget *window,gpointer data){
	system("logout");
}

/*回调函数，关机*/
void shutdown(GtkWidget *window,gpointer data){
	system("halt");
}

/*输出帮助里的“关于”信息*/
void aboutSysMo(GtkWidget *window,gpointer data){
	GtkWidget *dialog,*label,*label2,*dtable,*text;//变量声明

	dialog=gtk_dialog_new();//新建一个dialog
	gtk_widget_set_usize(dialog, 300, 100);//设置dialog大小
    gtk_window_set_title(GTK_WINDOW(dialog),"About Me");//设置窗口标题
	gtk_container_set_border_width(GTK_CONTAINER(dialog),5);//设置窗口边框宽度

	dtable=gtk_table_new(11,10,TRUE);//创建表格11行*10列
	gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dialog)->vbox),dtable, TRUE, TRUE, 0);//将表格加入dialog
	gtk_widget_show_all(dtable);

	label=gtk_label_new("");//新建一个标题label
	gtk_table_attach_defaults(GTK_TABLE(dtable),label,0,10,0,1);//将label加入表格
	gtk_widget_show (label);

	label2=gtk_label_new("U2011114374   Godric  Gryffindor");//新建一个内容label
	gtk_table_attach_defaults(GTK_TABLE(dtable),label2,0,10,1,11);//将label加入表格
	gtk_widget_show (label2);
	gtk_widget_show(dialog);
}

/*主函数*/
int main(int argc,char *argv[]){
	gint i;
	int v,v0;
	int timer;
	GtkWidget *label2,*label;
	GtkWidget *label0[6];
	GtkWidget *text;
	gpointer date,data;
	gdouble value1,value2;
	gpointer gdata;
	GtkWidget *label3;
	GtkWidget *button1;
	GtkWidget *vscrollbar,*hscrollbar;//定义滚动条
	GtkWidget *MenuBar;//定义菜单栏
	GtkWidget *MenuItemFile,*MenuItemShutDown,*MenuItemHelp;


    gtk_set_locale ();
	gtk_init(&argc,&argv);//在任何构件生成之前完成
 	memset (pfuser, 0 ,sizeof (pfuser));

	window=gtk_window_new(GTK_WINDOW_TOPLEVEL);//创建主窗口
	gtk_window_set_title(GTK_WINDOW(window),"System Monitor");//设置窗口标题
	gtk_widget_set_usize(window, 600, 500);//设置窗口大小
	gtk_window_set_resizable (GTK_WINDOW (window), TRUE);// 窗口大小可改变（TRUE）
	gtk_container_set_border_width(GTK_CONTAINER(window),5);//设置窗口边框宽度
	gtk_widget_show(window);

	table=gtk_table_new(12,12,TRUE);//创建表格12行*12列
	gtk_widget_show(table);
	gtk_container_add(GTK_CONTAINER(window),table);//将table装进窗口

	notebook=gtk_notebook_new();//创建notebook
	gtk_notebook_set_tab_pos(GTK_NOTEBOOK(notebook),GTK_POS_LEFT);//设置notebook的格式
	gtk_table_attach_defaults (GTK_TABLE (table), notebook, 0, 12, 0, 11);//将notebook加入表格中
	gtk_widget_show(notebook);
	/*创建notebook的每一页*/
	for(i=0;i<6;i++){
		label0[i]=gtk_label_new(title[i]);//notebook页面标题
		frame[i]=gtk_frame_new(NULL);//页面框架
		gtk_container_set_border_width (GTK_CONTAINER (frame[i]), 10);//框架与边缘的距离
		gtk_frame_set_shadow_type (GTK_FRAME (frame[i]),GTK_SHADOW_ETCHED_OUT);//框架的阴影模式
		gtk_widget_set_size_request(frame[i],450,450);//框架大小
		gtk_widget_show(frame[i]);
		gtk_widget_show (label0[i]);
		gtk_notebook_append_page(GTK_NOTEBOOK(notebook),frame[i],label0[i]);//创建notebook的第i页
		table1[i]=gtk_table_new(12,12,TRUE);//创建表格12行*12列
		gtk_widget_show(table1[i]);
		gtk_container_add(GTK_CONTAINER(frame[i]),table1[i]);//将表格tbale1放入框架
	}


	MenuBar=gtk_menu_bar_new();//创建菜单栏
	gtk_table_attach_defaults(GTK_TABLE(table),MenuBar,0,11,0,1);//将菜单栏加入表格
	MenuItemFile=CreateMenuItem(MenuBar,"File");//增加菜单项
  	CreateMenuFile(MenuItemFile);//调用创建根菜单项函数
  	MenuItemShutDown=CreateMenuItem(MenuBar,"Options");//增加菜单项
	CreateMenuShutDown(MenuItemShutDown);//调用创建根菜单项函数
    MenuItemHelp=CreateMenuItem(MenuBar,"Help");//增加菜单项
    CreateMenuHelp(MenuItemHelp);/*调用创建根菜单项函数*/
   	gtk_widget_show(MenuBar);

    time_label=gtk_label_new(" ");//显示系统时间
    temp_frame=gtk_frame_new(NULL);//页面框架
    temp_frame1=gtk_frame_new(NULL);//页面框架
    timer=gtk_timeout_add(1000,(GtkFunction)sys_time,data);//加一个定时器(timer),每隔１秒调用一次(GtkFunction)stys_time函数
    gtk_container_set_border_width (GTK_CONTAINER (temp_frame), 3);//框架与边缘的距离
    gtk_container_set_border_width (GTK_CONTAINER (temp_frame1), 3);//框架与边缘的距离
	gtk_table_attach_defaults(GTK_TABLE(table),temp_frame,9,12,11,12);//将时间标签装入table
    gtk_container_add(GTK_CONTAINER(temp_frame),temp_frame1);//将表格tbale1放入框架
    gtk_container_add(GTK_CONTAINER(temp_frame1),time_label);//将表格tbale1放入框架
	gtk_widget_show_all(temp_frame);


        uptime_label=gtk_label_new("");//
    	timer=gtk_timeout_add(1000,(GtkFunction)uptime,data);//加一个定时器(timer),每隔１秒调用一次函数
    	gtk_widget_show(uptime_label);//显示标签

     	process_label=gtk_label_new("");//创建显示进程数的标签
        gtk_widget_show(process_label);
    	timer=gtk_timeout_add(1000,(GtkFunction)process_num,data);//加一个定时器(timer),每隔１秒调用一次(GtkFunction)process_num函数
        gtk_table_attach_defaults(GTK_TABLE(table),process_label,0,2,11,12);//在状态栏中显示进程总数

    	cpu_rate_label=gtk_label_new(" ");//创建显示cpu占用率的标签
        gtk_widget_show(cpu_rate_label);
    	timer=gtk_timeout_add(1000,(GtkFunction)cpu_rate_ava,data);//加一个定时器(timer),每隔１秒调用一次(GtkFunction)cpu_rate_ava函数
        gtk_table_attach_defaults(GTK_TABLE(table),cpu_rate_label,2,4,11,12);//在状态栏中显示cpu占用率

    	mem_rate_label=gtk_label_new(NULL);//创建显示内存占用率的标签
     	gtk_widget_show(mem_rate_label);
   	timer=gtk_timeout_add(1000,(GtkFunction)mem_rate_ava,data);//加一个定时器(timer),每隔１秒调用一次(GtkFunction)mem_rate_ava函数
   	gtk_table_attach_defaults(GTK_TABLE(table),mem_rate_label,4,7,11,12);//在状态栏中显示内存占用率

 	swap_rate_label=gtk_label_new(NULL);//创建显示交换区占用率的标签
     	gtk_widget_show(swap_rate_label);
        timer=gtk_timeout_add(1000,(GtkFunction)swap_rate_ava,data);//加一个定时器(timer),每隔１秒调用一次(GtkFunction)swap_rate_ava函数
   	gtk_table_attach_defaults(GTK_TABLE(table),swap_rate_label,7,9,11,12);//在状态栏中显示交换区占用率

	cpu_bar=gtk_progress_bar_new();//创建CPU进度条
  	mem_bar=gtk_progress_bar_new();//创建内存进度条
	swap_bar=gtk_progress_bar_new();//创建交换区进度条
	gtk_widget_show(cpu_bar);
	gtk_widget_show(mem_bar);
	gtk_widget_show(swap_bar);

    notebook_history_init();//history页面初始化
	notebook_mem_init();//内存页面初始化
	notebook_pro_init();//进程页面初始化
	notebook_disk_init();//磁盘页面初始化
	notebook_sys_init();//系统页面初始化
    notebook_cpu_init();
    notebook_modules_init(notebook);//modules页面初始化

	timer = gtk_timeout_add (1000, (GtkFunction)get_cpu_rate, NULL);

   	gtk_signal_connect(GTK_OBJECT(window),"delete_event",GTK_SIGNAL_FUNC(delete_event),NULL);

 	gtk_main();

}

/*创建菜单项*/
GtkWidget *CreateMenuItem(GtkWidget *MenuBar,char *test){
    GtkWidget *MenuItem;
    MenuItem=gtk_menu_item_new_with_label(test);//创建根菜单项
   	gtk_menu_shell_append(GTK_MENU_SHELL(MenuBar),MenuItem);//把根菜单和菜单栏连在一起
  	gtk_widget_show(MenuItem);//显示根菜单
    return MenuItem;
}

/*创建文件菜单*/
GtkWidget *CreateMenuFile(GtkWidget *MenuItem){
     	GtkWidget *Menu;//定义根菜单
    	GtkWidget *Exit;
        Menu=gtk_menu_new();//创建菜单
        Exit=CreateMenuItem(Menu,"EXIT");//调用创建根菜单函数
    	gtk_signal_connect(GTK_OBJECT(Exit),"activate",GTK_SIGNAL_FUNC(delete_event),NULL);//关于delete_event的回调函数
    	gtk_menu_item_set_submenu(GTK_MENU_ITEM(MenuItem),Menu);//把根菜单项和根菜单绑定起来
    	gtk_widget_show(Menu);
}

/*创建关机选项菜单*/
GtkWidget *CreateMenuShutDown(GtkWidget *MenuItem){
     	GtkWidget *Menu;//定义菜单根
     	GtkWidget *Restart,*ShutDown,*LogOut;
    	Menu=gtk_menu_new();//创建根菜单
   	ShutDown=CreateMenuItem(Menu,"Shut Down");//调用创建根菜单函数
    	LogOut=CreateMenuItem(Menu,"Log Out");
    	Restart=CreateMenuItem(Menu,"Reboot");
    	gtk_signal_connect(GTK_OBJECT(Restart),"activate",GTK_SIGNAL_FUNC(restart),NULL);
    	gtk_signal_connect(GTK_OBJECT(LogOut),"activate",GTK_SIGNAL_FUNC(logout),NULL);
    	gtk_signal_connect(GTK_OBJECT(ShutDown),"activate",GTK_SIGNAL_FUNC(shutdown),NULL);
  	gtk_menu_item_set_submenu(GTK_MENU_ITEM(MenuItem),Menu);//把父菜单项于子菜单联系起来
    	gtk_widget_show(Menu);
}

/*创建帮助菜单*/
GtkWidget *CreateMenuHelp(GtkWidget *MenuItem){
    	GtkWidget *Menu;//定义子菜单
    	GtkWidget *AboutGtk;
    	Menu=gtk_menu_new();//创建子菜单
        AboutGtk=CreateMenuItem(Menu,"About Me");//调用创建菜单项函数
    	gtk_signal_connect(GTK_OBJECT(AboutGtk),"activate",GTK_SIGNAL_FUNC(aboutSysMo),NULL);
     	gtk_menu_item_set_submenu(GTK_MENU_ITEM(MenuItem),Menu);//把根菜单项和菜单项绑定起来
    	gtk_widget_show(Menu);
}

/*计算系统时间*/
gint sys_time(void){
     	time_t timep;
     	struct tm *p;
   	char buf[30]="Time:",temp[5];
    	time(&timep);
    	p=localtime(&timep); //取得当地时间
    	sprintf(temp,"%d",p->tm_hour);
	strcat(buf,temp);
    	sprintf(temp,"%2d",p->tm_min);
    	strcat(buf,":");
     	strcat(buf,temp);
     	sprintf(temp,"%2d",p->tm_sec);
    	strcat(buf,":");
   	strcat(buf,temp);
   	gtk_label_set_text(GTK_LABEL(time_label),buf);//改变标签的内容
    	return 1;
}

/*计算cpu占用率*/
gint cpu_rate_ava(void){
     	char buffer[1024*5];
	char *match;
   	FILE *fp;
  	size_t bytes;
   	char show[5];
  	char text_cpu[14]="CPU : ";
   	int user,nice,system,idle;

    	fp = fopen("/proc/stat","r");//打开/proc/stat文件
   	if (fp == 0){
		printf("open /proc/stat file error\n");
  		return 0;
	}
  	bytes = fread(buffer, 1, sizeof(buffer),fp);//读取所需信息
  	fclose(fp);
   	if (bytes == 0 || bytes == sizeof(buffer)){
  	printf("readBytes NULL OR large than buffer \n");
    	return 0;
    	}
   	buffer[bytes] = '\0';
   	match = strstr (buffer, "cpu");
   	if (match == NULL){
   	printf("match error\n");
    	return 0;
    	}
    	sscanf (match ,"cpu %d %d %d %d", &user, &nice, &system, &idle);
   	cpu_rate=100*(user-USER+nice-NICE+system-SYSTEM)/(float)(user-USER+nice-NICE+system-SYSTEM+idle-IDLE);//计算cpu的占用率
	USER=user;//保存本次的使用量
	NICE=nice;
	SYSTEM=system;
	IDLE=idle;
    	sprintf(show,"%.3f",cpu_rate);
   	strcat(text_cpu,show);
   	strcat(text_cpu,"%");
    	gtk_label_set_text(GTK_LABEL(cpu_rate_label),text_cpu);//将结果写入标签
  	gtk_progress_bar_set_fraction (GTK_PROGRESS_BAR (cpu_bar),cpu_rate/100);//将结果在进度条上显示
  	gtk_progress_bar_set_text(GTK_PROGRESS_BAR(cpu_bar),text_cpu);//将结果在进度条的文本上显示
}

/*计算内存占用率*/
gint mem_rate_ava(void){
    	int mem_fd;
    	int MemFree,MemTotal,MemUsed,i=0,j=0;
    	char buf[1000],buf1[1000]=" ",buf2[100]=" ",buf3[100]=" ",buf4[100]=" ",string[20];
    	char *p;
    	char Mem[100]=" ";

   	mem_fd=open("/proc/meminfo",O_RDONLY);//打开/proc/meminfo文件
   	read(mem_fd,buf,1000);//读取文件信息
   	close(mem_fd);
   	strcat(buf1,strtok(buf,"\n"));
    	strcat(buf2,buf1);
    	for(i=0;i<100&&buf2[i]!='\0';i++)
  		if(buf2[i]>='0'&&buf2[i]<='9')buf3[j++]=buf2[i];
     	buf3[j]='\0';
     	MemTotal=atoi(buf3);//读取内存总量
    	strcat(buf1,strtok(NULL,"\n"));
     	strcpy(buf2,buf1);
     	j=0;
    	for(;i<100&&buf2[i]!='\0';i++)
		if(buf2[i]>='0'&&buf2[i]<='9')buf4[j++]=buf2[i];
	buf4[j]='\0';
     	MemFree=atoi(buf4);//读取内存空闲量
    	MemUsed=MemTotal-MemFree;//计算内存使用量
   	MemTotal=MemTotal/1024;
   	MemUsed=MemUsed/1024;
    	strcpy(Mem,"Memory: ");
  	mem_rate=100*(MemUsed/(float) MemTotal);//计算内存使用率
    	sprintf(string,"%.2f",mem_rate);
   	strcat(Mem,string);
	strcat(Mem,"%");
   	gtk_label_set_text(GTK_LABEL(mem_rate_label),Mem);//将结果写入标签
	gtk_progress_bar_set_fraction (GTK_PROGRESS_BAR (mem_bar),mem_rate/100);//将结果在进度条上显示
  	gtk_progress_bar_set_text(GTK_PROGRESS_BAR(mem_bar),Mem);//将结果在进度条的文本上显示
}

/*计算交换区使用率*/
gint swap_rate_ava(void){
       	int swap_fd;
	gchar *swap_info[20];
       	int SwapFree,SwapTotal,i,swap,swap0;
       	char buf[1024*5];
       	char show[5];char s[6],s0[6];
       	char *match,*match0;
       	gchar *temp1,*temp2;
       	gchar *delim="kB";
	swap_info[0] = strtok (buf, delim);
       	char text_swap[16]="Swap:";
       	char SWap[100]=" ";

      	swap_fd=open("/proc/meminfo",O_RDONLY);//打开/proc/meminfo文件
       	read(swap_fd,buf,sizeof (buf));//读取文件信息
       	close(swap_fd);
       	for(i=1;i<20;i++){
		swap_info[i]=strtok(NULL,delim);
       	}
       	temp1=strstr(swap_info[17],":");
       	temp1+=9;
       	swap_info[17]=temp1;


        temp2=strstr(swap_info[18],":");
       	temp2+=10;
       	swap_info[18]=temp2;
        SwapTotal=atoi(swap_info[17]);
        SwapFree=atoi(swap_info[17]);
        swap_rate=100*(1-(float)(SwapTotal/SwapFree));//计算交换区使用率
        sprintf(show,"%.2f",swap_rate);
        strcat(text_swap,show);
        strcat(text_swap,"%");
        gtk_label_set_text(GTK_LABEL(swap_rate_label),text_swap);//将结果写入标签
	gtk_progress_bar_set_fraction (GTK_PROGRESS_BAR (swap_bar),swap_rate);//将结果在进度条上显示
	gtk_progress_bar_set_text(GTK_PROGRESS_BAR(swap_bar),text_swap);//将结果在进度条的文本上显示
}

/*计算系统启动时间和系统运行时间*/
gint uptime(void){
	int ut_fd,now_time,run_time,start_time,h,m,s;
	char buf[30],*ch,buffer[20],tp[5],start[100]="System Started Time:";
	int f;
	time_t timep;
     	struct tm *p;
	gchar *delim=" ";

	time(&timep);
    	p=localtime(&timep);//得到本地时间
	now_time=p->tm_hour*3600+p->tm_min*60+p->tm_sec;//将本地时间转化为秒
	ut_fd=open("/proc/uptime",O_RDONLY);//打开/proc/uptime文件
	read(ut_fd,buf,sizeof (buf));//读取文件
	close(ut_fd);
	ch=strtok(buf,delim);strtok(NULL,delim);strtok(NULL,delim);gtk_label_set_text(GTK_LABEL(uptime_label),ch);
	run_time=atoi(ch);//计算出系统运行的秒数
	start_time=now_time-run_time;//计算系统启动的时间
	if(start_time<0){
		start_time=24*3600+start_time;
	}
	h=start_time/3600;//系统启动时间，转化为时分秒
	m=(start_time-h*3600)/60;
	s=start_time-h*3600-m*60;
	sprintf(buffer,"%d",h);
	strcat(buffer,":");
	sprintf(tp,"%2d",m);
 	strcat(buffer,tp);
	strcat(buffer,":");
	sprintf(tp,"%2d",s);
 	strcat(buffer,tp);
	strcat(start,buffer);
	strcat(start,"\nSystem Running Time:");
	h=run_time/3600;//系统运行时间，转化为时分秒
	m=(run_time-h*3600)/60;
	s=run_time-h*3600-m*60;
	sprintf(tp,"%d",h);
	strcat(start,tp);
	strcat(start,":");
	sprintf(tp,"%2d",m);
	strcat(start,tp);
	strcat(start,":");
	sprintf(tp,"%2d",s);
	strcat(start,tp);
	gtk_label_set_text(GTK_LABEL(uptime_label),start);//将结果写入标签
	//return 1;
}

/*实时显示进程总数*/
gint process_num(void){
     	DIR * dir;
    	struct dirent * ptr;
    	char show[5];
     	int total=0;
    	char text_value[100]="Process:  ";

   	dir =opendir("/proc");
   	while((ptr = readdir(dir))!=NULL){
		if(select_name(ptr->d_name)){//d_name:文件名
		total++;
		}
	}
 	sprintf(show,"%3d",total);
   	strcat(text_value,show);//追加show中的内容到text_value中
  	closedir(dir);
    	gtk_label_set_text(GTK_LABEL(process_label),text_value);//将结果写入标签
}

//CPU 图表函数
    static int timer;
    gint cpu_expose_event (gpointer da)
    {
    GtkWidget * drawing_area = (GtkWidget *)da;
    GdkDrawable * drawable = drawing_area->window;

    GdkGC *gc_chart, *gc_text;
    GdkColor color;

    static unsigned coory[476];
    int da_width, da_height;
    int x;
    float load;
    char buf[30];

    // set chart green
    gc_chart = gdk_gc_new (drawing_area->window);
    color.red = 0;
    color.green = 0xffff;
    color.blue = 0;
    gdk_gc_set_rgb_fg_color(gc_chart, &color );
    // set text red
    gc_text = gdk_gc_new (drawing_area->window);
    color.red = 0xffff;
    color.green = 0;
    color.blue = 0;
    gdk_gc_set_rgb_fg_color(gc_text, &color );
  // getting size of drawing area
    da_width = drawing_area->allocation.width;
    da_height = drawing_area->allocation.height;
    gdk_draw_rectangle (drawable, drawing_area->style->black_gc,
    TRUE,
    0,0,
    da_width,
    da_height);

    // chart cpu load
    for (x=0; x<475; x++)
    coory[x] = coory[x+1];

    load = cpu_rate/100;
    coory[x] = (int)(load * da_height);

    for(x=0;x<da_width;x++){
    gdk_draw_line (drawable, gc_chart, x, da_height, x, da_height - coory[x+1]);
    }

    // following code for drawing text
    sprintf (buf, "Cpu load: %.1f%%", load * 100);

    PangoLayout *layout = gtk_widget_create_pango_layout( da, buf );
    PangoFontDescription *fontdesc = pango_font_description_from_string( "Luxi Mono 12" );
    pango_layout_set_font_description( layout, fontdesc );
    gdk_draw_layout( drawable,
    gc_text,
    5, 5, layout );
    pango_font_description_free( fontdesc );
    g_object_unref( layout );

    g_object_unref( G_OBJECT(gc_chart) );
    g_object_unref( G_OBJECT(gc_text) );

    return TRUE;
    }
/*SOURCES页面初始化*/
void notebook_cpu_init(void){
  	float v,v0;
  	char text[20];char text0[20];
  	gfloat value1,value2;

  	vbox = gtk_vbox_new (FALSE, 0);//添加纵向盒子
  	gtk_widget_show (vbox);
  	gtk_table_attach_defaults(GTK_TABLE(table1[5]),vbox,0,12,0,12);

  	frame0 = gtk_frame_new ("CPU");//添加用于显示CPU信息的边框构件
  	gtk_widget_show (frame0);
  	gtk_box_pack_start (GTK_BOX (vbox), frame0, FALSE, FALSE, 8);//加到盒子中
  	gtk_container_add (GTK_CONTAINER (frame0),cpu_bar);

  	frame0 = gtk_frame_new ("Memory");//添加用于显示内存信息的边框构件
  	gtk_widget_show (frame0);
  	gtk_box_pack_start (GTK_BOX (vbox), frame0, FALSE, FALSE, 8);
  	gtk_container_add (GTK_CONTAINER (frame0),mem_bar);

  	frame0 = gtk_frame_new ("Swap");//添加用于显示swap信息的边框构件
  	gtk_widget_show (frame0);
  	gtk_box_pack_start (GTK_BOX (vbox), frame0, FALSE, FALSE, 8);
  	gtk_container_add (GTK_CONTAINER (frame0),swap_bar);

  	frame0 = gtk_frame_new ("UpTime");//添加用于显示系统时间信息的边框构件
  	gtk_widget_show (frame0);
  	gtk_box_pack_start (GTK_BOX (vbox), frame0, FALSE, FALSE, 8);
  	gtk_container_add (GTK_CONTAINER (frame0),uptime_label);


    frame0 = gtk_frame_new ("CPU History");//添加用于显示系统时间信息的边框构件
  	gtk_widget_show (frame0);
  	gtk_box_pack_start (GTK_BOX (vbox), frame0, FALSE, FALSE, 8);

/* 现在,我们开始向屏幕绘图。
	 * 我们使用的构件是绘图区构件。
	 *一个绘图区构件本质上是一个 X 窗口,没有其它的东西。
	 *它是一个空白的画布,我们可以在其上绘制需要的东西。*/
	cpu_draw_area = gtk_drawing_area_new();
	/* 设置可以画图 */
	gtk_widget_set_app_paintable(cpu_draw_area, TRUE);

	/* 画图区默认大小 */
	gtk_drawing_area_size(GTK_DRAWING_AREA(cpu_draw_area), 500, 100);
	/* 画布添加到cpu曲线框架中 */
	gtk_container_add(GTK_CONTAINER(frame0), cpu_draw_area);
	gtk_widget_show(cpu_draw_area);
	/* 关联回调函数 */
	/* 关于各种事件不甚了解 */
	g_signal_connect(cpu_draw_area, "expose_event",
			G_CALLBACK(cpu_expose_event), NULL);
    gtk_widget_show (cpu_draw_area);
    gtk_widget_show(window);
    timer = gtk_timeout_add (1000, cpu_expose_event, (gpointer) cpu_draw_area);
}

//memory 图表函数
    gint memory_expose_event (gpointer da)
    {
    GtkWidget * drawing_area = (GtkWidget *)da;
    GdkDrawable * drawable = drawing_area->window;

    GdkGC *gc_chart, *gc_text;
    GdkColor color;

    static unsigned coory[476];
    int da_width, da_height;
    int x;
    float load;
    char buf[30];

    // set chart green
    gc_chart = gdk_gc_new (drawing_area->window);
    color.red = 0;
    color.green = 0xffff;
    color.blue = 0;
    gdk_gc_set_rgb_fg_color(gc_chart, &color );
    // set text red
    gc_text = gdk_gc_new (drawing_area->window);
    color.red = 0xffff;
    color.green = 0;
    color.blue = 0;
    gdk_gc_set_rgb_fg_color(gc_text, &color );
  // getting size of drawing area
    da_width = drawing_area->allocation.width;
    da_height = drawing_area->allocation.height;
    gdk_draw_rectangle (drawable, drawing_area->style->black_gc,
    TRUE,
    0,0,
    da_width,
    da_height);

    // chart cpu load
    for (x=0; x<475; x++)
    coory[x] = coory[x+1];

    load = mem_rate/100;
    coory[x] = (int)(load * da_height);

    for(x=0;x<da_width;x++){
    gdk_draw_line (drawable, gc_chart, x, da_height, x, da_height - coory[x+1]);
    }

    // following code for drawing text
    sprintf (buf, "Memory load: %.1f%%", load * 100);

    PangoLayout *layout = gtk_widget_create_pango_layout( da, buf );
    PangoFontDescription *fontdesc = pango_font_description_from_string( "Luxi Mono 12" );
    pango_layout_set_font_description( layout, fontdesc );
    gdk_draw_layout( drawable,
    gc_text,
    5, 5, layout );
    pango_font_description_free( fontdesc );
    g_object_unref( layout );

    g_object_unref( G_OBJECT(gc_chart) );
    g_object_unref( G_OBJECT(gc_text) );

    return TRUE;
    }

//swap 图表函数
    gint swap_expose_event (gpointer da)
    {
    GtkWidget * drawing_area = (GtkWidget *)da;
    GdkDrawable * drawable = drawing_area->window;

    GdkGC *gc_chart, *gc_text;
    GdkColor color;

    static unsigned coory[476];
    int da_width, da_height;
    int x;
    float load;
    char buf[30];

    // set chart green
    gc_chart = gdk_gc_new (drawing_area->window);
    color.red = 0;
    color.green = 0xffff;
    color.blue = 0;
    gdk_gc_set_rgb_fg_color(gc_chart, &color );
    // set text red
    gc_text = gdk_gc_new (drawing_area->window);
    color.red = 0xffff;
    color.green = 0;
    color.blue = 0;
    gdk_gc_set_rgb_fg_color(gc_text, &color );
  // getting size of drawing area
    da_width = drawing_area->allocation.width;
    da_height = drawing_area->allocation.height;
    gdk_draw_rectangle (drawable, drawing_area->style->black_gc,
    TRUE,
    0,0,
    da_width,
    da_height);

    // chart cpu load
    for (x=0; x<475; x++)
    coory[x] = coory[x+1];

    load = swap_rate/100;
    coory[x] = (int)(load * da_height);

    for(x=0;x<da_width;x++){
    gdk_draw_line (drawable, gc_chart, x, da_height, x, da_height - coory[x+1]);
    }

    // following code for drawing text
    sprintf (buf, "Swap load: %.1f%%", load * 100);

    PangoLayout *layout = gtk_widget_create_pango_layout( da, buf );
    PangoFontDescription *fontdesc = pango_font_description_from_string( "Luxi Mono 12" );
    pango_layout_set_font_description( layout, fontdesc );
    gdk_draw_layout( drawable,
    gc_text,
    5, 5, layout );
    pango_font_description_free( fontdesc );
    g_object_unref( layout );

    g_object_unref( G_OBJECT(gc_chart) );
    g_object_unref( G_OBJECT(gc_text) );

    return TRUE;
    }

void notebook_history_init(void)//history页面初始化
{
    float v,v0;
  	char text[20];char text0[20];
  	gfloat value1,value2;

  	vbox = gtk_vbox_new (FALSE, 0);//添加纵向盒子
  	gtk_widget_show (vbox);
  	gtk_table_attach_defaults(GTK_TABLE(table1[0]),vbox,0,12,0,12);


    frame0 = gtk_frame_new ("CPU History");//添加用于显示cpu history信息的边框构件
  	gtk_widget_show (frame0);
  	gtk_box_pack_start (GTK_BOX (vbox), frame0, FALSE, FALSE, 8);


/* 现在,我们开始向屏幕绘图。
	 * 我们使用的构件是绘图区构件。
	 *一个绘图区构件本质上是一个 X 窗口,没有其它的东西。
	 *它是一个空白的画布,我们可以在其上绘制需要的东西。*/
	cpu_draw_area = gtk_drawing_area_new();
	/* 设置可以画图 */
	gtk_widget_set_app_paintable(cpu_draw_area, TRUE);

	/* 画图区默认大小 */
	gtk_drawing_area_size(GTK_DRAWING_AREA(cpu_draw_area), 500, 100);
	/* 画布添加到cpu曲线框架中 */
	gtk_container_add(GTK_CONTAINER(frame0), cpu_draw_area);
	gtk_widget_show(cpu_draw_area);
	/* 关联回调函数 */
	/* 关于各种事件不甚了解 */
	g_signal_connect(cpu_draw_area, "expose_event",
			G_CALLBACK(cpu_expose_event), NULL);
    gtk_widget_show (cpu_draw_area);
    gtk_widget_show(window);
    timer = gtk_timeout_add (1000, cpu_expose_event, (gpointer) cpu_draw_area);

    frame0 = gtk_frame_new ("Memory History");//添加用于显示memory history信息的边框构件
  	gtk_widget_show (frame0);
  	gtk_box_pack_start (GTK_BOX (vbox), frame0, FALSE, FALSE, 8);

    /* 现在,我们开始向屏幕绘图。
	 * 我们使用的构件是绘图区构件。
	 *一个绘图区构件本质上是一个 X 窗口,没有其它的东西。
	 *它是一个空白的画布,我们可以在其上绘制需要的东西。*/
	memory_draw_area = gtk_drawing_area_new();
	/* 设置可以画图 */
	gtk_widget_set_app_paintable(memory_draw_area, TRUE);

	/* 画图区默认大小 */
	gtk_drawing_area_size(GTK_DRAWING_AREA(memory_draw_area), 500, 100);
	/* 画布添加到cpu曲线框架中 */
	gtk_container_add(GTK_CONTAINER(frame0), memory_draw_area);
	gtk_widget_show(memory_draw_area);
	/* 关联回调函数 */
	/* 关于各种事件不甚了解 */
	g_signal_connect(memory_draw_area, "expose_event",
			G_CALLBACK(memory_expose_event), NULL);
    gtk_widget_show (memory_draw_area);
    gtk_widget_show(window);
    timer = gtk_timeout_add (1000, memory_expose_event, (gpointer) memory_draw_area);

    frame0 = gtk_frame_new ("SWAP History");//添加用于显示swap history信息的边框构件
  	gtk_widget_show (frame0);
  	gtk_box_pack_start (GTK_BOX (vbox), frame0, FALSE, FALSE, 8);


/* 现在,我们开始向屏幕绘图。
	 * 我们使用的构件是绘图区构件。
	 *一个绘图区构件本质上是一个 X 窗口,没有其它的东西。
	 *它是一个空白的画布,我们可以在其上绘制需要的东西。*/
	swap_draw_area = gtk_drawing_area_new();
	/* 设置可以画图 */
	gtk_widget_set_app_paintable(swap_draw_area, TRUE);

	/* 画图区默认大小 */
	gtk_drawing_area_size(GTK_DRAWING_AREA(swap_draw_area), 500, 100);
	/* 画布添加到cpu曲线框架中 */
	gtk_container_add(GTK_CONTAINER(frame0), swap_draw_area);
	gtk_widget_show(swap_draw_area);
	/* 关联回调函数 */
	/* 关于各种事件不甚了解 */
	g_signal_connect(swap_draw_area, "expose_event",
			G_CALLBACK(swap_expose_event), NULL);
    gtk_widget_show (swap_draw_area);
    gtk_widget_show(window);
    timer = gtk_timeout_add (1000, swap_expose_event, (gpointer) swap_draw_area);
}
/*内存页面初始化*/
void notebook_mem_init(void){
  	int i;
  	gchar *col_name[3] = { "Project", "Size"," "};
  	vbox = gtk_vbox_new (FALSE, 0);//添加纵向盒子
  	gtk_widget_show (vbox);
  	gtk_table_attach_defaults(GTK_TABLE(table1[1]),vbox,0,12,0,12);

  	scrolled_window = gtk_scrolled_window_new (NULL, NULL);//添加滚动窗口控件
  	gtk_widget_set_size_request (scrolled_window, 300, 300);
  	gtk_widget_show (scrolled_window);
  	gtk_box_pack_start (GTK_BOX (vbox), scrolled_window, TRUE, TRUE, 0);


	//进程信息
 	 mem_store = gtk_list_store_new (NM_COLUMNS,
					G_TYPE_STRING,
					G_TYPE_STRING,
					G_TYPE_STRING);

  	mtree_view = gtk_tree_view_new_with_model (GTK_TREE_MODEL (mem_store));
  	//添加tree view控件，显示进程信息树

  	g_object_unref (G_OBJECT (mem_store));//减少引用次数


  	gtk_widget_show (mtree_view);
  	gtk_container_add (GTK_CONTAINER (scrolled_window), mtree_view);//将进程信息树加到滚动窗口中


  	for (i = 0; i < 3; i++) {
		mrenderer = gtk_cell_renderer_text_new ();//添加一个cell_renderer_text用于显示文字
  		mcolumn = gtk_tree_view_column_new_with_attributes (col_name[i],mrenderer,"text",i,NULL);//新建一列
	    	gtk_tree_view_append_column (GTK_TREE_VIEW (mtree_view), mcolumn);//将该列加到树中
  	}


  	get_mem_info(mem_store); //获取进程信息并存取显示到列表


  	hbox = gtk_hbox_new (FALSE, 0);//添加横排列盒子
  	gtk_widget_show (hbox);
  	gtk_box_pack_start (GTK_BOX (vbox), hbox, TRUE, FALSE, 0);//增加到竖排列盒子中

  	prefresh_button = gtk_button_new ();//创建刷新进程信息按钮
  	gtk_widget_show (prefresh_button);
  	gtk_widget_set_size_request (prefresh_button, 70, 30);//设置刷新按钮大小
  	gtk_button_set_label (GTK_BUTTON (prefresh_button), "refresh");//设置刷新按钮上显示的文字
  	g_signal_connect (G_OBJECT (prefresh_button),"clicked",G_CALLBACK(prefresh_button_clicked2),NULL);//刷新按钮点击后执行prefresh_button_clicked
  	gtk_box_pack_start (GTK_BOX (hbox), prefresh_button, TRUE, FALSE, 0);//增加该按钮到横排列盒子中
}

/*显示内存信息的回调函数*/
gint get_mem_info(GtkListStore *store){
        int mem_fd,i=1,j,m;
   	char buf[1000],buf1[1000]=" ";
    	char *p,*c[50],*ch[3];
    	gint sign;
	GtkTreeIter iter;
    	gpointer gdata;

     	mem_fd=open("/proc/meminfo",O_RDONLY);//打开/proc/meminfo文件
    	read(mem_fd,buf,1000);//读取文件信息
    	close(mem_fd);
    	c[0]=strtok(buf,"\n");
   	while(p=strtok(NULL,"\n")){
		c[i]=p;
		i++;
   	}
	for(j=0,m=1;j<i;j++,m=1){
		ch[0] = strtok(c[j]," ");
		while(p=strtok(NULL," ")){
			ch[m]=p;
			m++;
		}
		gtk_list_store_append (store, &iter);//增加到列表
		gtk_list_store_set (store, &iter,
					NAME3_COLUMN,ch[0],
					SIZE_COLUMN,ch[1],
					KB_COLUMN,ch[2],
					-1);
	}
     	return 1;
}

/*进程页面初始化*/
void notebook_pro_init(void){
	int i;
  	gchar *col_name[5] = { "Project Name", "PID" , "Status", "CPU %" , "Memory used"};

  	vbox = gtk_vbox_new (FALSE, 0);//添加纵向盒子
  	gtk_widget_show (vbox);
  	gtk_table_attach_defaults(GTK_TABLE(table1[2]),vbox,0,12,0,10);

  	scrolled_window = gtk_scrolled_window_new (NULL, NULL);//添加滚动窗口控件
  	gtk_widget_set_size_request (scrolled_window, 300, 300);
  	gtk_widget_show (scrolled_window);
  	gtk_box_pack_start (GTK_BOX (vbox), scrolled_window, TRUE, TRUE, 0);

	//进程信息
  	process_store = gtk_list_store_new (NP_COLUMNS,
					G_TYPE_STRING,
					G_TYPE_STRING,
					G_TYPE_STRING,
					G_TYPE_STRING,
					G_TYPE_STRING);

  	ptree_view = gtk_tree_view_new_with_model (GTK_TREE_MODEL (process_store));
  	//添加tree view控件，显示进程信息树

  	g_object_unref (G_OBJECT (process_store));//减少引用次数


  	gtk_widget_show (ptree_view);
  	gtk_container_add (GTK_CONTAINER (scrolled_window), ptree_view);//将进程信息树加到滚动窗口中


  	for (i = 0; i < 5; i++) {
		renderer = gtk_cell_renderer_text_new ();//添加一个cell_renderer_text用于显示文字
  		column = gtk_tree_view_column_new_with_attributes (col_name[i],renderer,"text",i,NULL);//新建一列
	    	gtk_tree_view_append_column (GTK_TREE_VIEW (ptree_view), column);//将该列加到树中
  	}

  	hbox = gtk_hbox_new (FALSE, 0);//添加横排列盒子
  	gtk_widget_show (hbox);
  	gtk_box_pack_start (GTK_BOX (vbox), hbox, TRUE, FALSE, 0);//增加到竖排列盒子中

  	prefresh_button = gtk_button_new ();//创建刷新进程信息按钮
  	gtk_widget_show (prefresh_button);
  	gtk_widget_set_size_request (prefresh_button, 60, 30);//设置刷新按钮大小
  	gtk_button_set_label (GTK_BUTTON (prefresh_button), "refresh");//设置刷新按钮上显示的文字
  	g_signal_connect (G_OBJECT (prefresh_button),"clicked",G_CALLBACK(prefresh_button_clicked),NULL);//刷新按钮点击后执行prefresh_button_clicked
  	gtk_table_attach_defaults(GTK_TABLE(table1[2]), prefresh_button,2,4,10,11);

  	pdelete_button = gtk_button_new ();//创建杀死进程按钮
  	gtk_widget_show (pdelete_button);
  	gtk_widget_set_size_request (pdelete_button, 60, 30);
  	gtk_button_set_label (GTK_BUTTON (pdelete_button), "kill");
  	g_signal_connect (G_OBJECT (pdelete_button),"clicked",G_CALLBACK(pdelete_button_clicked),NULL);//该按钮点击后执行pdelete_button_clicked
	gtk_table_attach_defaults(GTK_TABLE(table1[2]), pdelete_button,7,9,10,11);

  	s_label=gtk_label_new(" ");
  	gtk_widget_show(s_label);//显示标签
  	gtk_table_attach_defaults(GTK_TABLE(table1[2]),s_label,1,5,11,12);
  	r_label=gtk_label_new(" ");
  	gtk_widget_show(r_label);//显示标签
  	gtk_table_attach_defaults(GTK_TABLE(table1[2]),r_label,6,10,11,12);
  	get_process_info (process_store);
  	gtk_list_store_clear (process_store);
  	get_process_info (process_store);
}

/*获得进程相关信息的函数*/
void get_process_info (GtkListStore *store){
  	DIR *dir;
  	struct dirent *entry;
  	int fd, i,num = 0,s,r;
  	gchar dir_buf[256];
  	gchar buffer[128];
  	gchar *info[26];
  	gchar *delim = " ";
  	gchar sl[20],rl[20];
  	GtkTreeIter iter;
  	gdouble pcuser[PMAX];
  	gdouble srate;
  	gchar rate_buffer[16];
  	gdouble mem;
  	gchar mem_buffer[16];

  	s_total = 0;
  	r_total = 0;
  	dir = opendir ("/proc");//打开/proc目录，dir为返回的指针
  	while ((entry = readdir (dir)) != NULL ) {
		if ((entry->d_name[0] >= '0') && (entry->d_name[0] <= '9')) {
			sprintf (dir_buf, "/proc/%s/stat", entry->d_name);//读取文件到dir_buf中
			fd = open (dir_buf, O_RDONLY);
			read (fd, buffer, sizeof (buffer));
			close (fd);

			info[0] =  strtok (buffer, delim);//以delim分隔符切割buffer的一部分到info［0］
			for (i = 1; i < 26 ; i++) {
				info[i] = strtok (NULL, delim);//查找整个buffer字符串
			}
			pcuser[num] = atoi (info[13]);//转换字符串为长整数
			srate = (pcuser[num]-pfuser[num]) / (2 * total);//cpu利用占总cpu使用的百分比
                	if(srate<0||srate>1){srate=0;}
			sprintf (rate_buffer, "%.2f%%",100 * srate);

			mem = atoi (info[22]);//该进程的内存使用量
			mem = mem / (1024 * 1024);
			sprintf (mem_buffer, "%-.2f MB",mem);

			gtk_list_store_append (store, &iter);//增加到列表
			gtk_list_store_set (store, &iter,
					NAME_COLUMN,info[1],
					PID_COLUMN,info[0],
					STATUS_COLUMN,info[2],
					CPU_COLUMN,rate_buffer,
					MEMORY_COLUMN,mem_buffer,
					-1);
			if(!strcmp(info[2],"S")) s_total++;
   			if(!strcmp(info[2],"R")) r_total++;
			pfuser[num] = pcuser[num];
  			num = (num + 1 ) % PMAX;
		}
	}
  	closedir (dir);
  	sprintf(sl,"Sleep Process:%d",s_total);
  	sprintf(rl,"Running Process:%d",r_total);
  	gtk_label_set_text(GTK_LABEL(s_label),sl);
  	gtk_label_set_text(GTK_LABEL(r_label),rl);
}

/*磁盘页面初始化*/
void notebook_disk_init(void){
  	int i;
  	gchar *col_name[4] = { "MAJOR", "MINOR" , "BLOCK", "NAME"};

  	vbox = gtk_vbox_new (FALSE, 0);//添加纵向盒子
  	gtk_widget_show (vbox);
  	gtk_table_attach_defaults(GTK_TABLE(table1[3]),vbox,0,12,0,12);

  	scrolled_window = gtk_scrolled_window_new (NULL, NULL);//添加滚动窗口控件
  	gtk_widget_set_size_request (scrolled_window, 300, 300);
  	gtk_widget_show (scrolled_window);
  	gtk_box_pack_start (GTK_BOX (vbox), scrolled_window, TRUE, TRUE, 0);


	//进程信息
  	disk_store = gtk_list_store_new (NB_COLUMNS,
					G_TYPE_STRING,
					G_TYPE_STRING,
					G_TYPE_STRING,
					G_TYPE_STRING);


  	btree_view = gtk_tree_view_new_with_model (GTK_TREE_MODEL (disk_store));
  	//添加tree view控件，显示进程信息树
  	g_object_unref (G_OBJECT (disk_store));//减少引用次数

  	gtk_widget_show (btree_view);
  	gtk_container_add (GTK_CONTAINER (scrolled_window), btree_view);//将进程信息树加到滚动窗口中
  	for (i = 0; i < 4; i++) {
		drenderer = gtk_cell_renderer_text_new ();//添加一个cell_renderer_text用于显示文字
  		dcolumn = gtk_tree_view_column_new_with_attributes (col_name[i],drenderer,"text",i,NULL);//新建一列
	    	gtk_tree_view_append_column (GTK_TREE_VIEW (btree_view), dcolumn);//将该列加到树中
  	}
  	get_disk_info(disk_store); //获取进程信息并存取显示到列表
}

/*获得磁盘相关信息的函数*/
void get_disk_info(GtkListStore *store){
	int disk_fd,i=0,j,m;
       	char buf[1000],buf1[1000]=" ";
       	char *p;
	char *c[8],*ch[4],buffer[16];
	GtkTreeIter iter;
	GtkWidget *text;;

	text=gtk_text_new(NULL,NULL);
       	disk_fd=open("/proc/partitions",O_RDONLY);//打开/proc/partitions文件
       	read(disk_fd,buf,1000);//读取文件信息
       	close(disk_fd);
       	strcat(buf1,strtok(buf,"\n"));
       	while(p=strtok(NULL,"\n")){
		strcat(buf1,"\n\n");
              	strcat(buf1,p);
		c[i] = p;
                i++;
       	}
	for(j=0,m=1;j<i-1;j++,m=1){
		ch[0] = strtok(c[j]," ");
		while(p=strtok(NULL," ")){
			ch[m]=p;
			m++;
		}
		gtk_list_store_append (store, &iter);//增加到列表
		gtk_list_store_set (store, &iter,
					MAJOR_COLUMN,ch[0],
					MINOR_COLUMN,ch[1],
					BLOCK_COLUMN,ch[2],
					NAME2_COLUMN,ch[3],
					-1);
	}
}

/*系统页面初始化*/
void notebook_sys_init(void){
  	vbox = gtk_vbox_new (FALSE, 0);//添加纵向盒子
  	gtk_widget_show (vbox);
  	gtk_table_attach_defaults(GTK_TABLE(table1[4]),vbox,0,12,0,12);

  	frame0 = gtk_frame_new ("CPU Info:");//显示CPU信息边框
  	gtk_widget_show (frame0);
  	gtk_box_pack_start (GTK_BOX (vbox), frame0, TRUE, TRUE, 10);//加到纵向盒子里

  	info = g_string_new ("");
  	get_cpu_info (info);//用 info变量存储获得的cpu信息

  	info_label = gtk_label_new (info->str);//用标签info_label显示info变量的字符串内容
  	gtk_widget_show (info_label);
  	gtk_container_add (GTK_CONTAINER(frame0), info_label);//将该标签加到边框中

  	frame0 = gtk_frame_new ("OS Info:");//显示操作系统信息边框
  	gtk_widget_show (frame0);
  	gtk_box_pack_start (GTK_BOX (vbox), frame0, TRUE, TRUE, 10);//加到纵向盒子里

  	info = g_string_new ("");
  	get_os_info (info);//用 info变量存储获得的操作系统信息

  	info_label = gtk_label_new (info->str);//用标签info_label显示info变量的字符串内容
  	gtk_widget_show (info_label);
  	gtk_container_add (GTK_CONTAINER(frame0), info_label);//将该标签加到新边框中
}

/*获得cpu信息*/
void get_cpu_info(GString *string){
  	int fd,i;
  	gchar buffer[1024];
  	gchar *delim = "\n";
  	gchar *cpu_info[17];
  	gchar *tmp;

  	fd = open ("/proc/cpuinfo", O_RDONLY);//读取cpu相关信息的文件
  	read (fd, buffer, sizeof (buffer));
  	close (fd);

  	cpu_info[0] = strtok (buffer, delim);
  	for (i=1; i < 17 ; i++) {
		cpu_info[i] = strtok (NULL,delim);//分割字符串
  	}
  	for (i=0; i < 17; i++) {
  	  	tmp = strstr (cpu_info[i], ":");//忽略冒号:
		tmp += 2;
        	cpu_info[i] = tmp;
 	}

  	g_string_append (string, "CPU Type       :");
  	g_string_append (string, cpu_info[4]);
  	g_string_append_c (string, '\n');
  	g_string_append (string, "CPU Frequency       :");
  	g_string_append (string, cpu_info[6]);
  	g_string_append_c (string, '\n');
  	g_string_append (string, "Cache Size    :");
  	g_string_append (string, cpu_info[7]);
  	g_string_append_c (string, '\n');
}

/*获得操作系统相关信息*/
void get_os_info (GString *string){
	int fd,i;
      	gchar buffer[128];
      	gchar *tmp,*start,*stop;
	gchar *os_info[8];
	gchar *delim = " ";

	fd = open ("/proc/version", O_RDONLY);//读取操作系统相关信息的文件
	read(fd, buffer, sizeof (buffer));

	start = buffer;
	stop = strstr (buffer, "#");
	stop--;
	stop--;
	os_info[0] = strtok (buffer, delim);
	for (i = 1; i < 8; i++) {
		os_info[i] = strtok (NULL, delim);//分割字符串

	}

	g_string_append (string, "OS     :");//显示相关信息
	g_string_append (string, os_info[0]);
	g_string_append_c (string, '\n');
	g_string_append (string, "Version       :");
      	g_string_append (string, os_info[2]);
      	g_string_append_c (string, '\n');
      	g_string_append (string, "PC Name       :");
      	g_string_append (string, os_info[3]);
  	g_string_append_c (string, '\n');
      	g_string_append (string, "Gcc Version   :");
  	g_string_append (string, os_info[6]);
      	g_string_append_c (string, '\n');
}

/*进程刷新按钮处理函数*/
void prefresh_button_clicked (gpointer data){//刷新按钮被点击，刷新进程相关信息
  	gtk_list_store_clear (process_store);
  	get_process_info (process_store);
}

/*内存刷新按钮处理函数*/
void prefresh_button_clicked2 (gpointer data){//刷新按钮被点击，刷新进程相关信息
  	gtk_list_store_clear (mem_store);
  	get_mem_info (mem_store);
}

/*进程杀死按钮处理函数*/
void pdelete_button_clicked (gpointer data){//kill按钮被点击，停止一个进程的运行
  	GtkTreeSelection *selection;
  	GtkTreeModel *model;
  	GtkTreeIter iter;
  	gchar *pid;
  	pid_t pid_num;

  	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(ptree_view));//获得当前选择的项
  	if (gtk_tree_selection_get_selected(selection, &model, &iter)) {
		gtk_tree_model_get (model, &iter, PID_COLUMN, &pid, -1);//在树的相应列中获得该进程的PID
  		pid_num = atoi (pid);//字符串转换成长整数

	    	if(kill (pid_num, SIGTERM) == -1 ) {//根据PID结束该进程
			gchar *title = "ERROR";
			gchar *content = "Termination Failed,Check UID";
			show_dialog (title, content);//结束进程失败信息输出
		}
	    	gtk_list_store_clear (process_store);//刷新进程信息
	    	get_process_info (process_store);
  	}
}

/*显示一个对话框，标题title，内容为content*/
void show_dialog (gchar *title, gchar *content){
	GtkWidget *dialog;
	GtkWidget *label;
        dialog = gtk_dialog_new_with_buttons (title,
                                         GTK_WINDOW (window),
                                         GTK_DIALOG_DESTROY_WITH_PARENT,
                                         GTK_STOCK_CLOSE,
                                         GTK_RESPONSE_NONE,
                                         NULL);//创建一个对话框构件
	gtk_window_set_resizable (GTK_WINDOW (dialog), FALSE);//不可改变大小
	g_signal_connect_swapped (dialog,
                             "response",
                             G_CALLBACK (gtk_widget_destroy),
                             dialog);//事件回调函数为退出

	label = gtk_label_new (content);//创建一个标签，显示content的内容
	gtk_widget_show (label);
	gtk_container_add (GTK_CONTAINER (GTK_DIALOG(dialog)->vbox),
                      label);
	gtk_widget_show (dialog);
}

/*获得cpu使用率*/
gboolean get_cpu_rate ( gpointer data){
  	int fd,i;
  	gchar buffer[256];
  	gchar *cpu_time[9];
  	gchar *tmp;
  	gchar *delim =  " ";
  	gdouble cuser;
  	gdouble ctotal;


  	fd = open ("/proc/stat", O_RDONLY);//读取相关文件
  	read (fd, buffer, sizeof (buffer));
  	close (fd);
  	tmp = strstr (buffer, "cpu0");//从开始到以"cpu0"为结束符的buffer的一部分字符到tmp
  	tmp--;
  	*tmp = '\0';
  	cpu_time[0] = strtok (buffer, delim);//通过字符串分割得到目的字符串
  	for (i = 1; i < 9 ; i++) {
		cpu_time[i] = strtok (NULL, delim);
  	}
  	cuser = atoi (cpu_time[1]);//将字符串转换为长整数
  	ctotal = (cuser + atoi (cpu_time[2]) + atoi (cpu_time[3]) + atoi (cpu_time[4]));
  	total = ctotal - ftotal;
  	rate = (cuser - fuser) / total;
  	fuser = cuser;
  	ftotal = ctotal;
  	return TRUE;
}
void notebook_modules_init(GtkWidget* notebook)
{
	GtkWidget *frame_up, *frame_down, *table, *align, *button;
	GtkWidget *frame = gtk_frame_new("MODULES");
	gtk_container_set_border_width(GTK_CONTAINER(frame), 10);
	gtk_widget_set_size_request(frame, 100, 355);
	gtk_widget_show(frame);

	gtk_notebook_append_page(GTK_NOTEBOOK(notebook), frame, gtk_label_new("MODULES"));

	/* 建一个表格，分为上下两部分 */
	table = gtk_table_new(10, 4, TRUE);
	/* 将这个表添加到框架 */
	gtk_container_add(GTK_CONTAINER(frame), table);

	/* 建一个frame在表上边*/
	frame_down = gtk_frame_new("");
	gtk_table_attach_defaults(GTK_TABLE(table), frame_down, 0, 4, 0, 10);
	gtk_widget_show(frame_down);
	gtk_widget_show(table);

	GtkWidget *scrolled_window = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_window),GTK_POLICY_AUTOMATIC, GTK_POLICY_ALWAYS);
	gtk_container_add(GTK_CONTAINER(frame_down), scrolled_window);
	gtk_widget_show(scrolled_window);

	/* 老是不能显示，经测试直到这里都可以正确实现 */
	gchar *titles[3] ={ "Name", "Memory Used", "Loaded Times" }; //由于含有特殊情况，所以只读取通//用格式的范例
	clist_modul = gtk_clist_new_with_titles(3, titles);
	gtk_clist_set_shadow_type(GTK_CLIST(clist_modul), GTK_SHADOW_IN);
	gtk_clist_set_column_width(GTK_CLIST(clist_modul), 0, 270);
	gtk_clist_set_column_width(GTK_CLIST(clist_modul), 1, 270);
	gtk_clist_set_column_width(GTK_CLIST(clist_modul), 2, 270);
	gtk_container_add(GTK_CONTAINER(scrolled_window), clist_modul);
	set_modul_info();
	gtk_widget_show(clist_modul);
}

void set_modul_info()
{
	char infoBuf[2000]; //暂存读取的modules文件内容
	int fd = open("/proc/modules", O_RDONLY);
	read(fd, infoBuf, sizeof(infoBuf));
	close(fd);
	unsigned int lines = 0;
	unsigned int i = 0;
	gtk_clist_clear((GtkCList*)clist_modul);
	/* 先取得行数 */
	while (i != sizeof(infoBuf) / sizeof(char))
	{
		if (infoBuf[i] == '\n')
			lines++;
		i++;
	}
	i = 0;
	for (i = 1; i <= lines; i++)
	{
		char convert_mem[25];
		char modName[25]; //模块名
		char memUse[20]; //内存量
		char times[5]; //使用次数
		int mem_num;
		float real_mem;
		getModInfo(infoBuf, i, modName, memUse, times);
		mem_num = atoi(memUse);
		real_mem = (float) mem_num / (1024);
		gcvt(real_mem, 3, convert_mem);
		gchar *list[1][3] ={{ modName, convert_mem, times } };
		gtk_clist_append((GtkCList*) clist_modul, list[0]);
		gtk_clist_thaw((GtkCList *) clist_modul); //更新list列表显示
	}
}
void getModInfo(char store[], int i, char modName[], char memUse[],char times[])
{
	int j = 0;
	int cflags = 0; //记录读取的回车键个数以便判断行数
	int k = 0;
	char name2[25];
	char mem2[20];
	char times2[5];
	while (cflags < i - 1)
	{
		if (store[j] == '\n')
			cflags++; //回车数加1
		j++;
	}
	while (store[j] != ' ')
	{ //读取进程名
		name2[k++] = store[j];
		j++;
	}
	name2[k] = '\0';
	j++; //跳转到下一个不是空格的地方
	k = 0;
	while (store[j] != ' ')
	{
		mem2[k++] = store[j];
		j++;
	}
	mem2[k] = '\0'; //封口
	j++;
	times2[0] = store[j]; //读取模块的使用次数
	times2[1] = '\0'; //封口
	strcpy(modName, name2);
	strcpy(memUse, mem2);
	strcpy(times, times2);
}
