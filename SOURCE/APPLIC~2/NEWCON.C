/************************************************************************/
/* Sky Operating System V2
/* Copyright (c) 1996 - 1999 by Szeleney Robert
/*
/* Project Members: Szeleney Robert
/*                  Resl Christian
/*                  Hayrapetian Gregory
/************************************************************************/
/* File       : applications\newcon.c
/* Last Update: 24.08.1999
/* Version    : alpha
/* Coded by   : Gregory Hayrapetian, Szeleney Robert
/* Docus      :
/************************************************************************/
#include "types.h"
#include "module.h"
#include "newgui.h"
#include "sched.h"
#include "rtc.h"
#include "fcntl.h"
#include "error.h"
#include "msgbox.h"
#include "vfs.h"

#include <stdarg.h>

#define CONSOLE_ROWS  30

char str_console[CONSOLE_ROWS][256];
char *str_ptr[CONSOLE_ROWS];
window_t *win_console;

extern int ifsound_pid;

int active_row, str_nr, cursor_pos;
int cmd_pid;
unsigned char cursor_color;

extern struct task_struct *current;

int pass_char, console_out;


extern unsigned int MAXX;               // Screen X Resolution --> svgadev.c
extern unsigned int MAXY;               // Screen Y Resolution --> svgadev.c

extern int timerticks;
extern char task_time;                  // delay between timertick --> sched.c

extern unsigned int scheduler_debug_wait; // delay between each taskswitch --> sched.c

extern struct s_net_device *active_device; // active network device --> 3c509.c
extern int tasksactive;                 // multiaskting active? --> sched.c

extern unsigned int net_arp_debug ;        // debug parameters
extern unsigned int net_ip_debug ;
extern unsigned int net_icmp_debug ;
extern unsigned int net_eth_debug ;
extern unsigned int net_udp_debug ;
extern unsigned int net_tcp_debug ;
extern unsigned int tftpd_monitor_debug;

extern unsigned int module_debug;
extern int cache_enabled;

// Prototypes
void help_task(void);                   // --> help.c
void system_monitor(void);              // --> sysmon.c
void device_manager_task(void);         // --> devman.c

unsigned int ftp_dest_addr;
unsigned int ftp_local_port;

void console_add_line(char *str);

void console_init_all()
{
  int i;

  for (i=0;i<CONSOLE_ROWS;i++)
  {
    str_console[i][0] = '\0';
    str_ptr[i] = str_console[i];
  }

  active_row = -1;
  str_nr = 0;
  cursor_pos = 0;
  cursor_color = COLOR_BLACK;
  pass_char = 0;
  console_out = 1;

  console_add_line("\0");
}

void console_add_line(char *str)
{
  int i;

  active_row++;

  if (active_row >= CONSOLE_ROWS)
  {
    console_clear_all();

    for (i=0;i<CONSOLE_ROWS-1;i++)
      str_ptr[i] = str_ptr[i+1];

    active_row = CONSOLE_ROWS-1;

    strcpy(str_console[str_nr],str);
    str_ptr[active_row] = str_console[str_nr];
    str_nr++;
    if (str_nr == CONSOLE_ROWS) str_nr = 0;
  }
  else
    strcpy(str_console[active_row],str);

  cursor_pos = strlen(str_ptr[active_row]);

  win_outs(win_console,10,10+active_row*13,str_ptr[active_row],COLOR_BLACK,255);
}

void console_add_char(char ch)
{
  int i;

  for (i=0;i<256;i++)
  {
    if (str_ptr[active_row][i] == '\0')
    {
      str_ptr[active_row][i] = ch;
      str_ptr[active_row][i+1] = '\0';
      break;
    }
  }
}

void console_clear_all(void)
{
  int i;


  for (i=0;i<CONSOLE_ROWS;i++)
    if (str_ptr[i][0] != '\0')
      win_outs(win_console,10,10+i*13,str_ptr[i],COLOR_LIGHTGRAY,255);

  win_outs(win_console,10+8*cursor_pos,10+(active_row)*13,"_",COLOR_LIGHTGRAY,255);

}

void console_redraw()
{
  int i;


  for (i=0;i<CONSOLE_ROWS;i++)
    if (str_ptr[i][0] != '\0')
      win_outs(win_console,10,10+i*13,str_ptr[i],COLOR_BLACK,255);

  win_outs(win_console,10+8*cursor_pos,10+(active_row)*13,"_",cursor_color,255);

}

