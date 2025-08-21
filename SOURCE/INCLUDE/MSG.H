/************************************************************************/
/* Sky Operating System V2
/* Copyright (c) 1996 by Szeleney Robert
/*
/* Project Members: Szeleney Robert
/*                  Resl Christian
/*                  Hayrapetian Gregory
/************************************************************************/
/* File       : include\msg.h
/* Last Update: 04.11.1998
/* Version    : beta
/* Coded by   : Szeleney Robert
/* Docus      :
/************************************************************************/
/* Definition:
/* Definition of all used messages and types
/************************************************************************/

/************************ Block messages and types ********************/
#define MSG_READ_BLOCK                                          1
        #define MSGT_READ_BLOCK_BLOCKNUMMER     u.lpara1
        #define MSGT_READ_BLOCK_RESULT          u.lpara3
        #define MSGT_READ_BLOCK_DEVICENAME      u.cppara1
        #define MSGT_READ_BLOCK_BUFFER          u.cppara2
        #define MSGT_READ_BLOCK_MINOR           u.lpara2
        #define MSGT_READ_BLOCK_DEVICE          u.lpara4
#define MSG_READ_BLOCK_REPLY                                    2
#define MSG_READ_BLOCK_CACHE_REPLY                              3

#define MSG_WRITE_BLOCK                                         4
        #define MSGT_WRITE_BLOCK_BLOCKNUMMER     u.lpara1
        #define MSGT_WRITE_BLOCK_RESULT          u.lpara3
        #define MSGT_WRITE_BLOCK_DEVICENAME      u.cppara1
        #define MSGT_WRITE_BLOCK_BUFFER          u.cppara2
        #define MSGT_WRITE_BLOCK_MINOR           u.lpara2
#define MSG_WRITE_BLOCK_REPLY                                   5
#define MSG_WRITE_BLOCK_CACHE_REPLY                             6

/************************ Char messages and types *********************/
#define MSG_READ_CHAR                                           10
#define MSG_READ_CHAR_REPLY                                     11
        #define MSGT_READ_CHAR_DEVICENAME      u.cppara1
        #define MSGT_READ_CHAR_CHAR            u.cpara1


#define MSG_INIT                                                255

/************************** VFS messages and types *******************/
/************************** VFS messages and types *******************/
#define MSG_MOUNT                                               20
#define MSG_MOUNT_REPLY                                         21
        #define MSGT_MOUNT_DEVICE          u.cppara1
        #define MSGT_MOUNT_PROTOCOL        u.cppara2
        #define MSGT_MOUNT_SUPERBLOCK      u.cppara3
        #define MSGT_MOUNT_ROOTINODE       u.cppara4
        #define MSGT_MOUNT_RESULT          u.lpara1
#define MSG_UNMOUNT                                             22
        #define MSGT_UNMOUNT_DEVICE        u.cppara1
        #define MSGT_UNMOUNT_RESULT        u.lpara1
#define MSG_LOOKUP                                              27
#define MSG_LOOKUP_REPLY                                        28
        #define MSGT_LOOKUP_DEVICE         u.cppara1
        #define MSGT_LOOKUP_NAME           u.cppara2
        #define MSGT_LOOKUP_ACTINODE       u.cppara3
        #define MSGT_LOOKUP_INODE          u.cppara4
        #define MSGT_LOOKUP_RESULT         u.lpara1
#define MSG_OPEN                                                29
#define MSG_OPEN_REPLY                                          30
        #define MSGT_OPEN_DEVICE           u.cppara1
        #define MSGT_OPEN_NAME             u.cppara2
        #define MSGT_OPEN_ACTINODE         u.cppara3
        #define MSGT_OPEN_INODE            u.cppara4
        #define MSGT_OPEN_ROOTINODE        u.vppara2
        #define MSGT_OPEN_MODE             u.lpara1
        #define MSGT_OPEN_RESULT           u.lpara2
