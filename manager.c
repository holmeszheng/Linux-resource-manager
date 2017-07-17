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

#define PMAX     100 //��������Ŀ
enum//���̽����ö������
{
	NAME_COLUMN,//��������
	PID_COLUMN,//����PID��
	STATUS_COLUMN,//����״̬��
	CPU_COLUMN,//CPU��Ϣ��
	MEMORY_COLUMN,//�ڴ���Ϣ��
	NP_COLUMNS
};

enum//���̽����ö������
{
	MAJOR_COLUMN,//���豸����
	MINOR_COLUMN,//���豸����
	BLOCK_COLUMN,//���С��
	NAME2_COLUMN,//������
	NB_COLUMNS
};

enum//�ڴ�����ö������
{
	NAME3_COLUMN,//��Ŀ����
	SIZE_COLUMN,//ռ�ô�С��
	KB_COLUMN,//��λ��
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

gdouble fuser = 0;//��ǰCPUʹ����Ϣ
gdouble ftotal = 0;//��ǰCPUʹ������
gdouble total = 0;//��ǰ��������
gint s_total = 0;
gint r_total = 0;
gdouble pfuser[PMAX];
gdouble rate = 0;//cpu ʹ����

GtkWidget *window;//������
GtkWidget *notebook;//notebook�ؼ�
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

GtkCellRenderer *renderer;//������ʾtree view�е�ÿ���б���
GtkCellRenderer *drenderer;
GtkCellRenderer *mrenderer;
GtkTreeViewColumn *column;//tree view�ؼ�����
GtkTreeViewColumn *dcolumn;
GtkTreeViewColumn *mcolumn;
GtkWidget *prefresh_button,*pdelete_button; //ˢ�½��̰�ť�ͽ������̰�ť
GtkWidget *main_vbox;


GtkWidget *table;

GtkWidget *table1[6];//������

GtkWidget *frame[6];//������

GtkWidget *frame0;

GtkWidget *button[6],*cpu_rate_label,*process_label,*mem_rate_label,*swap_rate_label,*time_label,*s_label,*r_label,*uptime_label;//�����ǩ

gint length;

GtkWidget *CreateMenuItem(GtkWidget *,char *);//�����˵����

GtkWidget *CreateMenuFile(GtkWidget *);//�����˵�File����

GtkWidget *CreateMenuShutDown(GtkWidget *);//�����˵�Shutdown����

GtkWidget *CreateMenuHelp(GtkWidget *);//�����˵�Help����

void show_dialog (gchar *, gchar *);
void get_cpu_info (GString *);
void get_os_info (GString *);
void draw_cpu_load_graph (void);
void draw_mem_load_graph (void);

gint cpu_rate_ava(void);//����CPUʹ����

gint mem_rate_ava(void);//�����ڴ�ʹ����

gint swap_rate_ava(void);//���㽻����ʹ����

gint uptime(void);//����ϵͳ����ʱ�������ʱ��

gint process_num(void);//���������

gint mem_timeout(void);//

gint pro_timeout(void);//

gint sys_time(void);//����ϵͳʱ��

void get_process_info (GtkListStore *);
void get_disk_info (GtkListStore *);
void prefresh_button_clicked (gpointer data);
void prefresh_button_clicked2 (gpointer data);
void cpu_refresh(gpointer);
void pdelete_button_clicked (gpointer data);
gboolean get_cpu_rate (gpointer data);

void notebook_cpu_init(void);//cpuҳ���ʼ��
void notebook_mem_init(void);//�ڴ�ҳ���ʼ��
void notebook_pro_init(void);//����ҳ���ʼ��
void notebook_disk_init(void);//����ҳ���ʼ��
void notebook_sys_init(void);//ϵͳҳ���ʼ��
void notebook_history_init(void);//historyҳ���ʼ��
void notebook_modules_init(GtkWidget* notebook);//modules

int select_name(char name[]){
	int i;
	for(i=0;name[i]!=0;i++)
		if(isalpha(name[i])||name[i]=='.')
		return 0;
	return 1;
}

/*�ص��������˳����ڹ������*/
void delete_event(GtkWidget *window,gpointer data){
	gtk_main_quit();
}

/*�ص�����������*/
void restart(GtkWidget *window,gpointer data){
	system("reboot");
}

/*�ص�������ע��*/
void logout(GtkWidget *window,gpointer data){
	system("logout");
}

/*�ص��������ػ�*/
void shutdown(GtkWidget *window,gpointer data){
	system("halt");
}

/*���������ġ����ڡ���Ϣ*/
void aboutSysMo(GtkWidget *window,gpointer data){
	GtkWidget *dialog,*label,*label2,*dtable,*text;//��������

	dialog=gtk_dialog_new();//�½�һ��dialog
	gtk_widget_set_usize(dialog, 300, 100);//����dialog��С
    gtk_window_set_title(GTK_WINDOW(dialog),"About Me");//���ô��ڱ���
	gtk_container_set_border_width(GTK_CONTAINER(dialog),5);//���ô��ڱ߿���

	dtable=gtk_table_new(11,10,TRUE);//�������11��*10��
	gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dialog)->vbox),dtable, TRUE, TRUE, 0);//��������dialog
	gtk_widget_show_all(dtable);

	label=gtk_label_new("");//�½�һ������label
	gtk_table_attach_defaults(GTK_TABLE(dtable),label,0,10,0,1);//��label������
	gtk_widget_show (label);

	label2=gtk_label_new("U2011114374   Godric  Gryffindor");//�½�һ������label
	gtk_table_attach_defaults(GTK_TABLE(dtable),label2,0,10,1,11);//��label������
	gtk_widget_show (label2);
	gtk_widget_show(dialog);
}

