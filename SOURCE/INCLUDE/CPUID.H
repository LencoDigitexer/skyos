/************************************************************************/
/* Sky Operating System V2
/* Copyright (c) 1996 by Szeleney Robert
/*
/* Project Members: Szeleney Robert
/*                  Resl Christian
/*                  Hayrapetian Gregory
/************************************************************************/
/* File       : include\cpuid.h
/* Last Update: 04.11.1998
/* Version    : beta
/* Coded by   : Szeleney Robert
/* Docus      :
/************************************************************************/
/* Definition:
/*   Include file for pc.h to detect installed processor.
/************************************************************************/
#define PACKED __attribute__((packed))

struct cpuinfo_x86
{
	unsigned char	x86 PACKED;		/* CPU family */
	unsigned char	x86_vendor PACKED;	/* CPU vendor */
	unsigned char	x86_model PACKED;
	unsigned char	x86_mask PACKED;
	char	wp_works_ok  PACKED;	/* It doesn't on 386's */
	char	hlt_works_ok  PACKED;	/* Problems on some 486Dx4's and old 386's */
	char	hard_math  PACKED;
	char	rfu  PACKED;
	int	cpuid_level  PACKED;	/* Maximum supported CPUID level, -1=no CPUID */
	unsigned int	x86_capability  PACKED;
	char	x86_vendor_id[16]  PACKED;
	char	x86_model_id[64]  PACKED;
	int 	x86_cache_size  PACKED;  /* in KB - valid for CPUS which support this
				    call  */
	int	fdiv_bug  PACKED;
	int	f00f_bug  PACKED;
	unsigned long loops_per_sec  PACKED;
	unsigned long *pgd_quick  PACKED;
	unsigned long *pte_quick  PACKED;
	unsigned long pgtable_cache_sz  PACKED;
};

struct cpu_info
{
	unsigned 	stepping	: 4;
	unsigned 	model		: 4;
	unsigned 	family		: 4;
	unsigned	type		: 2;
	unsigned	reserved	: 18;
	unsigned	feature_flags;
	char		vendor_id[12];
	unsigned char	cache_config[16];
};

/*
 * Values of the cpu_info.family field
 */
#define CPU_FAMILY_386		3
#define CPU_FAMILY_486		4
#define CPU_FAMILY_PENTIUM	5
#define CPU_FAMILY_PENTIUM_PRO	6

/*
 * Values of the cpu_info.type field
 */
#define CPU_TYPE_ORIGINAL	0
#define CPU_TYPE_OVERDRIVE	1
#define CPU_TYPE_DUAL		2

/*
 * CPU feature_flags bit definitions.
 */
#define CPUF_ON_CHIP_FPU	0x00000001	/* On-chip floating point */
#define CPUF_VM86_EXT		0x00000002	/* Virtual mode extensions */
#define CPUF_IO_BKPTS		0x00000004	/* I/O breakpoint support */
#define CPUF_4MB_PAGES		0x00000008	/* 4MB page support */
#define CPUF_TS_COUNTER		0x00000010	/* Timestamp counter */
#define CPUF_PENTIUM_MSR	0x00000020	/* Pentium model-spec regs */
#define CPUF_PAGE_ADDR_EXT	0x00000040	/* Page address extensions */
#define CPUF_MACHINE_CHECK_EXCP	0x00000080	/* Machine check exception */
#define CPUF_CMPXCHG8B		0x00000100	/* CMPXCHG8B instruction */
#define CPUF_LOCAL_APIC		0x00000200	/* CPU contains a local APIC */
#define CPUF_MEM_RANGE_REGS	0x00001000	/* memory type range regs */
#define CPUF_PAGE_GLOBAL_EXT	0x00002000	/* page global extensions */
#define CPUF_MACHINE_CHECK_ARCH	0x00004000	/* Machine check architecture */
#define CPUF_CMOVCC		0x00008000	/* CMOVcc instructions */

/*
 * Cache configuration descriptor entry values.
 */
#define CPUC_NULL			0x00
#define CPUC_CODE_TLB_4K_4W_64E		0x01
#define CPUC_CODE_TLB_4M_4W_4E		0x02
#define CPUC_DATA_TLB_4K_4W_64E		0x03
#define CPUC_DATA_TLB_4M_4W_8E		0x04
#define CPUC_CODE_L1_32B_4W_8KB		0x06
#define CPUC_DATA_L1_32B_2W_8KB		0x0a
#define CPUC_COMB_L2_32B_4W_128KB	0x41
#define CPUC_COMB_L2_32B_4W_256KB	0x42
#define CPUC_COMB_L2_32B_4W_512KB	0x43