#define MSG_CLOSE                                               31
        #define MSGT_CLOSE_FD               u.lpara1
        #define MSGT_CLOSE_RESULT           u.lpara2
#define MSG_FILELENGTH                                          32
        #define MSGT_FILELENGTH_FD          u.lpara1
        #define MSGT_FILELENGTH_RESULT      u.lpara3
#define MSG_SEEK                                                33
#define MSG_SEEK_REPLY                                          34
        #define MSGT_SEEK_FD                u.lpara1
        #define MSGT_SEEK_POS               u.lpara2
        #define MSGT_SEEK_RESULT            u.lpara3

#define MSG_TELL                                                35
#define MSG_TELL_REPLY                                          36
        #define MSGT_TELL_FD                u.lpara1
        #define MSGT_TELL_RESULT            u.lpara2

#define MSG_READ                                                37
#define MSG_READ_REPLY                                          38
#define MSG_WRITE                                               39
#define MSG_WRITE_REPLY                                         40
        #define MSGT_READ_DEVICE            u.cppara1
        #define MSGT_READ_BUFFER            u.cppara2
        #define MSGT_READ_INODE             u.cppara3
        #define MSGT_READ_ROOTINODE         u.cppara4
        #define MSGT_READ_FD                u.lpara1
        #define MSGT_READ_FP                u.ippara1
        #define MSGT_READ_SIZE              u.lpara3
        #define MSGT_READ_RESULT            u.lpara4
        #define MSGT_READ_MODE              u.lpara5

#define MSG_FIND                                                41
#define MSG_FIND_REPLY                                          42
        #define MSGT_FIND_DEVICE           u.cppara1
        #define MSGT_FIND_NAME             u.cppara2
        #define MSGT_FIND_ACTINODE         u.cppara3
        #define MSGT_FIND_INODE            u.cppara4
        #define MSGT_FIND_ROOTINODE        u.vppara2
        #define MSGT_FIND_RESULT           u.lpara2
#define MSG_GETINODE                                                43
#define MSG_GETINODE_REPLY                                          44
        #define MSGT_GETINODE_DEVICE           u.cppara1
        #define MSGT_GETINODE_NAME             u.cppara2
        #define MSGT_GETINODE_ACTINODE         u.cppara3
        #define MSGT_GETINODE_INODE            u.cppara4
        #define MSGT_GETINODE_ROOTINODE        u.vppara2
        #define MSGT_GETINODE_FIRST            u.lpara1
        #define MSGT_GETINODE_COUNT            u.lpara3
        #define MSGT_GETINODE_RESULT           u.lpara2
/************************** Network ******************************/
#define MSG_NET_RECEIVED                                    50

#define MSG_NET_SEND                                        55
        #define MSGT_NET_SEND_BUFFER                        u.cppara1
        #define MSGT_NET_SEND_DEST                          u.cppara2
        #define MSGT_NET_SEND_SIZE                          u.lpara1
        #define MSGT_NET_SEND_RESULT                        u.lpara3
#define MSG_NET_SEND_REPLY                                  56

#define MSG_NET_SOCKET_SOCKET                               57
        #define MSGT_NET_SOCKET_SOCKET_FAMILY               u.lpara1
        #define MSGT_NET_SOCKET_SOCKET_TYPE                 u.lpara2
        #define MSGT_NET_SOCKET_SOCKET_PROTOCOL             u.lpara3
        #define MSGT_NET_SOCKET_SOCKET_PID                  u.lpara4
        #define MSGT_NET_SOCKET_SOCKET_RESULT               u.lpara5
#define MSG_NET_SOCKET_SOCKET_REPLY                         58

#define MSG_NET_SOCKET_BIND                                 59
        #define MSGT_NET_SOCKET_BIND_SOCKET               u.lpara1
        #define MSGT_NET_SOCKET_BIND_SADDR                u.vppara1
        #define MSGT_NET_SOCKET_BIND_SIZE                 u.lpara2
        #define MSGT_NET_SOCKET_BIND_RESULT               u.lpara3
