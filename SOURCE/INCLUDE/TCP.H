struct s_tcphdr
{
	unsigned short	source;
	unsigned short	dest;
	unsigned int	seq;
	unsigned int	ack_seq;
	unsigned short	res1:4,
		doff:4,
		fin:1,
		syn:1,
		rst:1,
		psh:1,
		ack:1,
		urg:1,
		res2:2;
	unsigned short	window;
	unsigned short	check;
	unsigned short	urg_ptr;
};