/*������*/
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
	GtkWidget *vscrollbar,*hscrollbar;//���������
	GtkWidget *MenuBar;//����˵���
	GtkWidget *MenuItemFile,*MenuItemShutDown,*MenuItemHelp;


    gtk_set_locale ();
	gtk_init(&argc,&argv);//���κι�������֮ǰ���
 	memset (pfuser, 0 ,sizeof (pfuser));

	window=gtk_window_new(GTK_WINDOW_TOPLEVEL);//����������
	gtk_window_set_title(GTK_WINDOW(window),"System Monitor");//���ô��ڱ���
	gtk_widget_set_usize(window, 600, 500);//���ô��ڴ�С
	gtk_window_set_resizable (GTK_WINDOW (window), TRUE);// ���ڴ�С�ɸı䣨TRUE��
	gtk_container_set_border_width(GTK_CONTAINER(window),5);//���ô��ڱ߿���
	gtk_widget_show(window);

	table=gtk_table_new(12,12,TRUE);//�������12��*12��
	gtk_widget_show(table);
	gtk_container_add(GTK_CONTAINER(window),table);//��tableװ������

	notebook=gtk_notebook_new();//����notebook
	gtk_notebook_set_tab_pos(GTK_NOTEBOOK(notebook),GTK_POS_LEFT);//����notebook�ĸ�ʽ
	gtk_table_attach_defaults (GTK_TABLE (table), notebook, 0, 12, 0, 11);//��notebook��������
	gtk_widget_show(notebook);
	/*����notebook��ÿһҳ*/
	for(i=0;i<6;i++){
		label0[i]=gtk_label_new(title[i]);//notebookҳ�����
		frame[i]=gtk_frame_new(NULL);//ҳ����
		gtk_container_set_border_width (GTK_CONTAINER (frame[i]), 10);//������Ե�ľ���
		gtk_frame_set_shadow_type (GTK_FRAME (frame[i]),GTK_SHADOW_ETCHED_OUT);//��ܵ���Ӱģʽ
		gtk_widget_set_size_request(frame[i],450,450);//��ܴ�С
		gtk_widget_show(frame[i]);
		gtk_widget_show (label0[i]);
		gtk_notebook_append_page(GTK_NOTEBOOK(notebook),frame[i],label0[i]);//����notebook�ĵ�iҳ
		table1[i]=gtk_table_new(12,12,TRUE);//�������12��*12��
		gtk_widget_show(table1[i]);
		gtk_container_add(GTK_CONTAINER(frame[i]),table1[i]);//�����tbale1������
	}


	MenuBar=gtk_menu_bar_new();//�����˵���
	gtk_table_attach_defaults(GTK_TABLE(table),MenuBar,0,11,0,1);//���˵���������
	MenuItemFile=CreateMenuItem(MenuBar,"File");//���Ӳ˵���
  	CreateMenuFile(MenuItemFile);//���ô������˵����
  	MenuItemShutDown=CreateMenuItem(MenuBar,"Options");//���Ӳ˵���
	CreateMenuShutDown(MenuItemShutDown);//���ô������˵����
    MenuItemHelp=CreateMenuItem(MenuBar,"Help");//���Ӳ˵���
    CreateMenuHelp(MenuItemHelp);/*���ô������˵����*/
   	gtk_widget_show(MenuBar);

    time_label=gtk_label_new(" ");//��ʾϵͳʱ��
    temp_frame=gtk_frame_new(NULL);//ҳ����
    temp_frame1=gtk_frame_new(NULL);//ҳ����
    timer=gtk_timeout_add(1000,(GtkFunction)sys_time,data);//��һ����ʱ��(timer),ÿ���������һ��(GtkFunction)stys_time����
    gtk_container_set_border_width (GTK_CONTAINER (temp_frame), 3);//������Ե�ľ���
    gtk_container_set_border_width (GTK_CONTAINER (temp_frame1), 3);//������Ե�ľ���
	gtk_table_attach_defaults(GTK_TABLE(table),temp_frame,9,12,11,12);//��ʱ���ǩװ��table
    gtk_container_add(GTK_CONTAINER(temp_frame),temp_frame1);//�����tbale1������
    gtk_container_add(GTK_CONTAINER(temp_frame1),time_label);//�����tbale1������
	gtk_widget_show_all(temp_frame);


        uptime_label=gtk_label_new("");//
    	timer=gtk_timeout_add(1000,(GtkFunction)uptime,data);//��һ����ʱ��(timer),ÿ���������һ�κ���
    	gtk_widget_show(uptime_label);//��ʾ��ǩ

     	process_label=gtk_label_new("");//������ʾ�������ı�ǩ
        gtk_widget_show(process_label);
    	timer=gtk_timeout_add(1000,(GtkFunction)process_num,data);//��һ����ʱ��(timer),ÿ���������һ��(GtkFunction)process_num����
        gtk_table_attach_defaults(GTK_TABLE(table),process_label,0,2,11,12);//��״̬������ʾ��������

    	cpu_rate_label=gtk_label_new(" ");//������ʾcpuռ���ʵı�ǩ
        gtk_widget_show(cpu_rate_label);
    	timer=gtk_timeout_add(1000,(GtkFunction)cpu_rate_ava,data);//��һ����ʱ��(timer),ÿ���������һ��(GtkFunction)cpu_rate_ava����
        gtk_table_attach_defaults(GTK_TABLE(table),cpu_rate_label,2,4,11,12);//��״̬������ʾcpuռ����

    	mem_rate_label=gtk_label_new(NULL);//������ʾ�ڴ�ռ���ʵı�ǩ
     	gtk_widget_show(mem_rate_label);
   	timer=gtk_timeout_add(1000,(GtkFunction)mem_rate_ava,data);//��һ����ʱ��(timer),ÿ���������һ��(GtkFunction)mem_rate_ava����
   	gtk_table_attach_defaults(GTK_TABLE(table),mem_rate_label,4,7,11,12);//��״̬������ʾ�ڴ�ռ����

 	swap_rate_label=gtk_label_new(NULL);//������ʾ������ռ���ʵı�ǩ
     	gtk_widget_show(swap_rate_label);
        timer=gtk_timeout_add(1000,(GtkFunction)swap_rate_ava,data);//��һ����ʱ��(timer),ÿ���������һ��(GtkFunction)swap_rate_ava����
   	gtk_table_attach_defaults(GTK_TABLE(table),swap_rate_label,7,9,11,12);//��״̬������ʾ������ռ����

	cpu_bar=gtk_progress_bar_new();//����CPU������
  	mem_bar=gtk_progress_bar_new();//�����ڴ������
	swap_bar=gtk_progress_bar_new();//����������������
	gtk_widget_show(cpu_bar);
	gtk_widget_show(mem_bar);
	gtk_widget_show(swap_bar);

    notebook_history_init();//historyҳ���ʼ��
	notebook_mem_init();//�ڴ�ҳ���ʼ��
	notebook_pro_init();//����ҳ���ʼ��
	notebook_disk_init();//����ҳ���ʼ��
	notebook_sys_init();//ϵͳҳ���ʼ��
    notebook_cpu_init();
    notebook_modules_init(notebook);//modulesҳ���ʼ��

	timer = gtk_timeout_add (1000, (GtkFunction)get_cpu_rate, NULL);

   	gtk_signal_connect(GTK_OBJECT(window),"delete_event",GTK_SIGNAL_FUNC(delete_event),NULL);

 	gtk_main();

}