#define MSG_NET_SOCKET_BIND_REPLY                         60

#define MSG_NET_SOCKET_READ                                 61
        #define MSGT_NET_SOCKET_READ_SOCKET               u.lpara1
        #define MSGT_NET_SOCKET_READ_BUFFER               u.vppara1
        #define MSGT_NET_SOCKET_READ_SIZE                 u.lpara2
        #define MSGT_NET_SOCKET_READ_SOURCE               u.vppara2
        #define MSGT_NET_SOCKET_READ_PID                  u.lpara3
        #define MSGT_NET_SOCKET_READ_RESULT               u.lpara4
#define MSG_NET_SOCKET_READ_REPLY                               62

#define MSG_NET_SOCKET_WRITE                                63
        #define MSGT_NET_SOCKET_WRITE_SOCKET               u.lpara1
        #define MSGT_NET_SOCKET_WRITE_BUFFER               u.vppara1
        #define MSGT_NET_SOCKET_WRITE_SIZE                 u.lpara2
        #define MSGT_NET_SOCKET_WRITE_SOURCE               u.vppara2
        #define MSGT_NET_SOCKET_WRITE_RESULT               u.lpara3
#define MSG_NET_SOCKET_WRITE_REPLY                          64

#define MSG_NET_SOCKET_CONNECT                              65
        #define MSGT_NET_SOCKET_CONNECT_SOCKET               u.lpara1
        #define MSGT_NET_SOCKET_CONNECT_SOURCE               u.vppara2
        #define MSGT_NET_SOCKET_CONNECT_RESULT               u.lpara3
#define MSG_NET_SOCKET_CONNECT_REPLY                        66

#define MSG_CLOCK_SYSTEM_PERFORMANCE                            90

#define MSG_CREATE_NEW_APPLICATION                              100
        #define MSGT_PROGRAMM_ADDRESS         u.lpara1
        #define MSGT_PROGRAMM_NAME            u.cppara1
        #define MSGT_CREATE_PROGRAMM_RESULT   u.lpara2
        #define MSGT_PROGRAMM_PRIVILEGE       u.lpara3


/************************** GUI messages and types *******************/
/************************** GUI messages and types *******************/
#define MSGT_WINDOW                         u.vppara1

#define MSG_OLDGUI_CREATE_WINDOW                                   150
        #define MSGT_GUI_CREATE_WINDOW_TASK   u.vppara2
#define MSG_OLDGUI_CREATE_WINDOW_REPLY                             151

#define MSG_GUI_WINDOW_REDRAW                                   152
#define MSG_GUI_WINDOW_CONFIG                                   153

#define MSG_GUI_ADD_ITEM                                        154
        #define MSGT_ITEM                     u.vppara2

#define MSG_GUI_DUMP                                            155

#define MSG_GUI_DRAW_STRING                                     156
        #define MSGT_GUI_DRAW_STRING_STR    u.vppara2

/************************ Mouse messages ****************************/
#define MSG_MOUSE_INT                                            160
#define MSG_MOUSE_MOVE                                           161
        #define MSGT_MOUSE_X                     u.lpara1
        #define MSGT_MOUSE_Y                     u.lpara2

#define MSG_MOUSE_BUT1_PRESSED                                   162
#define MSG_MOUSE_BUT2_PRESSED                                   163
#define MSG_MOUSE_BUT1_RELEASED                                  164
#define MSG_MOUSE_BUT2_RELEASED                                  165

