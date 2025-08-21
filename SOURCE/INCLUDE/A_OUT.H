struct exec
{
   unsigned long   a_magic;        /* magic number */
   unsigned long   a_text;         /* size of text segment */
   unsigned long   a_data;         /* size of initialized data */
   unsigned long   a_bss;          /* size of uninitialized data */
   unsigned long   a_syms;         /* size of symbol table */
   unsigned long   a_entry;        /* entry point */
   unsigned long   a_trsize;       /* size of text relocation */
   unsigned long   a_drsize;       /* size of data relocation */
};

struct nlist
{
   long n_strx;
   unsigned char n_type;
   char n_other;
   short n_desc;
   unsigned long n_value;
};

#define OMAGIC 0407
#define NMAGIC 0410
#define ZMAGIC 0413
#define QMAGIC 0314
#define N_GETMAGIC(ex) \
   ( (ex).a_magic & 0xffff )

#define N_GETMAGIC_NET(ex) \
  (ntohl((ex).a_magic) & 0xffff)

/* Valid magic number check. */
#define  N_BADMAG(ex) \
(N_GETMAGIC(ex) != OMAGIC && N_GETMAGIC(ex) != NMAGIC && \
N_GETMAGIC(ex) != ZMAGIC && N_GETMAGIC(ex) != QMAGIC && \
N_GETMAGIC_NET(ex) != OMAGIC && N_GETMAGIC_NET(ex) != NMAGIC && \
N_GETMAGIC_NET(ex) != ZMAGIC && N_GETMAGIC_NET(ex) != QMAGIC)

/* We don't provide any N_???OFF macros here
because they vary too much between the different a.out variants;
it's practically impossible to create one set of macros
that works for UX, FreeBSD, NetBSD, Linux, etc.  */

