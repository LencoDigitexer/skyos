/************************************************************************/
/* Sky Operating System V2
/* Copyright (c) 1996 by Szeleney Robert
/*
/* Project Members: Szeleney Robert
/*                  Resl Christian
/*                  Hayrapetian Gregory
/************************************************************************/
/* File       : include\german.h
/* Last Update: 04.11.1998
/* Version    : beta
/* Coded by   : Szeleney Robert
/* Docus      :
/************************************************************************/
/* Definition:
/*   Defines the german keyboard layout.
/************************************************************************/
#include "msdos.h"
#define	C(c)	((c) & 0x1F)	/* Map to control code		*/
#define A(c)	((c) | 0x80)	/* Set eight bit (ALT)		*/
#define CA(c)	A(C(c))		/* Control-Alt			*/
#define	L(c)	((c) | HASCAPS)	/* Add "Caps Lock has effect" attribute */

#define EXT	0x0100		/* Normal function keys		*/
#define CTRL	29
#define SHIFT   42
#define RSHIFT	54		/* Shift key			*/
#define LSHIFT	42		/* Shift key			*/
#define ALT	56
#define EXTKEY	0x1000		/* extended keycode		*/

#define HASCAPS	0x8000		/* Caps Lock has effect		*/

/* Numeric keypad */
#define HOME	(0x01 + EXT)
#define END	(0x02 + EXT)
#define UP	(0x03 + EXT)
#define DOWN	(0x04 + EXT)
#define LEFT	(0x05 + EXT)
#define RIGHT	(0x06 + EXT)
#define PGUP	(0x07 + EXT)
#define PGDN	(0x08 + EXT)
#define MID	(0x09 + EXT)
#define NMIN	(0x0A + EXT)
#define PLUS	(0x0B + EXT)
#define INSRT	(0x0C + EXT)

/* Alt + Numeric keypad */
#define AHOME	(0x01 + ALT)
#define AEND	(0x02 + ALT)
#define AUP	(0x03 + ALT)
#define ADOWN	(0x04 + ALT)
#define ALEFT	(0x05 + ALT)
#define ARIGHT	(0x06 + ALT)
#define APGUP	(0x07 + ALT)
#define APGDN	(0x08 + ALT)
#define AMID	(0x09 + ALT)
#define ANMIN	(0x0A + ALT)
#define APLUS	(0x0B + ALT)
#define AINSRT	(0x0C + ALT)

/* Ctrl + Numeric keypad */
#define CHOME	(0x01 + CTRL)
#define CEND	(0x02 + CTRL)
#define CUP	(0x03 + CTRL)
#define CDOWN	(0x04 + CTRL)
#define CLEFT	(0x05 + CTRL)
#define CRIGHT	(0x06 + CTRL)
#define CPGUP	(0x07 + CTRL)
#define CPGDN	(0x08 + CTRL)
#define CMID	(0x09 + CTRL)
#define CNMIN	(0x0A + CTRL)
#define CPLUS	(0x0B + CTRL)
#define CINSRT	(0x0C + CTRL)

/* Lock keys */
#define CALOCK	(0x0D + EXT)	/* caps lock	*/
#define	NLOCK	(0x0E + EXT)	/* number lock	*/
#define SLOCK	(0x0F + EXT)	/* scroll lock	*/

/* Function keys */
#define F1	(0x10 + EXT)
#define F2	(0x11 + EXT)
#define F3	(0x12 + EXT)
#define F4	(0x13 + EXT)
#define F5	(0x14 + EXT)
#define F6	(0x15 + EXT)
#define F7	(0x16 + EXT)
#define F8	(0x17 + EXT)
#define F9	(0x18 + EXT)
#define F10	(0x19 + EXT)
#define F11	(0x1A + EXT)
#define F12	(0x1B + EXT)