#define MSG_GUI_REDRAW                                           170
#define MSG_GUI_REDRAW_ALL                                       171
#define MSG_GUI_REDRAW_WINDOW                                    172
#define MSG_GUI_WINDOW_CREATED                                   173
#define MSG_GUI_WINDOW_CLOSE                                     174
#define MSG_GUI_WINDOW_MOVED                                     175
#define MSG_GUI_WINDOW_SIZED                                     176
#define MSG_GUI_WINDOW_FOCUS_LOST                                177
#define MSG_GUI_WINDOW_FOCUS_GET                                 178
#define MSG_GUI_CREATE_WINDOW                                    179
        #define MSGT_GUI_X                       u.lpara2
        #define MSGT_GUI_Y                       u.lpara3
        #define MSGT_GUI_LENGTH                  u.lpara4
        #define MSGT_GUI_WIDTH                   u.lpara5
        #define MSGT_GUI_TASK                    u.vppara1
        #define MSGT_GUI_WNAME                   u.cppara1
        #define MSGT_GUI_WINADR                  u.vppara2
        #define MSGT_GUI_FLAGS                   u.cpara1
#define MSG_GUI_CLOSE_WIN                                        180
        #define MSGT_GUI_WINDOW                  u.vppara1

#define MSG_GUI_WINDOW_CLOSE_REPLY                               181
#define MSG_GUI_WINDOW_SWITCH					  182
#define MSG_GUI_DRAW_VLINE                                       183
        #define MSGT_GUI_X1                      u.lpara1
        #define MSGT_GUI_Y1                      u.lpara2
        #define MSGT_GUI_X2                      u.lpara3
        #define MSGT_GUI_Y2                      u.lpara4
        #define MSGT_GUI_COLOR                   u.lpara5
#define MSG_GUI_REDRAW_EX                                        184
        #define MSGT_GUI_RECT                    u.vppara1


/************************ Message Window messages ********************/
#define MSG_SHOW_MSG                                             202
        #define MSGT_STRING_ADDRESS              u.lpara1

/**************************** Console messages ***********************/
#define MSG_CONSOLE_GETCH                                        203
#define MSG_CONSOLE_GETCH_REPLY                                  204
        #define MSGT_CONSOLE_CHAR                u.cpara1
#define MSG_CONSOLE_PRINT                                        205
        #define MSGT_CONSOLE_STR                 u.lpara1
#define MSG_CONSOLE_CLEAR                                        206
#define MSG_CONSOLE_HIDE_OUTPUT                                  207
#define MSG_CONSOLE_SHOW_OUTPUT                                  208

/**************************** SKY Panel messages ***********************/
#define MSG_SKYPANEL_ADD_APP                                        209
#define MSG_SKYPANEL_DEL_APP                                        210
        #define MSGT_GUI_WID                       u.lpara1


/***************************** Timer messages ************************/
#define MSG_TIMER_CLOCKS                                         300

/************************* Msgbox messages ***************************/
#define MSG_SHOW_ALERT                                           301
#define MSG_ALERT_CHECKED                                        302

#define MSG_SHOW_MSGBOX                                          301
        #define MSGT_MSGBOX_TYPE                   u.lpara2
        #define MSGT_MSGBOX_MODAL                  u.lpara3

#define MSG_TIMER                                               420
#define MSG_REPLY                                               540

#define MSG_CRITICAL                                            500

#define MSG_SOUND_PLAY_BUFFER                                   700
#define MSG_SOUND_PLAY_SYSTEM                                   700
        #define MSGT_SOUND_PLAY_BUF               u.cppara1
        #define MSGT_SOUND_PLAY_SIZE              u.lpara1
#define MSG_SOUND_PLAY_FIN                                      701

#define MSG_EXCEPTION                                           800

#define MSG_MSGBOX_REPLY                                        900

#define MSG_LOGON                                               1000
#define MSG_LOGON_REPLY                                         1001


struct ipc_message
{
  volatile int type;
  unsigned int   sender;
  unsigned int   reciever;

  struct task_struct *source;

  struct
  {
    int lpara1;
    int lpara2;
    int lpara3;
    int lpara4;
    int lpara5;
    int cpara1;
    int cpara2;
    char* cppara1;
    char* cppara2;
    char* cppara3;
    char* cppara4;
    char* cppara5;
    void *vppara1;
    void *vppara2;
    int *ippara1;
  }u;
  struct ipc_message *next;
} ;