/*�����˵���*/
GtkWidget *CreateMenuItem(GtkWidget *MenuBar,char *test){
    GtkWidget *MenuItem;
    MenuItem=gtk_menu_item_new_with_label(test);//�������˵���
   	gtk_menu_shell_append(GTK_MENU_SHELL(MenuBar),MenuItem);//�Ѹ��˵��Ͳ˵�������һ��
  	gtk_widget_show(MenuItem);//��ʾ���˵�
    return MenuItem;
}

/*�����ļ��˵�*/
GtkWidget *CreateMenuFile(GtkWidget *MenuItem){
     	GtkWidget *Menu;//������˵�
    	GtkWidget *Exit;
        Menu=gtk_menu_new();//�����˵�
        Exit=CreateMenuItem(Menu,"EXIT");//���ô������˵�����
    	gtk_signal_connect(GTK_OBJECT(Exit),"activate",GTK_SIGNAL_FUNC(delete_event),NULL);//����delete_event�Ļص�����
    	gtk_menu_item_set_submenu(GTK_MENU_ITEM(MenuItem),Menu);//�Ѹ��˵���͸��˵�������
    	gtk_widget_show(Menu);
}

/*�����ػ�ѡ��˵�*/
GtkWidget *CreateMenuShutDown(GtkWidget *MenuItem){
     	GtkWidget *Menu;//����˵���
     	GtkWidget *Restart,*ShutDown,*LogOut;
    	Menu=gtk_menu_new();//�������˵�
   	ShutDown=CreateMenuItem(Menu,"Shut Down");//���ô������˵�����
    	LogOut=CreateMenuItem(Menu,"Log Out");
    	Restart=CreateMenuItem(Menu,"Reboot");
    	gtk_signal_connect(GTK_OBJECT(Restart),"activate",GTK_SIGNAL_FUNC(restart),NULL);
    	gtk_signal_connect(GTK_OBJECT(LogOut),"activate",GTK_SIGNAL_FUNC(logout),NULL);
    	gtk_signal_connect(GTK_OBJECT(ShutDown),"activate",GTK_SIGNAL_FUNC(shutdown),NULL);
  	gtk_menu_item_set_submenu(GTK_MENU_ITEM(MenuItem),Menu);//�Ѹ��˵������Ӳ˵���ϵ����
    	gtk_widget_show(Menu);
}

/*���������˵�*/
GtkWidget *CreateMenuHelp(GtkWidget *MenuItem){
    	GtkWidget *Menu;//�����Ӳ˵�
    	GtkWidget *AboutGtk;
    	Menu=gtk_menu_new();//�����Ӳ˵�
        AboutGtk=CreateMenuItem(Menu,"About Me");//���ô����˵����
    	gtk_signal_connect(GTK_OBJECT(AboutGtk),"activate",GTK_SIGNAL_FUNC(aboutSysMo),NULL);
     	gtk_menu_item_set_submenu(GTK_MENU_ITEM(MenuItem),Menu);//�Ѹ��˵���Ͳ˵��������
    	gtk_widget_show(Menu);
}

/*����ϵͳʱ��*/
gint sys_time(void){
     	time_t timep;
     	struct tm *p;
   	char buf[30]="Time:",temp[5];
    	time(&timep);
    	p=localtime(&timep); //ȡ�õ���ʱ��
    	sprintf(temp,"%d",p->tm_hour);
	strcat(buf,temp);
    	sprintf(temp,"%2d",p->tm_min);
    	strcat(buf,":");
     	strcat(buf,temp);
     	sprintf(temp,"%2d",p->tm_sec);
    	strcat(buf,":");
   	strcat(buf,temp);
   	gtk_label_set_text(GTK_LABEL(time_label),buf);//�ı��ǩ������
    	return 1;
}

/*����cpuռ����*/
gint cpu_rate_ava(void){
     	char buffer[1024*5];
	char *match;
   	FILE *fp;
  	size_t bytes;
   	char show[5];
  	char text_cpu[14]="CPU : ";
   	int user,nice,system,idle;

    	fp = fopen("/proc/stat","r");//��/proc/stat�ļ�
   	if (fp == 0){
		printf("open /proc/stat file error\n");
  		return 0;
	}
  	bytes = fread(buffer, 1, sizeof(buffer),fp);//��ȡ������Ϣ
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
   	cpu_rate=100*(user-USER+nice-NICE+system-SYSTEM)/(float)(user-USER+nice-NICE+system-SYSTEM+idle-IDLE);//����cpu��ռ����
	USER=user;//���汾�ε�ʹ����
	NICE=nice;
	SYSTEM=system;
	IDLE=idle;
    	sprintf(show,"%.3f",cpu_rate);
   	strcat(text_cpu,show);
   	strcat(text_cpu,"%");
    	gtk_label_set_text(GTK_LABEL(cpu_rate_label),text_cpu);//�����д���ǩ
  	gtk_progress_bar_set_fraction (GTK_PROGRESS_BAR (cpu_bar),cpu_rate/100);//������ڽ���������ʾ
  	gtk_progress_bar_set_text(GTK_PROGRESS_BAR(cpu_bar),text_cpu);//������ڽ��������ı�����ʾ
}

/*�����ڴ�ռ����*/
gint mem_rate_ava(void){
    	int mem_fd;
    	int MemFree,MemTotal,MemUsed,i=0,j=0;
    	char buf[1000],buf1[1000]=" ",buf2[100]=" ",buf3[100]=" ",buf4[100]=" ",string[20];
    	char *p;
    	char Mem[100]=" ";

   	mem_fd=open("/proc/meminfo",O_RDONLY);//��/proc/meminfo�ļ�
   	read(mem_fd,buf,1000);//��ȡ�ļ���Ϣ
   	close(mem_fd);
   	strcat(buf1,strtok(buf,"\n"));
    	strcat(buf2,buf1);
    	for(i=0;i<100&&buf2[i]!='\0';i++)
  		if(buf2[i]>='0'&&buf2[i]<='9')buf3[j++]=buf2[i];
     	buf3[j]='\0';
     	MemTotal=atoi(buf3);//��ȡ�ڴ�����
    	strcat(buf1,strtok(NULL,"\n"));
     	strcpy(buf2,buf1);
     	j=0;
    	for(;i<100&&buf2[i]!='\0';i++)
		if(buf2[i]>='0'&&buf2[i]<='9')buf4[j++]=buf2[i];
	buf4[j]='\0';
     	MemFree=atoi(buf4);//��ȡ�ڴ������
    	MemUsed=MemTotal-MemFree;//�����ڴ�ʹ����
   	MemTotal=MemTotal/1024;
   	MemUsed=MemUsed/1024;
    	strcpy(Mem,"Memory: ");
  	mem_rate=100*(MemUsed/(float) MemTotal);//�����ڴ�ʹ����
    	sprintf(string,"%.2f",mem_rate);
   	strcat(Mem,string);
	strcat(Mem,"%");
   	gtk_label_set_text(GTK_LABEL(mem_rate_label),Mem);//�����д���ǩ
	gtk_progress_bar_set_fraction (GTK_PROGRESS_BAR (mem_bar),mem_rate/100);//������ڽ���������ʾ
  	gtk_progress_bar_set_text(GTK_PROGRESS_BAR(mem_bar),Mem);//������ڽ��������ı�����ʾ
}