void console_backspace()
{
  int i;

  if (cursor_pos == 0)
    return;

  for (i=0;i<256;i++)
  {
    if (str_ptr[active_row][i] == '\0')
    {
      win_outs(win_console,10,10+(active_row)*13,str_ptr[active_row],COLOR_LIGHTGRAY, 255);
      str_ptr[active_row][i-1] = '\0';
      win_outs(win_console,10,10+(active_row)*13,str_ptr[active_row],COLOR_BLACK, 255);

      cursor_pos--;
      break;
    }
  }

}

void console_set_cursor(int x, int y)
{
  win_outs(win_console,10+8*cursor_pos,10+(active_row)*13,"_",COLOR_LIGHTGRAY,255);

  cursor_pos = x;
  win_outs(win_console,10+8*cursor_pos,10+(active_row)*13,"_",cursor_color,255);
}

void console_printchar(unsigned char ch)
{
  if (!console_out)
    return;

  win_outs(win_console,10+8*cursor_pos,10+(active_row)*13,"_",COLOR_LIGHTGRAY,255);

  switch (ch)
  {
    case '\n':
    case 13: console_add_line("\0");
             break;
    case  8: console_backspace();
             break;
    default: console_add_char(ch);
             cursor_pos++;
             break;
  }
}

void console_printf(char *fmt, ...)
{
//  char buf[255]={0};
  char *buf;
  va_list args;
  struct ipc_message m;

  buf = (char*) valloc(256);
  memset(buf,0,255);

  va_start(args, fmt);
  vsprintf(buf,fmt,args);
  va_end(args);

  m.type = MSG_CONSOLE_PRINT;
  m.MSGT_CONSOLE_STR = buf;
  send_msg(win_console->ptask->pid, &m);
}

void console_print_str(char *str)
{
  int i=0;

  while (str[i] != '\0')
    console_printchar(str[i++]);

  console_redraw();
}


void console_cls()
{
  struct ipc_message m;

  m.type = MSG_CONSOLE_CLEAR;
  send_msg(win_console->ptask->pid, &m);
}

void console_hide_output()
{
  struct ipc_message m;

  m.type = MSG_CONSOLE_HIDE_OUTPUT;
  send_msg(win_console->ptask->pid, &m);
}

void console_show_output()
{
  struct ipc_message m;

  m.type = MSG_CONSOLE_SHOW_OUTPUT;
  send_msg(win_console->ptask->pid, &m);
}

unsigned char console_getch()
{
   struct ipc_message m;
   unsigned char ch;

   m.type = MSG_CONSOLE_GETCH;
//   m->source = ;
   send_msg(win_console->ptask->pid, &m);

   wait_msg(&m, MSG_CONSOLE_GETCH_REPLY);
   ch = m.MSGT_CONSOLE_CHAR;

   return ch;
}

void console_gets(char *str)
{
  unsigned char ch = 0;
  int i=0;

  while (ch != 13)
  {
    ch = console_getch();
    if (ch == 13)
      str[i] = 0;
    else if (ch == 8)
    {
      if (i > 0)
      {
        i--;
        str[i] = 0;
      }
      else
      {
        console_printf(" ");
      }
    }
    else
    {
      str[i] = ch;
      i++;
    }
  }

}

int wstrlen(char *str)
{
  int i = strlen(str)-1;

  for (;i>=0;i--)
  {
    if (str[i] != ' ')
      return i+1;
  }

  return 0;
}

void console_keypressed(unsigned char ch)
{
  struct ipc_message m;

  console_printchar(ch);
  console_redraw();

//  if (pass_char)
  {
    m.type = MSG_CONSOLE_GETCH_REPLY;
    m.MSGT_CONSOLE_CHAR = ch;
    send_msg(cmd_pid, &m);
    pass_char = 0;
  }
}

void console_cmd_task()
{
  struct ipc_message m;
  char str[256];

  wait_msg(&m,-1);

  while (1)
  {
    console_printf("Command: ");
    console_gets(str);
    perform_command(str);
  }

}