/* Alt+Fn */
#define AF1	(0x10 + ALT)
#define AF2	(0x11 + ALT)
#define AF3	(0x12 + ALT)
#define AF4	(0x13 + ALT)
#define AF5	(0x14 + ALT)
#define AF6	(0x15 + ALT)
#define AF7	(0x16 + ALT)
#define AF8	(0x17 + ALT)
#define AF9	(0x18 + ALT)
#define AF10	(0x19 + ALT)
#define AF11	(0x1A + ALT)
#define AF12	(0x1B + ALT)

/* Ctrl+Fn */
#define CF1	(0x10 + CTRL)
#define CF2	(0x11 + CTRL)
#define CF3	(0x12 + CTRL)
#define CF4	(0x13 + CTRL)
#define CF5	(0x14 + CTRL)
#define CF6	(0x15 + CTRL)
#define CF7	(0x16 + CTRL)
#define CF8	(0x17 + CTRL)
#define CF9	(0x18 + CTRL)
#define CF10	(0x19 + CTRL)
#define CF11	(0x1A + CTRL)
#define CF12	(0x1B + CTRL)

/* Shift+Fn */
#define SF1	(0x10 + SHIFT)
#define SF2	(0x11 + SHIFT)
#define SF3	(0x12 + SHIFT)
#define SF4	(0x13 + SHIFT)
#define SF5	(0x14 + SHIFT)
#define SF6	(0x15 + SHIFT)
#define SF7	(0x16 + SHIFT)
#define SF8	(0x17 + SHIFT)
#define SF9	(0x18 + SHIFT)
#define SF10	(0x19 + SHIFT)
#define SF11	(0x1A + SHIFT)
#define SF12	(0x1B + SHIFT)

/* Alt+Shift+Fn */
#define ASF1	(0x10 + ALT + SHIFT)
#define ASF2	(0x11 + ALT + SHIFT)
#define ASF3	(0x12 + ALT + SHIFT)
#define ASF4	(0x13 + ALT + SHIFT)
#define ASF5	(0x14 + ALT + SHIFT)
#define ASF6	(0x15 + ALT + SHIFT)
#define ASF7	(0x16 + ALT + SHIFT)
#define ASF8	(0x17 + ALT + SHIFT)
#define ASF9	(0x18 + ALT + SHIFT)
#define ASF10	(0x19 + ALT + SHIFT)
#define ASF11	(0x1A + ALT + SHIFT)
#define ASF12	(0x1B + ALT + SHIFT)

#define MAP_COLS	6	/* Number of columns in keymap */
#define NR_SCAN_CODES	0x80	/* Number of scan codes (rows in keymap) */

typedef unsigned short keymap_t[NR_SCAN_CODES * MAP_COLS];

#define KEY_MAGIC	"KMAZ"	/* Magic number of keymap file */