/*���㽻����ʹ����*/
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

      	swap_fd=open("/proc/meminfo",O_RDONLY);//��/proc/meminfo�ļ�
       	read(swap_fd,buf,sizeof (buf));//��ȡ�ļ���Ϣ
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
        swap_rate=100*(1-(float)(SwapTotal/SwapFree));//���㽻����ʹ����
        sprintf(show,"%.2f",swap_rate);
        strcat(text_swap,show);
        strcat(text_swap,"%");
        gtk_label_set_text(GTK_LABEL(swap_rate_label),text_swap);//�����д���ǩ
	gtk_progress_bar_set_fraction (GTK_PROGRESS_BAR (swap_bar),swap_rate);//������ڽ���������ʾ
	gtk_progress_bar_set_text(GTK_PROGRESS_BAR(swap_bar),text_swap);//������ڽ��������ı�����ʾ
}

/*����ϵͳ����ʱ���ϵͳ����ʱ��*/
gint uptime(void){
	int ut_fd,now_time,run_time,start_time,h,m,s;
	char buf[30],*ch,buffer[20],tp[5],start[100]="System Started Time:";
	int f;
	time_t timep;
     	struct tm *p;
	gchar *delim=" ";

	time(&timep);
    	p=localtime(&timep);//�õ�����ʱ��
	now_time=p->tm_hour*3600+p->tm_min*60+p->tm_sec;//������ʱ��ת��Ϊ��
	ut_fd=open("/proc/uptime",O_RDONLY);//��/proc/uptime�ļ�
	read(ut_fd,buf,sizeof (buf));//��ȡ�ļ�
	close(ut_fd);
	ch=strtok(buf,delim);strtok(NULL,delim);strtok(NULL,delim);gtk_label_set_text(GTK_LABEL(uptime_label),ch);
	run_time=atoi(ch);//�����ϵͳ���е�����
	start_time=now_time-run_time;//����ϵͳ������ʱ��
	if(start_time<0){
		start_time=24*3600+start_time;
	}
	h=start_time/3600;//ϵͳ����ʱ�䣬ת��Ϊʱ����
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
	h=run_time/3600;//ϵͳ����ʱ�䣬ת��Ϊʱ����
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
	gtk_label_set_text(GTK_LABEL(uptime_label),start);//�����д���ǩ
	//return 1;
}

/*ʵʱ��ʾ��������*/
gint process_num(void){
     	DIR * dir;
    	struct dirent * ptr;
    	char show[5];
     	int total=0;
    	char text_value[100]="Process:  ";

   	dir =opendir("/proc");
   	while((ptr = readdir(dir))!=NULL){
		if(select_name(ptr->d_name)){//d_name:�ļ���
		total++;
		}
	}
 	sprintf(show,"%3d",total);
   	strcat(text_value,show);//׷��show�е����ݵ�text_value��
  	closedir(dir);
    	gtk_label_set_text(GTK_LABEL(process_label),text_value);//�����д���ǩ
}

//CPU ͼ����
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
/*SOURCESҳ���ʼ��*/
void notebook_cpu_init(void){
  	float v,v0;
  	char text[20];char text0[20];
  	gfloat value1,value2;

  	vbox = gtk_vbox_new (FALSE, 0);//����������
  	gtk_widget_show (vbox);
  	gtk_table_attach_defaults(GTK_TABLE(table1[5]),vbox,0,12,0,12);

  	frame0 = gtk_frame_new ("CPU");//���������ʾCPU��Ϣ�ı߿򹹼�
  	gtk_widget_show (frame0);
  	gtk_box_pack_start (GTK_BOX (vbox), frame0, FALSE, FALSE, 8);//�ӵ�������
  	gtk_container_add (GTK_CONTAINER (frame0),cpu_bar);

  	frame0 = gtk_frame_new ("Memory");//���������ʾ�ڴ���Ϣ�ı߿򹹼�
  	gtk_widget_show (frame0);
  	gtk_box_pack_start (GTK_BOX (vbox), frame0, FALSE, FALSE, 8);
  	gtk_container_add (GTK_CONTAINER (frame0),mem_bar);

  	frame0 = gtk_frame_new ("Swap");//���������ʾswap��Ϣ�ı߿򹹼�
  	gtk_widget_show (frame0);
  	gtk_box_pack_start (GTK_BOX (vbox), frame0, FALSE, FALSE, 8);
  	gtk_container_add (GTK_CONTAINER (frame0),swap_bar);

  	frame0 = gtk_frame_new ("UpTime");//���������ʾϵͳʱ����Ϣ�ı߿򹹼�
  	gtk_widget_show (frame0);
  	gtk_box_pack_start (GTK_BOX (vbox), frame0, FALSE, FALSE, 8);
  	gtk_container_add (GTK_CONTAINER (frame0),uptime_label);


    frame0 = gtk_frame_new ("CPU History");//���������ʾϵͳʱ����Ϣ�ı߿򹹼�
  	gtk_widget_show (frame0);
  	gtk_box_pack_start (GTK_BOX (vbox), frame0, FALSE, FALSE, 8);

/* ����,���ǿ�ʼ����Ļ��ͼ��
	 * ����ʹ�õĹ����ǻ�ͼ��������
	 *һ����ͼ��������������һ�� X ����,û�������Ķ�����
	 *����һ���հ׵Ļ���,���ǿ��������ϻ�����Ҫ�Ķ�����*/
	cpu_draw_area = gtk_drawing_area_new();
	/* ���ÿ��Ի�ͼ */
	gtk_widget_set_app_paintable(cpu_draw_area, TRUE);

	/* ��ͼ��Ĭ�ϴ�С */
	gtk_drawing_area_size(GTK_DRAWING_AREA(cpu_draw_area), 500, 100);
	/* ������ӵ�cpu���߿���� */
	gtk_container_add(GTK_CONTAINER(frame0), cpu_draw_area);
	gtk_widget_show(cpu_draw_area);
	/* �����ص����� */
	/* ���ڸ����¼������˽� */
	g_signal_connect(cpu_draw_area, "expose_event",
			G_CALLBACK(cpu_expose_event), NULL);
    gtk_widget_show (cpu_draw_area);
    gtk_widget_show(window);
    timer = gtk_timeout_add (1000, cpu_expose_event, (gpointer) cpu_draw_area);
}