void console_task()
{
  struct ipc_message m;
  char str[256];
  int n=0;

  win_console = create_window("Console",50,50,400,450,WF_STANDARD);

  settimer(5);

  console_init_all();

  // autostart functions
  //show_msgf("newcon.c: automounting hd0d to skyfs...");
  //error(sys_mount("hd0d","skyfs"));


  while (1)
  {
     wait_msg(&m, -1);

     switch (m.type)
     {
       case MSG_CONSOLE_HIDE_OUTPUT:
            console_out = 0;
            break;
       case MSG_CONSOLE_SHOW_OUTPUT:
            console_out = 1;
            break;
       case MSG_CONSOLE_CLEAR:
            console_clear_all();
            console_init_all();
            break;
       case MSG_CONSOLE_PRINT:
            console_print_str(m.MSGT_CONSOLE_STR);
            vfree(m.MSGT_CONSOLE_STR);
            break;
       case MSG_CONSOLE_GETCH:
            pass_char = 1;
            break;
       case MSG_TIMER:
            cursor_color = (cursor_color != COLOR_LIGHTGRAY) ? COLOR_LIGHTGRAY : COLOR_BLACK;
            win_outs(win_console,10+8*cursor_pos,10+(active_row)*13,"_",cursor_color,255);
            break;
       case MSG_READ_CHAR_REPLY:
            console_keypressed(m.MSGT_READ_CHAR_CHAR);
            break;
       case MSG_GUI_WINDOW_CREATED:
            console_printf("Console ready. Type help\n\n");
            send_msg(cmd_pid,MSG_GUI_WINDOW_CREATED);
	    break;
       case MSG_GUI_WINDOW_CLOSE:
            task_control(cmd_pid,TASK_DEAD);
            cleartimer();
            destroy_app(win_console);
            break;
       case MSG_GUI_REDRAW:
            set_wflag(win_console, WF_STOP_OUTPUT, 0);
            set_wflag(win_console, WF_MSG_GUI_REDRAW, 0);
            console_redraw();
            break;
     }

  }

}

void nwin_task()
{
  window_t *win_nwin;
  struct ipc_message m;

  win_nwin = create_window("Test Window",20,20,300,300,WF_STANDARD);

  while (1)
  {
     wait_msg(&m, -1);

     switch (m.type)
     {
	   case MSG_GUI_WINDOW_CLOSE:
            destroy_app(win_nwin);
            break;
     }
  }
}

void cmd_login()
{
  char str_user[50], str_pwd[50];

  console_printf("Username: ");
  console_gets(str_user);

  console_printf("Password: ");
  console_hide_output();
  console_gets(str_pwd);
  console_show_output();

  if (!strcmp(str_user, str_pwd))
    console_printf("\nWelcome to SKY, user %c%s%c \n",'"',str_user,'"');
  else
    console_printf("\nWrong password\n", str_user);
}

