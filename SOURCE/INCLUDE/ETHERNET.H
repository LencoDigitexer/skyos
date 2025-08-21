/* IEEE 802.3 Ethernet magic constants.  The frame sizes omit the preamble
   and FCS/CRC (frame check sequence). */
#define ETH_ALEN	6		/* Octets in one ethernet addr	 */
#define ETH_HLEN	14		/* Total octets in header.	 */
#define ETH_ZLEN	60		/* Min. octets in frame sans FCS */
#define ETH_DATA_LEN	1500		/* Max. octets in payload	 */
#define ETH_FRAME_LEN	1514		/* Max. octets in frame sans FCS */

struct s_ethhdr
{
  unsigned char		h_dest[6];
  unsigned char		h_source[6];
  unsigned short	h_proto;
};

/* Ethernet statistics collection data. */
struct enet_statistics
{
  int	rx_packets;			/* total packets received	*/
  int	tx_packets;			/* total packets transmitted	*/
  int	rx_errors;			/* bad packets received		*/
  int	tx_errors;			/* packet transmit problems	*/
  int	rx_dropped;			/* no space in linux buffers	*/
  int	tx_dropped;			/* no space available in linux	*/
  int	multicast;			/* multicast packets received	*/
  int	collisions;

  /* detailed rx_errors: */
  int	rx_length_errors;
  int	rx_over_errors;			/* receiver ring buff overflow	*/
  int	rx_crc_errors;			/* recved pkt with crc error	*/
  int	rx_frame_errors;		/* recv'd frame alignment error */
  int	rx_fifo_errors;			/* recv'r fifo overrun		*/
  int	rx_missed_errors;		/* receiver missed packet	*/

  /* detailed tx_errors */
  int	tx_aborted_errors;
  int	tx_carrier_errors;
  int	tx_fifo_errors;
  int	tx_heartbeat_errors;
  int	tx_window_errors;
};