//memory ͼ����
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

//swap ͼ����
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

void notebook_history_init(void)//historyҳ���ʼ��
{
    float v,v0;
  	char text[20];char text0[20];
  	gfloat value1,value2;

  	vbox = gtk_vbox_new (FALSE, 0);//����������
  	gtk_widget_show (vbox);
  	gtk_table_attach_defaults(GTK_TABLE(table1[0]),vbox,0,12,0,12);


    frame0 = gtk_frame_new ("CPU History");//���������ʾcpu history��Ϣ�ı߿򹹼�
  	gtk_widget_show (frame0);
  	gtk_box_pack_start (GTK_BOX (vbox), frame0, FALSE, FALSE, 8);


/* ����,���ǿ�ʼ����Ļ��ͼ��
	 * ����ʹ�õĹ����ǻ�ͼ��������
	 *һ����ͼ��������������һ�� X ����,û�������Ķ�����
	 *����һ���հ׵Ļ���,���ǿ��������ϻ�����Ҫ�Ķ�����*/
	cpu_draw_area = gtk_drawing_area_new();
	/* ���ÿ��Ի�ͼ */
	gtk_widget_set_app_paintable(cpu_draw_area, TRUE);

	/* ��ͼ��Ĭ�ϴ�С */
	gtk_drawing_area_size(GTK_DRAWING_AREA(cpu_draw_area), 500, 100);
	/* ������ӵ�cpu���߿���� */
	gtk_container_add(GTK_CONTAINER(frame0), cpu_draw_area);
	gtk_widget_show(cpu_draw_area);
	/* �����ص����� */
	/* ���ڸ����¼������˽� */
	g_signal_connect(cpu_draw_area, "expose_event",
			G_CALLBACK(cpu_expose_event), NULL);
    gtk_widget_show (cpu_draw_area);
    gtk_widget_show(window);
    timer = gtk_timeout_add (1000, cpu_expose_event, (gpointer) cpu_draw_area);

    frame0 = gtk_frame_new ("Memory History");//���������ʾmemory history��Ϣ�ı߿򹹼�
  	gtk_widget_show (frame0);
  	gtk_box_pack_start (GTK_BOX (vbox), frame0, FALSE, FALSE, 8);

    /* ����,���ǿ�ʼ����Ļ��ͼ��
	 * ����ʹ�õĹ����ǻ�ͼ��������
	 *һ����ͼ��������������һ�� X ����,û�������Ķ�����
	 *����һ���հ׵Ļ���,���ǿ��������ϻ�����Ҫ�Ķ�����*/
	memory_draw_area = gtk_drawing_area_new();
	/* ���ÿ��Ի�ͼ */
	gtk_widget_set_app_paintable(memory_draw_area, TRUE);

	/* ��ͼ��Ĭ�ϴ�С */
	gtk_drawing_area_size(GTK_DRAWING_AREA(memory_draw_area), 500, 100);
	/* ������ӵ�cpu���߿���� */
	gtk_container_add(GTK_CONTAINER(frame0), memory_draw_area);
	gtk_widget_show(memory_draw_area);
	/* �����ص����� */
	/* ���ڸ����¼������˽� */
	g_signal_connect(memory_draw_area, "expose_event",
			G_CALLBACK(memory_expose_event), NULL);
    gtk_widget_show (memory_draw_area);
    gtk_widget_show(window);
    timer = gtk_timeout_add (1000, memory_expose_event, (gpointer) memory_draw_area);

    frame0 = gtk_frame_new ("SWAP History");//���������ʾswap history��Ϣ�ı߿򹹼�
  	gtk_widget_show (frame0);
  	gtk_box_pack_start (GTK_BOX (vbox), frame0, FALSE, FALSE, 8);


/* ����,���ǿ�ʼ����Ļ��ͼ��
	 * ����ʹ�õĹ����ǻ�ͼ��������
	 *һ����ͼ��������������һ�� X ����,û�������Ķ�����
	 *����һ���հ׵Ļ���,���ǿ��������ϻ�����Ҫ�Ķ�����*/
	swap_draw_area = gtk_drawing_area_new();
	/* ���ÿ��Ի�ͼ */
	gtk_widget_set_app_paintable(swap_draw_area, TRUE);

	/* ��ͼ��Ĭ�ϴ�С */
	gtk_drawing_area_size(GTK_DRAWING_AREA(swap_draw_area), 500, 100);
	/* ������ӵ�cpu���߿���� */
	gtk_container_add(GTK_CONTAINER(frame0), swap_draw_area);
	gtk_widget_show(swap_draw_area);
	/* �����ص����� */
	/* ���ڸ����¼������˽� */
	g_signal_connect(swap_draw_area, "expose_event",
			G_CALLBACK(swap_expose_event), NULL);
    gtk_widget_show (swap_draw_area);
    gtk_widget_show(window);
    timer = gtk_timeout_add (1000, swap_expose_event, (gpointer) swap_draw_area);
}
/*�ڴ�ҳ���ʼ��*/
void notebook_mem_init(void){
  	int i;
  	gchar *col_name[3] = { "Project", "Size"," "};
  	vbox = gtk_vbox_new (FALSE, 0);//����������
  	gtk_widget_show (vbox);
  	gtk_table_attach_defaults(GTK_TABLE(table1[1]),vbox,0,12,0,12);

  	scrolled_window = gtk_scrolled_window_new (NULL, NULL);//��ӹ������ڿؼ�
  	gtk_widget_set_size_request (scrolled_window, 300, 300);
  	gtk_widget_show (scrolled_window);
  	gtk_box_pack_start (GTK_BOX (vbox), scrolled_window, TRUE, TRUE, 0);


	//������Ϣ
 	 mem_store = gtk_list_store_new (NM_COLUMNS,
					G_TYPE_STRING,
					G_TYPE_STRING,
					G_TYPE_STRING);

  	mtree_view = gtk_tree_view_new_with_model (GTK_TREE_MODEL (mem_store));
  	//���tree view�ؼ�����ʾ������Ϣ��

  	g_object_unref (G_OBJECT (mem_store));//�������ô���


  	gtk_widget_show (mtree_view);
  	gtk_container_add (GTK_CONTAINER (scrolled_window), mtree_view);//��������Ϣ���ӵ�����������


  	for (i = 0; i < 3; i++) {
		mrenderer = gtk_cell_renderer_text_new ();//���һ��cell_renderer_text������ʾ����
  		mcolumn = gtk_tree_view_column_new_with_attributes (col_name[i],mrenderer,"text",i,NULL);//�½�һ��
	    	gtk_tree_view_append_column (GTK_TREE_VIEW (mtree_view), mcolumn);//�����мӵ�����
  	}


  	get_mem_info(mem_store); //��ȡ������Ϣ����ȡ��ʾ���б�


  	hbox = gtk_hbox_new (FALSE, 0);//��Ӻ����к���
  	gtk_widget_show (hbox);
  	gtk_box_pack_start (GTK_BOX (vbox), hbox, TRUE, FALSE, 0);//���ӵ������к�����

  	prefresh_button = gtk_button_new ();//����ˢ�½�����Ϣ��ť
  	gtk_widget_show (prefresh_button);
  	gtk_widget_set_size_request (prefresh_button, 70, 30);//����ˢ�°�ť��С
  	gtk_button_set_label (GTK_BUTTON (prefresh_button), "refresh");//����ˢ�°�ť����ʾ������
  	g_signal_connect (G_OBJECT (prefresh_button),"clicked",G_CALLBACK(prefresh_button_clicked2),NULL);//ˢ�°�ť�����ִ��prefresh_button_clicked
  	gtk_box_pack_start (GTK_BOX (hbox), prefresh_button, TRUE, FALSE, 0);//���Ӹð�ť�������к�����
}

