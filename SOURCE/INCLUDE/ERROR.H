/************************************************************************/
/* Sky Operating System V2
/* Copyright (c) 1996 by Szeleney Robert
/*
/* Project Members: Szeleney Robert
/*                  Resl Christian
/*                  Hayrapetian Gregory
/************************************************************************/
/* File       : include\error.h
/* Last Update: 04.11.1998
/* Version    : beta
/* Coded by   : Resl Christian, Szeleney Robert
/* Docus      :
/************************************************************************/
/* Definition:
/*   Defines all ERROR Numbers.
/************************************************************************/
#define EWRONG_FILESYS          -10

#define EBLOCK_READ_ERROR       -15
#define EBLOCK_WRITE_ERROR       -15

#define ECREATE_FILE            -17
#define EFILE_EXISTS            -18
#define EFILE_NOT_FOUND         -19
#define ENOFOLDER               -20
#define ENO_SPACE_LEFT          -30

#define SUCCESS                         0
#define ERROR_DEVICE_NOT_FOUND          -1
#define ERROR_READ_BLOCK                -2
#define ERROR_WRITE_BLOCK               -3
#define ERROR_DEVICE_MOUNTED            -4
#define ERROR_DEVICE_NOT_MOUNTED        -5
#define ERROR_PROTOCOL_REGISTERED       -6
#define ERROR_PROTOCOL_NOT_REGISTERED   -7
#define ERROR_WRONG_PROTOCOL            -8
#define ERROR_FILE_NOT_FOUND            -9
#define ERROR_DIRECTORY_NOT_FOUND       -10
#define ERROR_CREATE_FILE               -11
#define ERROR_GENERAL_FAILURE           -20
#define ERROR_DEVICE_NOT_INSTALLED      -21
#define ERROR_NO_MEM                    -22
#define ERROR_FILE_NOT_OPEN             -23
#define ERROR_FILE_OPEN                 -24
#define ERROR_FILE_USED                 -25
#define ERROR_WRONG_OFFSET              -26
#define ERROR_TIMEOUT                   -50