unsigned char keymap[NR_SCAN_CODES * MAP_COLS] = {

/* scan-code		unsh	Shift	Alt	AltGr	Alt+Sh	Strg	*/
/* ==================================================================== */
/* 00 - none	*/	0,	0,	0,	0,	0,	0,	
/* 01 - ESC	*/	C('['),	C('['),	CA('['),C('['),	C('['),	C('['),
/* 02 - '1'	*/	'1',	'!',	A('1'),	'1',	'!',	C('A'),
/* 03 - '2'	*/	'2',	'"',	A('2'),	0375,	'@',	C('@'),
/* 04 - '3'	*/	'3',	025,	A('3'),	0374,	'#',	C('C'),
/* 05 - '4'	*/	'4',	'$',	A('4'),	'4',	'$',	C('D'),
/* 06 - '5'	*/	'5',	'%',	A('5'),	'5',	'%',	C('E'),
/* 07 - '6'	*/	'6',	'&',	A('6'),	'6',	'^',	C('^'),
/* 08 - '7'	*/	'7',	'/',	A('7'),	'{',	'&',	C('G'),
/* 09 - '8'	*/	'8',	'(',	A('8'),	'[',	'*',	C('H'),
/* 10 - '9'	*/	'9',	')',	A('9'),	']',	'(',	C('I'),
/* 11 - '0'	*/	'0',	'=',	A('0'),	'}',	')',	C('@'),
/* 12 - '-'	*/	0341,	'?',	0341,	'\\',	'_',	C('_'),
/* 13 - '='	*/	'\'',	'`',	A('\''),'=',	'+',	C('@'),
/* 14 - BS	*/	C('H'),	C('H'),	CA('H'),C('H'),	C('H'),	0177,	
/* 15 - TAB	*/	C('I'),	C('I'),	CA('I'),C('I'),	C('I'),	C('I'),
/* 16 - 'q'	*/	L('q'),	'Q',	A('q'),	'@',	'Q',	C('Q'),
/* 17 - 'w'	*/	L('w'),	'W',	A('w'),	'w',	'W',	C('W'),
/* 18 - 'e'	*/	L('e'),	'E',	A('e'),	'e',	'E',	C('E'),
/* 19 - 'r'	*/	L('r'),	'R',	A('r'),	'r',	'R',	C('R'),
/* 20 - 't'	*/	L('t'),	'T',	A('t'),	't',	'T',	C('T'),
/* 21 - 'y'	*/	L('z'),	'Z',	A('z'),	'z',	'Z',	C('Z'),
/* 22 - 'u'	*/	L('u'),	'U',	A('u'),	'u',	'U',	C('U'),
/* 23 - 'i'	*/	L('i'),	'I',	A('i'),	'i',	'I',	C('I'),
/* 24 - 'o'	*/	L('o'),	'O',	A('o'),	'o',	'O',	C('O'),
/* 25 - 'p'	*/	L('p'),	'P',	A('p'),	'p',	'P',	C('P'),
/* 26 - '['	*/	L(0201),0232,	0201,	'[',	'{',	C('['),
/* 27 - ']'	*/	'+',	'*',	A('+'),	'~',	']',	C(']'),
/* 28 - CR/LF	*/	C('M'),	C('M'),	CA('M'),C('M'),	C('M'),	C('J'),
/* 29 - Strg;-) */	CTRL,	CTRL,	CTRL,	CTRL,	CTRL,	CTRL,
/* 30 - 'a'	*/	L('a'),	'A',	A('a'),	'a',	'A',	C('A'),
/* 31 - 's'	*/	L('s'),	'S',	A('s'),	's',	'S',	C('S'),
/* 32 - 'd'	*/	L('d'),	'D',	A('d'),	'd',	'D',	C('D'),
/* 33 - 'f'	*/	L('f'),	'F',	A('f'),	'f',	'F',	C('F'),
/* 34 - 'g'	*/	L('g'),	'G',	A('g'),	'g',	'G',	C('G'),
/* 35 - 'h'	*/	L('h'),	'H',	A('h'),	'h',	'H',	C('H'),
/* 36 - 'j'	*/	L('j'),	'J',	A('j'),	'j',	'J',	C('J'),
/* 37 - 'k'	*/	L('k'),	'K',	A('k'),	'k',	'K',	C('K'),
/* 38 - 'l'	*/	L('l'),	'L',	A('l'),	'l',	'L',	C('L'),
/* 39 - ';'	*/	L(0224),0231,	0224,	';',	':',	C('@'),
/* 40 - '\''	*/	L(0204),0216,	0204,	'\'',	'"',	C('@'),
/* 41 - '`'	*/	'^',	0370,	A('^'),	'`',	'~',	C('^'),
/* 42 - SHIFT	*/	LSHIFT,	LSHIFT,	LSHIFT,	LSHIFT,	LSHIFT,	LSHIFT,
/* 43 - '\\'	*/	'#',	'\'',	A('#'),	'\\',	'|',	C('\\'),
/* 44 - 'z'	*/	L('y'),	'Y',	A('y'),	'y',	'Y',	C('Y'),
/* 45 - 'x'	*/	L('x'),	'X',	A('x'),	'x',	'X',	C('X'),
/* 46 - 'c'	*/	L('c'),	'C',	A('c'),	'c',	'C',	C('C'),
/* 47 - 'v'	*/	L('v'),	'V',	A('v'),	'v',	'V',	C('V'),
/* 48 - 'b'	*/	L('b'),	'B',	A('b'),	'b',	'B',	C('B'),
/* 49 - 'n'	*/	L('n'),	'N',	A('n'),	'n',	'N',	C('N'),
/* 50 - 'm'	*/	L('m'),	'M',	A('m'),	0346,	'M',	C('M'),
/* 51 - ','	*/	',',	';',	A(','),	',',	'<',	C('@'),
/* 52 - '.'	*/	'.',	':',	A('.'),	'.',	'>',	C('@'),
/* 53 - '/'	*/	'-',	'_',	A('-'),	'/',	'?',	C('_'),
/* 54 - SHIFT	*/	RSHIFT,	RSHIFT,	RSHIFT,	RSHIFT,	RSHIFT,	RSHIFT,
/* 55 - '*'	*/	'*',	'*',	A('*'),	'*',	'*',	C('@'),
/* 56 - ALT	*/	ALT,	ALT,	ALT,	ALT,	ALT,	ALT,
/* 57 - ' '	*/	' ',	' ',	A(' '),	' ',	' ',	C('@'),
/* 58 - CapsLck	*/	CALOCK,	CALOCK,	CALOCK,	CALOCK,	CALOCK,	CALOCK,
/* 59 - F1	*/	F1,	SF1,	AF1,	AF1,	ASF1,	CF1,
/* 60 - F2	*/	F2,	SF2,	AF2,	AF2,	ASF2,	CF2,
/* 61 - F3	*/	F3,	SF3,	AF3,	AF3,	ASF3,	CF3,
/* 62 - F4	*/	F4,	SF4,	AF4,	AF4,	ASF4,	CF4,
/* 63 - F5	*/	F5,	SF5,	AF5,	AF5,	ASF5,	CF5,
/* 64 - F6	*/	F6,	SF6,	AF6,	AF6,	ASF6,	CF6,
/* 65 - F7	*/	F7,	SF7,	AF7,	AF7,	ASF7,	CF7,
/* 66 - F8	*/	F8,	SF8,	AF8,	AF8,	ASF8,	CF8,
/* 67 - F9	*/	F9,	SF9,	AF9,	AF9,	ASF9,	CF9,
/* 68 - F10	*/	F10,	SF10,	AF10,	AF10,	ASF10,	CF10,
/* 69 - NumLock	*/	NLOCK,	NLOCK,	NLOCK,	NLOCK,	NLOCK,	NLOCK,
/* 70 - ScrLock */	SLOCK,	SLOCK,	SLOCK,	SLOCK,	SLOCK,	SLOCK,
/* 71 - Home	*/	HOME,	'7',	AHOME,	AHOME,	'7',	CHOME,	
/* 72 - CurUp	*/	UP,	'8',	AUP,	AUP,	'8',	CUP,
/* 73 - PgUp	*/	PGUP,	'9',	APGUP,	APGUP,	'9',	CPGUP,
/* 74 - '-'	*/	NMIN,	'-',	ANMIN,	ANMIN,	'-',	CNMIN,
/* 75 - Left	*/	LEFT,	'4',	ALEFT,	ALEFT,	'4',	CLEFT,
/* 76 - MID	*/	MID,	'5',	AMID,	AMID,	'5',	CMID,
/* 77 - Right	*/	RIGHT,	'6',	ARIGHT,	ARIGHT,	'6',	CRIGHT,
/* 78 - '+'	*/	PLUS,	'+',	APLUS,	APLUS,	'+',	CPLUS,
/* 79 - End	*/	END,	'1',	AEND,	AEND,	'1',	CEND,
/* 80 - Down	*/	DOWN,	'2',	ADOWN,	ADOWN,	'2',	CDOWN,
/* 81 - PgDown	*/	PGDN,	'3',	APGDN,	APGDN,	'3',	CPGDN,
/* 82 - Insert	*/	INSRT,	'0',	AINSRT,	AINSRT,	'0',	CINSRT,
/* 83 - Delete	*/	0177,	'.',	A(0177),0177,	'.',	0177,
/* 84 - Enter	*/	C('M'),	C('M'),	CA('M'),C('M'),	C('M'),	C('J'),
/* 85 - ???	*/	0,	0,	0,	0,	0,	0,
/* 86 - ???	*/	'<',	'>',	A('<'),	'|',	'>',	C('@'),
/* 87 - F11	*/	F11,	SF11,	AF11,	AF11,	ASF11,	CF11,
/* 88 - F12	*/	F12,	SF12,	AF12,	AF12,	ASF12,	CF12,
/* 89 - ???	*/	0,	0,	0,	0,	0,	0,
/* 90 - ???	*/	0,	0,	0,	0,	0,	0,
/* 91 - ???	*/	0,	0,	0,	0,	0,	0,
/* 92 - ???	*/	0,	0,	0,	0,	0,	0,
/* 93 - ???	*/	0,	0,	0,	0,	0,	0,
/* 94 - ???	*/	0,	0,	0,	0,	0,	0,
/* 95 - ???	*/	0,	0,	0,	0,	0,	0,
/* 96 - EXT_KEY	*/	EXTKEY,	EXTKEY,	EXTKEY,	EXTKEY,	EXTKEY,	EXTKEY,
/* 97 - ???	*/	0,	0,	0,	0,	0,	0,
/* 98 - ???	*/	0,	0,	0,	0,	0,	0,
/* 99 - ???	*/	0,	0,	0,	0,	0,	0,
/*100 - ???	*/	0,	0,	0,	0,	0,	0,
/*101 - ???	*/	0,	0,	0,	0,	0,	0,
/*102 - ???	*/	0,	0,	0,	0,	0,	0,
/*103 - ???	*/	0,	0,	0,	0,	0,	0,
/*104 - ???	*/	0,	0,	0,	0,	0,	0,
/*105 - ???	*/	0,	0,	0,	0,	0,	0,
/*106 - ???	*/	0,	0,	0,	0,	0,	0,
/*107 - ???	*/	0,	0,	0,	0,	0,	0,
/*108 - ???	*/	0,	0,	0,	0,	0,	0,
/*109 - ???	*/	0,	0,	0,	0,	0,	0,
/*110 - ???	*/	0,	0,	0,	0,	0,	0,
/*111 - ???	*/	0,	0,	0,	0,	0,	0,
/*112 - ???	*/	0,	0,	0,	0,	0,	0,
/*113 - ???	*/	0,	0,	0,	0,	0,	0,
/*114 - ???	*/	0,	0,	0,	0,	0,	0,
/*115 - ???	*/	0,	0,	0,	0,	0,	0,
/*116 - ???	*/	0,	0,	0,	0,	0,	0,
/*117 - ???	*/	0,	0,	0,	0,	0,	0,
/*118 - ???	*/	0,	0,	0,	0,	0,	0,
/*119 - ???	*/	0,	0,	0,	0,	0,	0,
/*120 - ???	*/	0,	0,	0,	0,	0,	0,
/*121 - ???	*/	0,	0,	0,	0,	0,	0,
/*122 - ???	*/	0,	0,	0,	0,	0,	0,
/*123 - ???	*/	0,	0,	0,	0,	0,	0,
/*124 - ???	*/	0,	0,	0,	0,	0,	0,
/*125 - ???	*/	0,	0,	0,	0,	0,	0,
/*126 - ???	*/	0,	0,	0,	0,	0,	0,
/*127 - ???	*/	0,	0,	0,	0,	0,	0
};