/*��ʾ�ڴ���Ϣ�Ļص�����*/
gint get_mem_info(GtkListStore *store){
        int mem_fd,i=1,j,m;
   	char buf[1000],buf1[1000]=" ";
    	char *p,*c[50],*ch[3];
    	gint sign;
	GtkTreeIter iter;
    	gpointer gdata;

     	mem_fd=open("/proc/meminfo",O_RDONLY);//��/proc/meminfo�ļ�
    	read(mem_fd,buf,1000);//��ȡ�ļ���Ϣ
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
		gtk_list_store_append (store, &iter);//���ӵ��б�
		gtk_list_store_set (store, &iter,
					NAME3_COLUMN,ch[0],
					SIZE_COLUMN,ch[1],
					KB_COLUMN,ch[2],
					-1);
	}
     	return 1;
}

/*����ҳ���ʼ��*/
void notebook_pro_init(void){
	int i;
  	gchar *col_name[5] = { "Project Name", "PID" , "Status", "CPU %" , "Memory used"};

  	vbox = gtk_vbox_new (FALSE, 0);//����������
  	gtk_widget_show (vbox);
  	gtk_table_attach_defaults(GTK_TABLE(table1[2]),vbox,0,12,0,10);

  	scrolled_window = gtk_scrolled_window_new (NULL, NULL);//��ӹ������ڿؼ�
  	gtk_widget_set_size_request (scrolled_window, 300, 300);
  	gtk_widget_show (scrolled_window);
  	gtk_box_pack_start (GTK_BOX (vbox), scrolled_window, TRUE, TRUE, 0);

	//������Ϣ
  	process_store = gtk_list_store_new (NP_COLUMNS,
					G_TYPE_STRING,
					G_TYPE_STRING,
					G_TYPE_STRING,
					G_TYPE_STRING,
					G_TYPE_STRING);

  	ptree_view = gtk_tree_view_new_with_model (GTK_TREE_MODEL (process_store));
  	//���tree view�ؼ�����ʾ������Ϣ��

  	g_object_unref (G_OBJECT (process_store));//�������ô���


  	gtk_widget_show (ptree_view);
  	gtk_container_add (GTK_CONTAINER (scrolled_window), ptree_view);//��������Ϣ���ӵ�����������


  	for (i = 0; i < 5; i++) {
		renderer = gtk_cell_renderer_text_new ();//���һ��cell_renderer_text������ʾ����
  		column = gtk_tree_view_column_new_with_attributes (col_name[i],renderer,"text",i,NULL);//�½�һ��
	    	gtk_tree_view_append_column (GTK_TREE_VIEW (ptree_view), column);//�����мӵ�����
  	}

  	hbox = gtk_hbox_new (FALSE, 0);//��Ӻ����к���
  	gtk_widget_show (hbox);
  	gtk_box_pack_start (GTK_BOX (vbox), hbox, TRUE, FALSE, 0);//���ӵ������к�����

  	prefresh_button = gtk_button_new ();//����ˢ�½�����Ϣ��ť
  	gtk_widget_show (prefresh_button);
  	gtk_widget_set_size_request (prefresh_button, 60, 30);//����ˢ�°�ť��С
  	gtk_button_set_label (GTK_BUTTON (prefresh_button), "refresh");//����ˢ�°�ť����ʾ������
  	g_signal_connect (G_OBJECT (prefresh_button),"clicked",G_CALLBACK(prefresh_button_clicked),NULL);//ˢ�°�ť�����ִ��prefresh_button_clicked
  	gtk_table_attach_defaults(GTK_TABLE(table1[2]), prefresh_button,2,4,10,11);

  	pdelete_button = gtk_button_new ();//����ɱ�����̰�ť
  	gtk_widget_show (pdelete_button);
  	gtk_widget_set_size_request (pdelete_button, 60, 30);
  	gtk_button_set_label (GTK_BUTTON (pdelete_button), "kill");
  	g_signal_connect (G_OBJECT (pdelete_button),"clicked",G_CALLBACK(pdelete_button_clicked),NULL);//�ð�ť�����ִ��pdelete_button_clicked
	gtk_table_attach_defaults(GTK_TABLE(table1[2]), pdelete_button,7,9,10,11);

  	s_label=gtk_label_new(" ");
  	gtk_widget_show(s_label);//��ʾ��ǩ
  	gtk_table_attach_defaults(GTK_TABLE(table1[2]),s_label,1,5,11,12);
  	r_label=gtk_label_new(" ");
  	gtk_widget_show(r_label);//��ʾ��ǩ
  	gtk_table_attach_defaults(GTK_TABLE(table1[2]),r_label,6,10,11,12);
  	get_process_info (process_store);
  	gtk_list_store_clear (process_store);
  	get_process_info (process_store);
}