void perform_command(char *str)
{
  IntRegs regs;
  char *str_cmd;
  char *str_par[10];
  char str2[256];
  int i,j, par_num;
  unsigned char *buffer;
  int ret;
  unsigned int addr;

  str_cmd = str;

  buffer = (unsigned char*)valloc(60000);

  i = j = par_num = 0;
  while (str_cmd[i] != '\0')
  {
    if (str_cmd[i] == ' ')
    {
      str_par[j++] = &str_cmd[i+1];
      str_cmd[i] = '\0';
    }
    i++;
  }
  par_num = j;


  if (!strcmp(str_cmd,"cls"))
  {
    console_cls();
    return;
  }
  else if (!strcmp(str_cmd,"help"))
  {
    help();
  }
  else if (!strcmp(str_cmd,"s3detect"))
  {
    int r=s3_detect();

    show_msgf("ret: %d",r);
  }
  else if (!strcmp(str_cmd,"s3init"))
  {
    int r=s3_init();

    show_msgf("ret: %d",r);
  }
  else if (!strcmp(str_cmd,"blend"))
  {
    blend_out();
    blend_in();
  }
  else if (!strcmp(str_cmd,"vesainfo"))
  {
    vesa_info();
	show_msgf("lfb adr: %x",get_lfb_adr());
  }
  else if (!strcmp(str_cmd,"pnp"))
    pnp_init();
  else if (!strcmp(str_cmd,"applist"))
    show_applist();
  else if (!strcmp(str_cmd,"killapp"))
  {
    int wid;
    window_t *win;
    char str_quest[50];

    wid = atoi(str_par[0]);
    win = get_win(wid);

    if (win)
    {
      console_printf("Closing %s (y/n)? ",win->name);
      console_gets(str_quest);
      if ((str_quest[0] == 'y') || (str_quest[0] == 'Y'))
	  {
		set_wflag(win,WF_STATE_CLOSEABLE,1);
        destroy_app(win);
	  }
    }
    else
      console_printf("Wrong id\n");
  }
  else if (!strcmp(str_cmd,"fullmove"))
  {
    if (!strcmp(str_par[0], "on"))
      window_fullmove(1);
    if (!strcmp(str_par[0], "off"))
      window_fullmove(0);
  }
  else if (!strcmp(str_cmd,"imgview"))
    imgview_init_app();
  else if (!strcmp(str_cmd,"bmp"))
    bmpviewer_init_app();
  else if (!strcmp(str_cmd,"memlist"))
    memlist();
  else if (!strcmp(str_cmd,"palloc"))
  {
    unsigned int ptr, pages;

    pages = atoi(str_par[0]);
    console_printf("Allocating %d Pages",pages);
    ptr = palloc(pages);
    if (ptr)
      console_printf("Address: 0x%00000008x  Dec: %d\n",ptr,ptr);
    else
      console_printf("Failed\n");
  }
  else if (!strcmp(str_cmd,"ctldemo"))
    ctldemo_init_app();
  else if (!strcmp(str_cmd, "videocon"))
    app_videocon();
  else if (!strcmp(str_cmd,"alert"))
  {
    show_alert("dlsa\0");
    show_alert("1234\0");
    show_alert("5678\0");
  }
  else if (!strcmp(str_cmd,"wininfo"))
    wininfo_init_app();
  else if (!strcmp(str_cmd,"testapp"))
    testapp();
  else if (!strcmp(str_cmd,"cache"))
  {
      cache_init();
  }
  else if (!strcmp(str_cmd,"div0"))
    i = 10 / 0;
  else if (!strcmp(str_cmd,"debugger"))
    debugger_init_app();

  else if (!strcmp(str_cmd,"int3"))
     asm("int $0x03");
  else if (!strcmp(str_cmd,"int8"))
     asm("int $0x08");
  else if (!strcmp(str_cmd,"lm"))
    load_module(str_par[0], str_par[1]);
  else if (!strcmp(str_cmd,"em"))
    execute_module();
  else if (!strcmp(str_cmd,"module"))
    start_module(str_par[0], str_par[1]);
  else if (!strcmp(str_cmd,"mapfile"))
    mapfile_init_app();
  else if (!strcmp(str_cmd,"skypad"))
    skypad_init_app();
  else if (!strcmp(str_cmd,"gf"))
    {
       addr = mapfile_get_function(atoi(str_par[0]), str2);
       if (!addr) printk("failure");
       else
       {
         printk("%0x:",addr);
         printk("%s",str2);
       }
    }

  else if (!strcmp(str_cmd,"actdev"))
    strcpy(current->actual_device, str_par[0]);
  else if (!strcmp(str_cmd,"srpl"))
    srpl_init_app();
  else if (!strcmp(str_cmd,"dosemu"))
    dosemu();
  else if (!strcmp(str_cmd,"mount"))
  {
    ret = sys_mount(str_par[0], str_par[1]);
    if (ret == SUCCESS)
      msgbox(ID_OK, 1, "Device mounted");
    else
    {
      msgbox(ID_OK, 1, "Mount failed.");
      error(ret);
    }
  }
  else if (!strcmp(str_cmd,"m"))
    error(sys_mount("fd1","skyfs"));
  else if (!strcmp(str_cmd,"unmount"))
    error(sys_unmount(str_par[0]));
  else if (!strcmp(str_cmd,"open"))
  {
    unsigned int mode;
    ret = -1;

    if (!strcmp(str_par[2],"t")) mode = O_TEXT;
    else
       mode = O_BINARY;

    if (!strcmp(str_par[1],"r")) ret = sys_open(str_par[0],mode | O_RDONLY);
    else if (!strcmp(str_par[1],"w")) ret = sys_open(str_par[0],mode | O_WRONLY);
    else if (!strcmp(str_par[1],"rw")) ret = sys_open(str_par[0],mode | O_RDWR);

    if (ret < 0)
    {
      msgbox(ID_OK,1,"Failed to open file");
      error(ret);
    }
    else
      msgbox(ID_OK,1,"File opened");
  }
  else if (!strcmp(str_cmd,"close"))
  {
    i = atoi(str_par[0]);
    error(sys_close(i));
  }
  else if (!strcmp(str_cmd,"read"))
  {
    i = atoi(str_par[0]);
    j = atoi(str_par[1]);

    ret = sys_read(i, buffer, j);
    if (ret < 0) error(ret);
    else
    {
      sprintf(str2,"%d bytes read",ret);
      show_msg(str2);
    }
    mem_dump(buffer,ret);
  }
  else if (!strcmp(str_cmd,"write"))
  {
    i = atoi(str_par[0]);
    j = atoi(str_par[1]);

    ret = sys_write(i, buffer, j);
    if (ret < 0) error(ret);
  }
  else if (!strcmp(str_cmd,"filelength"))
  {
    i = atoi(str_par[0]);
    j = sys_filelength(i);
    if (j < 0) error(j);

    sprintf(str2,"filelength of %d: %d bytes",i,j);
    show_msg(str2);
  }
  else if (!strcmp(str_cmd,"seek"))
  {
    i = atoi(str_par[0]);
    j = atoi(str_par[1]);
    error(sys_seek(i,j));
  }
  else if (!strcmp(str_cmd,"mounttable"))
    show_mount_table();
  else if (!strcmp(str_cmd,"protocoltable"))
    show_protocol_table();
  else if (!strcmp(str_cmd,"filetable"))
    show_file_table();

  else if (!strcmp(str_cmd,"login"))
    cmd_login();
  else if (!strcmp(str_cmd,"logout"))
    console_printf("logging out...\n");
  else if (!strcmp(str_cmd,"ps"))
    ps();
  else if (!strcmp(str_cmd,"sysmon"))
    system_monitor();
  else if (!strcmp(str_cmd,"cs"))
    cs_init_app();
  else if (!strcmp(str_cmd,"memman"))
    app_memman();
  else if (!strcmp(str_cmd,"taskman"))
    app_taskman();
  else if (!strcmp(str_cmd,"palviewer"))
    pal_viewer();
  else if (!strcmp(str_cmd,"memmon"))
    memmon_init_app();
  else if (!strcmp(str_cmd,"memstat"))
  {
    console_printf("Memory status\n");
    memstat();
  }
  else if (!strcmp(str_cmd,"freemem"))
    console_printf("Memory free: %d\n",get_freemem());
  else if (!strcmp(str_cmd,"bootlog"))
    dump_messages();
  else if (!strcmp(str_cmd,"showmsg"))
  {
    if (par_num > 0)
      show_msg(str_par[0]);
  }
  else if (!strcmp(str_cmd,"background"))
    draw_background(0,0,MAXX, MAXY, atoi(str_par[0]));

  else if (!strcmp(str_cmd,"setback"))
  {
    if (par_num == 6)
      set_backpal(atoi(str_par[0]),atoi(str_par[1]),atoi(str_par[2]),
                  atoi(str_par[3]),atoi(str_par[4]),atoi(str_par[5]));
  }
  else if (!strcmp(str_cmd,"redraw"))
    redraw_all();
  else if (!strcmp(str_cmd,"user"))
    CreateUserTask();
  else if (!strcmp(str_cmd,"devices"))
	  show_devices();
  else if (!strcmp(str_cmd,"valloc"))
  {
    unsigned int ptr1, size;

    size = atoi(str_par[0]);
    console_printf("Allocating %d Bytes",size);
    ptr1 = valloc(size);
    if (ptr1)
      console_printf("Address: 0x%00000008x  Dec: %d\n",ptr1,ptr1);
    else
      console_printf("Failed\n");
  }
  else if (!strcmp(str_cmd,"vfree"))
  {
    unsigned int ptr1;

    ptr1 = atoi(str_par[0]);
    console_printf("Freeing allocation at addr: 0x%00000008x",ptr1);

    if (!vfree(ptr1))
      console_printf("Worked\n");
    else
      console_printf("Failed\n");
  }
  else if (!strcmp(str_cmd,"test"))
  {
    int etime;
    struct time t;
    struct time t2;

    get_time(&t);
    for (i=0;i<300;i++)
    {
//      sprintf(str2,"Message number %d sended to msgwin",i+1);
//      show_msg2(str2);
      show_msgf("Message number %d sended to msgwin",i+1);
    }
    get_time(&t2);
    etime = (t2.min - t.min)*60 + (t2.sec - t.sec);
    console_printf("Elapsed seconds: %d",etime);
  }
  else if (!strcmp(str_cmd,"nwin"))
  {
    int i,num;

    if (par_num > 0)
    {
      num = atoi(str_par[0]);

      for (i=0;i<num;i++)
        CreateKernelTask(nwin_task,"nwin",NP,0);
    }
  }
  else if (!strcmp(str_cmd,"memtest"))
  {
    int *ptr, idx, jdx, etime;
    struct time t;
    struct time t2;

    ptr = (int*) valloc(5000*sizeof(int));

    console_printf("10 times allocating 5000 times 1000 bytes");
    get_time(&t);
    for (jdx=0;jdx<10;jdx++)
    {
     for (idx=0;idx<5000;idx++)
       ptr[idx] = valloc(1000);

     for (idx=0;idx<5000;idx++)
       vfree(ptr[idx]);
    }
    get_time(&t2);
    etime = (t2.min - t.min)*60 + (t2.sec - t.sec);
    console_printf("Elapsed seconds: %d\n",etime);
    vfree(ptr);
  }

/**** SOUND CARD FUNCTIONS ******/
  else if (!strcmp(str_cmd, "sbinit"))
    sb_init(atoi(str_par[0]), atoi(str_par[1]));
/*  else if (!strcmp(str_cmd, "sbtest"))
    sb_test(atoi(str_par[0]));*/
  else if (!strcmp(str_cmd, "wave"))
    waveplay_init_app();
  else if (!strcmp(str_cmd, "syssound"))
  {
    system_sound(str_par[0]);
  }
  else if (!strcmp(str_cmd, "sbclick"))
  {
    sb_play_click();
  }

  else if (!strcmp(str_cmd,"readblock"))
  {
     int ret, starttime, k;
     struct time t;
     struct time t2;
     unsigned char buffer[1024];

     ret = 0;

     i = atoi(str_par[1]);
     j = atoi(str_par[2]);
     if (str_par[3][0] == 'y')
       ret = 1;

     starttime = timerticks;
     k = 0;
     while (k<j)
     {
       readblock(str_par[0], i+k, buffer);
       if (ret) mem_dump(buffer, 512);
       k++;
     }
     console_printf("Elapsed seconds: %d",(int)((timerticks - starttime)/100));
     console_printf("Elapsed ticks  : %d",(int)((timerticks - starttime)));
   }

   else if (!strcmp(str_cmd,"fm"))
     fm_init_app();
   else if (!strcmp(str_cmd,"find"))
   {
     struct inode in;
     struct inode g;

     ret = sys_find(str_par[0],&in);
     if (ret == SUCCESS)
     {
        msgbox(ID_OK,MODAL,"Inode found");
        ret = SUCCESS;
        i=0;
        while (ret==SUCCESS)
        {
          ret = sys_get_inode(&in, &g, i);
          msgbox(ID_OK,MODAL,"%8s",g.u.fat_dir_entry.name);
          i++;
        }
     }
     else msgbox(ID_OK,MODAL,"Inode not found");
   }

   else if (!strcmp(str_cmd,"memdump"))
   {
     unsigned char buf[255];
     unsigned char *buf2;

     console_printf("Enter address to dump: ");
     console_gets(buf);

     if ((buf[0] == '0') && (buf[1] == 'x'))
     {
       i = hex2dec(buf+2);
     }
     else
       i = atoi(buf);

     console_printf("Dumping area 0x%00000008X:",i);
     mem_dump(i,200);
   }
   else if (!strcmp(str_cmd,"hex2dec"))
   {
     unsigned char buf[255];
     unsigned char *buf2;

     console_printf("Hex: ");
     console_gets(buf);

     if ((buf[0] == '0') && (buf[1] == 'x'))
     {
       i = hex2dec(buf+2);
     }
     else
       i = hex2dec(buf);

     printk("Dec is %d",i);
   }

   else if (!strcmp(str_cmd,"msgwin"))
     msg_init_app();
   else if (!strcmp(str_cmd,"shutdown"))
     shutdown();
   else if (!strcmp(str_cmd,"devicemanager"))
     app_devman();
   else if (!strcmp(str_cmd,"pci"))
       pci_init();

   else if (!strcmp(str_cmd,"terminal"))
     terminal_init_app();
   else if (!strcmp(str_cmd,"sched"))
   {
     if (!strcmp(str_par[0],"time"))
     {
       task_time = atoi(str_par[1]);
     }
    if (!strcmp(str_par[0],"wait"))
       scheduler_debug_wait = atoi(str_par[1])*1000000;
   }

   else if (!strcmp(str_cmd,"net"))
   {

     if (!strcmp(str_par[0],"tftp"))
       tftp(in_aton(str_par[1]),str_par[2]);
     if (!strcmp(str_par[0],"ftp"))
        ftp();
     if (!strcmp(str_par[0],"ping"))
     {
       icmp_echo_request(str_par[1], active_device, 0, 1);
       icmp_echo_request(str_par[1], active_device, 0, 1);
       icmp_echo_request(str_par[1], active_device, 0, 1);
       icmp_echo_request(str_par[1], active_device, 0, 1);
     }
     if (!strcmp(str_par[0],"up"))
     {
       if (!strcmp(str_par[1],"loopback"))
       {
         int ret;

         ret = atoi(str_par[2]);
         el3_open(ret);
       }
       else el3_open(0);
     }

     if (!strcmp(str_par[0],"down")) el3_close();
     if (!strcmp(str_par[0],"config")) el3_config();
     if (!strcmp(str_par[0],"debug"))
     {
       int ret;

       if (!strcmp(str_par[2],"on"))
         ret = 1;
       else
         ret = 0;

       if (!strcmp(str_par[1],"tftp"))
         tftpd_monitor_debug = ret;
       if (!strcmp(str_par[1],"arp"))
         net_arp_debug = ret;
       if (!strcmp(str_par[1],"ip"))
         net_ip_debug = ret;
       if (!strcmp(str_par[1],"icmp"))
         net_icmp_debug = ret;
       if (!strcmp(str_par[1],"eth"))
         net_eth_debug = ret;
       if (!strcmp(str_par[1],"udp"))
         net_udp_debug = ret;
       if (!strcmp(str_par[1],"tcp"))
         net_tcp_debug = ret;
     }

     if (!strcmp(str_par[0],"help")) console_printf("net: up <loopback [mode]>, down, config, debug [protocol (ip/icmp/udp/tcp/arp/eth/tftp] [on/off]");
   }

   else if (!strcmp(str_cmd,"debug"))
   {
      if (!strcmp(str_par[0],"module"))
        module_debug = atoi(str_par[1]);
   }

   else if (!strcmp(str_cmd,"ip"))
   {
     if (!strcmp(str_par[0],"config"))
        ip_config(str_par[1], str_par[2]);
     if (!strcmp(str_par[0],"stat"))
        ip_stat();
   }

  else if (!strcmp(str_cmd,"arpdump"))
     arp_dump();
  else if (!strcmp(str_cmd,"ethernet"))
     net_buff_debug();
  else if (!strcmp(str_cmd,"kill"))
    task_control(atoi(str_par[0]), TASK_DEAD);
  else if (!strcmp(str_cmd,"quicklaunch"))
    quick_init_app();
  else if (!strcmp(str_cmd,"sdesrv"))
    sdesrv_init_app();
  else if (!strcmp(str_cmd,"sdeclt"))
    sdeclt_init_app();
  else if (!strcmp(str_cmd,"unasm"))
  {
    addr = atoi(str_par[0]);

    for(i = 0; i < atoi(str_par[1]); i++)
  	 {
	  	 addr = unassemble(0x10000, addr, 0, &regs, buffer);
       printk("%s",buffer);
    }
  }

    else if (!strcmp(str_cmd,"msgbox"))
    msgbox(atoi(str_par[0]), atoi(str_par[1]),str_par[2]);
  else if (!strcmp(str_cmd,"clock"))
  {
     if (!strcmp(str_par[0],"set"))
     {
        i = atoi(str_par[1]);
        set_clock(i);
     }
     if (!strcmp(str_par[0],"sched"))
     {
       int starttime;

       starttime = timerticks;

       i = atoi(str_par[1]);

       while (i)
       {
         sys_clock_system_performance();
         i--;
       }
       console_printf("Elapsed timerticks/seconds: %d/%d",(int)(timerticks - starttime),(int)((timerticks - starttime)/18.2));
     }
   }

  else
  {
    msgbox(ID_OK,1,"Unknown or not support command.");
    //console_printf("Unknown command");
  }

  console_printf("\n");
}


void console_init_app()
{
  if (!valid_app(win_console))
  {
    /* Console Application */
    CreateKernelTask(console_task, "newcon", NP,0);

    /* Console Task: Command Interpeter */
    cmd_pid = ((struct task_struct*)CreateKernelTask((unsigned int)console_cmd_task, "cmd", NP,0))->pid;
  }
}