/*��ý��������Ϣ�ĺ���*/
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
  	dir = opendir ("/proc");//��/procĿ¼��dirΪ���ص�ָ��
  	while ((entry = readdir (dir)) != NULL ) {
		if ((entry->d_name[0] >= '0') && (entry->d_name[0] <= '9')) {
			sprintf (dir_buf, "/proc/%s/stat", entry->d_name);//��ȡ�ļ���dir_buf��
			fd = open (dir_buf, O_RDONLY);
			read (fd, buffer, sizeof (buffer));
			close (fd);

			info[0] =  strtok (buffer, delim);//��delim�ָ����и�buffer��һ���ֵ�info��0��
			for (i = 1; i < 26 ; i++) {
				info[i] = strtok (NULL, delim);//��������buffer�ַ���
			}
			pcuser[num] = atoi (info[13]);//ת���ַ���Ϊ������
			srate = (pcuser[num]-pfuser[num]) / (2 * total);//cpu����ռ��cpuʹ�õİٷֱ�
                	if(srate<0||srate>1){srate=0;}
			sprintf (rate_buffer, "%.2f%%",100 * srate);

			mem = atoi (info[22]);//�ý��̵��ڴ�ʹ����
			mem = mem / (1024 * 1024);
			sprintf (mem_buffer, "%-.2f MB",mem);

			gtk_list_store_append (store, &iter);//���ӵ��б�
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

/*����ҳ���ʼ��*/
void notebook_disk_init(void){
  	int i;
  	gchar *col_name[4] = { "MAJOR", "MINOR" , "BLOCK", "NAME"};

  	vbox = gtk_vbox_new (FALSE, 0);//����������
  	gtk_widget_show (vbox);
  	gtk_table_attach_defaults(GTK_TABLE(table1[3]),vbox,0,12,0,12);

  	scrolled_window = gtk_scrolled_window_new (NULL, NULL);//��ӹ������ڿؼ�
  	gtk_widget_set_size_request (scrolled_window, 300, 300);
  	gtk_widget_show (scrolled_window);
  	gtk_box_pack_start (GTK_BOX (vbox), scrolled_window, TRUE, TRUE, 0);


	//������Ϣ
  	disk_store = gtk_list_store_new (NB_COLUMNS,
					G_TYPE_STRING,
					G_TYPE_STRING,
					G_TYPE_STRING,
					G_TYPE_STRING);


  	btree_view = gtk_tree_view_new_with_model (GTK_TREE_MODEL (disk_store));
  	//���tree view�ؼ�����ʾ������Ϣ��
  	g_object_unref (G_OBJECT (disk_store));//�������ô���

  	gtk_widget_show (btree_view);
  	gtk_container_add (GTK_CONTAINER (scrolled_window), btree_view);//��������Ϣ���ӵ�����������
  	for (i = 0; i < 4; i++) {
		drenderer = gtk_cell_renderer_text_new ();//���һ��cell_renderer_text������ʾ����
  		dcolumn = gtk_tree_view_column_new_with_attributes (col_name[i],drenderer,"text",i,NULL);//�½�һ��
	    	gtk_tree_view_append_column (GTK_TREE_VIEW (btree_view), dcolumn);//�����мӵ�����
  	}
  	get_disk_info(disk_store); //��ȡ������Ϣ����ȡ��ʾ���б�
}

/*��ô��������Ϣ�ĺ���*/
void get_disk_info(GtkListStore *store){
	int disk_fd,i=0,j,m;
       	char buf[1000],buf1[1000]=" ";
       	char *p;
	char *c[8],*ch[4],buffer[16];
	GtkTreeIter iter;
	GtkWidget *text;;

	text=gtk_text_new(NULL,NULL);
       	disk_fd=open("/proc/partitions",O_RDONLY);//��/proc/partitions�ļ�
       	read(disk_fd,buf,1000);//��ȡ�ļ���Ϣ
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
		gtk_list_store_append (store, &iter);//���ӵ��б�
		gtk_list_store_set (store, &iter,
					MAJOR_COLUMN,ch[0],
					MINOR_COLUMN,ch[1],
					BLOCK_COLUMN,ch[2],
					NAME2_COLUMN,ch[3],
					-1);
	}
}

/*ϵͳҳ���ʼ��*/
void notebook_sys_init(void){
  	vbox = gtk_vbox_new (FALSE, 0);//����������
  	gtk_widget_show (vbox);
  	gtk_table_attach_defaults(GTK_TABLE(table1[4]),vbox,0,12,0,12);

  	frame0 = gtk_frame_new ("CPU Info:");//��ʾCPU��Ϣ�߿�
  	gtk_widget_show (frame0);
  	gtk_box_pack_start (GTK_BOX (vbox), frame0, TRUE, TRUE, 10);//�ӵ����������

  	info = g_string_new ("");
  	get_cpu_info (info);//�� info�����洢��õ�cpu��Ϣ

  	info_label = gtk_label_new (info->str);//�ñ�ǩinfo_label��ʾinfo�������ַ�������
  	gtk_widget_show (info_label);
  	gtk_container_add (GTK_CONTAINER(frame0), info_label);//���ñ�ǩ�ӵ��߿���

  	frame0 = gtk_frame_new ("OS Info:");//��ʾ����ϵͳ��Ϣ�߿�
  	gtk_widget_show (frame0);
  	gtk_box_pack_start (GTK_BOX (vbox), frame0, TRUE, TRUE, 10);//�ӵ����������

  	info = g_string_new ("");
  	get_os_info (info);//�� info�����洢��õĲ���ϵͳ��Ϣ

  	info_label = gtk_label_new (info->str);//�ñ�ǩinfo_label��ʾinfo�������ַ�������
  	gtk_widget_show (info_label);
  	gtk_container_add (GTK_CONTAINER(frame0), info_label);//���ñ�ǩ�ӵ��±߿���
}

/*���cpu��Ϣ*/
void get_cpu_info(GString *string){
  	int fd,i;
  	gchar buffer[1024];
  	gchar *delim = "\n";
  	gchar *cpu_info[17];
  	gchar *tmp;

  	fd = open ("/proc/cpuinfo", O_RDONLY);//��ȡcpu�����Ϣ���ļ�
  	read (fd, buffer, sizeof (buffer));
  	close (fd);

  	cpu_info[0] = strtok (buffer, delim);
  	for (i=1; i < 17 ; i++) {
		cpu_info[i] = strtok (NULL,delim);//�ָ��ַ���
  	}
  	for (i=0; i < 17; i++) {
  	  	tmp = strstr (cpu_info[i], ":");//����ð��:
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

/*��ò���ϵͳ�����Ϣ*/
void get_os_info (GString *string){
	int fd,i;
      	gchar buffer[128];
      	gchar *tmp,*start,*stop;
	gchar *os_info[8];
	gchar *delim = " ";

	fd = open ("/proc/version", O_RDONLY);//��ȡ����ϵͳ�����Ϣ���ļ�
	read(fd, buffer, sizeof (buffer));

	start = buffer;
	stop = strstr (buffer, "#");
	stop--;
	stop--;
	os_info[0] = strtok (buffer, delim);
	for (i = 1; i < 8; i++) {
		os_info[i] = strtok (NULL, delim);//�ָ��ַ���

	}

	g_string_append (string, "OS     :");//��ʾ�����Ϣ
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

/*����ˢ�°�ť������*/
void prefresh_button_clicked (gpointer data){//ˢ�°�ť�������ˢ�½��������Ϣ
  	gtk_list_store_clear (process_store);
  	get_process_info (process_store);
}

/*�ڴ�ˢ�°�ť������*/
void prefresh_button_clicked2 (gpointer data){//ˢ�°�ť�������ˢ�½��������Ϣ
  	gtk_list_store_clear (mem_store);
  	get_mem_info (mem_store);
}

/*����ɱ����ť������*/
void pdelete_button_clicked (gpointer data){//kill��ť�������ֹͣһ�����̵�����
  	GtkTreeSelection *selection;
  	GtkTreeModel *model;
  	GtkTreeIter iter;
  	gchar *pid;
  	pid_t pid_num;

  	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(ptree_view));//��õ�ǰѡ�����
  	if (gtk_tree_selection_get_selected(selection, &model, &iter)) {
		gtk_tree_model_get (model, &iter, PID_COLUMN, &pid, -1);//��������Ӧ���л�øý��̵�PID
  		pid_num = atoi (pid);//�ַ���ת���ɳ�����

	    	if(kill (pid_num, SIGTERM) == -1 ) {//����PID�����ý���
			gchar *title = "ERROR";
			gchar *content = "Termination Failed,Check UID";
			show_dialog (title, content);//��������ʧ����Ϣ���
		}
	    	gtk_list_store_clear (process_store);//ˢ�½�����Ϣ
	    	get_process_info (process_store);
  	}
}

/*��ʾһ���Ի��򣬱���title������Ϊcontent*/
void show_dialog (gchar *title, gchar *content){
	GtkWidget *dialog;
	GtkWidget *label;
        dialog = gtk_dialog_new_with_buttons (title,
                                         GTK_WINDOW (window),
                                         GTK_DIALOG_DESTROY_WITH_PARENT,
                                         GTK_STOCK_CLOSE,
                                         GTK_RESPONSE_NONE,
                                         NULL);//����һ���Ի��򹹼�
	gtk_window_set_resizable (GTK_WINDOW (dialog), FALSE);//���ɸı��С
	g_signal_connect_swapped (dialog,
                             "response",
                             G_CALLBACK (gtk_widget_destroy),
                             dialog);//�¼��ص�����Ϊ�˳�

	label = gtk_label_new (content);//����һ����ǩ����ʾcontent������
	gtk_widget_show (label);
	gtk_container_add (GTK_CONTAINER (GTK_DIALOG(dialog)->vbox),
                      label);
	gtk_widget_show (dialog);
}

/*���cpuʹ����*/
gboolean get_cpu_rate ( gpointer data){
  	int fd,i;
  	gchar buffer[256];
  	gchar *cpu_time[9];
  	gchar *tmp;
  	gchar *delim =  " ";
  	gdouble cuser;
  	gdouble ctotal;


  	fd = open ("/proc/stat", O_RDONLY);//��ȡ����ļ�
  	read (fd, buffer, sizeof (buffer));
  	close (fd);
  	tmp = strstr (buffer, "cpu0");//�ӿ�ʼ����"cpu0"Ϊ��������buffer��һ�����ַ���tmp
  	tmp--;
  	*tmp = '\0';
  	cpu_time[0] = strtok (buffer, delim);//ͨ���ַ����ָ�õ�Ŀ���ַ���
  	for (i = 1; i < 9 ; i++) {
		cpu_time[i] = strtok (NULL, delim);
  	}
  	cuser = atoi (cpu_time[1]);//���ַ���ת��Ϊ������
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

	/* ��һ����񣬷�Ϊ���������� */
	table = gtk_table_new(10, 4, TRUE);
	/* ���������ӵ���� */
	gtk_container_add(GTK_CONTAINER(frame), table);

	/* ��һ��frame�ڱ��ϱ�*/
	frame_down = gtk_frame_new("");
	gtk_table_attach_defaults(GTK_TABLE(table), frame_down, 0, 4, 0, 10);
	gtk_widget_show(frame_down);
	gtk_widget_show(table);

	GtkWidget *scrolled_window = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_window),GTK_POLICY_AUTOMATIC, GTK_POLICY_ALWAYS);
	gtk_container_add(GTK_CONTAINER(frame_down), scrolled_window);
	gtk_widget_show(scrolled_window);

	/* ���ǲ�����ʾ��������ֱ�����ﶼ������ȷʵ�� */
	gchar *titles[3] ={ "Name", "Memory Used", "Loaded Times" }; //���ں����������������ֻ��ȡͨ//�ø�ʽ�ķ���
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
	char infoBuf[2000]; //�ݴ��ȡ��modules�ļ�����
	int fd = open("/proc/modules", O_RDONLY);
	read(fd, infoBuf, sizeof(infoBuf));
	close(fd);
	unsigned int lines = 0;
	unsigned int i = 0;
	gtk_clist_clear((GtkCList*)clist_modul);
	/* ��ȡ������ */
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
		char modName[25]; //ģ����
		char memUse[20]; //�ڴ���
		char times[5]; //ʹ�ô���
		int mem_num;
		float real_mem;
		getModInfo(infoBuf, i, modName, memUse, times);
		mem_num = atoi(memUse);
		real_mem = (float) mem_num / (1024);
		gcvt(real_mem, 3, convert_mem);
		gchar *list[1][3] ={{ modName, convert_mem, times } };
		gtk_clist_append((GtkCList*) clist_modul, list[0]);
		gtk_clist_thaw((GtkCList *) clist_modul); //����list�б���ʾ
	}
}
void getModInfo(char store[], int i, char modName[], char memUse[],char times[])
{
	int j = 0;
	int cflags = 0; //��¼��ȡ�Ļس��������Ա��ж�����
	int k = 0;
	char name2[25];
	char mem2[20];
	char times2[5];
	while (cflags < i - 1)
	{
		if (store[j] == '\n')
			cflags++; //�س�����1
		j++;
	}
	while (store[j] != ' ')
	{ //��ȡ������
		name2[k++] = store[j];
		j++;
	}
	name2[k] = '\0';
	j++; //��ת����һ�����ǿո�ĵط�
	k = 0;
	while (store[j] != ' ')
	{
		mem2[k++] = store[j];
		j++;
	}
	mem2[k] = '\0'; //���
	j++;
	times2[0] = store[j]; //��ȡģ���ʹ�ô���
	times2[1] = '\0'; //���
	strcpy(modName, name2);
	strcpy(memUse, mem2);
	strcpy(times, times2);
}
