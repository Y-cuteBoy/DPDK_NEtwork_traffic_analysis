#include <stdint.h>
#include <inttypes.h>
#include <rte_eal.h>
#include <rte_ethdev.h>
#include <rte_cycles.h>
#include <rte_lcore.h>
#include <rte_mbuf.h>
#include"DPDKHandle.h"
#include "rte_ip.h"
#include "rte_tcp.h"
#include "rte_udp.h"
#include "rte_ether.h"
#include <arpa/inet.h>

#define RX_RING_SIZE 128        //���ջ���С
#define TX_RING_SIZE 512        //���ͻ���С

#define NUM_MBUFS 8191
#define MBUF_CACHE_SIZE 250
#define BURST_SIZE 32



// max_rx_pkt_len���ѡ��ֻ�е�jumbo_frame��ʹ�ܵ�ʱ�����Ч��
// ����Ͱ���1500��ip����������
// mac֡Ϊ1514
static const struct rte_eth_conf port_conf_default = {
		.rxmode = {.max_rx_pkt_len = ETHER_MAX_LEN }
};

/* basicfwd.c: Basic DPDK skeleton forwarding example. */

/*
 * Initializes a given port using global settings and with the RX buffers
 * coming from the mbuf_pool passed as a parameter.
 */

 /*
		 ָ�����ڵĶ�������������ָ�����ǵ�����
		 ��tx��rx���������ϣ����û�����
 */

char *ip_transform(long int ip_addr)//���������תΪС��������
{
	char *buf = (char *)malloc(128);
	long int *ip = &ip_addr;
	unsigned char *ptr_uc = (unsigned char *)ip;
	snprintf(buf, 128, "%u.%u.%u.%u", ptr_uc[3], ptr_uc[2], ptr_uc[1], ptr_uc[0]);//��ptr_uc[3], ptr_uc[2], ptr_uc[1], ptr_uc[0]��"%u.%u.%u.%u"�ĸ�ʽ�ŵ�buf�128Ϊbuf��С
	static char ip_adr[20];
	strcpy(ip_adr, buf);
	free(buf);
	return ip_adr;
}



static inline int
port_init(uint8_t port, struct rte_mempool *mbuf_pool)
{
	struct rte_eth_conf port_conf = port_conf_default;      //��������=Ĭ�ϵ���������
	const uint16_t rx_rings = 1, tx_rings = 1;              //����tx��rx���еĸ���
	int retval;                     //��ʱ����������ֵ
	uint16_t q;    //��ʱ���������к�

	if (port >= rte_eth_dev_count())
		return -1;

	/* Configure the Ethernet device. */
	//������̫�����豸
	//���ںš����Ͷ��и��������ն��и��������ڵ�����
	retval = rte_eth_dev_configure(port, rx_rings, tx_rings, &port_conf);   //���������豸
	if (retval != 0)
		return retval;

	/* Allocate and set up 1 RX queue per Ethernet port. */
	//RX���г�ʼ��
	for (q = 0; q < rx_rings; q++) {        //����ָ�����ڵ�����rx����

			//���벢����һ���հ�����
			//ָ�����ڣ�ָ�����У�ָ������RING�Ĵ�С��ָ��SOCKET_ID�ţ�ָ������ѡ�Ĭ��NULL����ָ���ڴ��
			//����rte_eth_dev_socket_id(port)�����,ͨ��port������ȡdev_socket_id??
			//dev_socket_id����δ֪���д��о�
		retval = rte_eth_rx_queue_setup(port, q, RX_RING_SIZE,
			rte_eth_dev_socket_id(port), NULL, mbuf_pool);  //����������̫���˿ڵ�NUMA�׽��� 
		if (retval < 0)
			return retval;
	}

	//TX���г�ʼ��
	/* Allocate and set up 1 TX queue per Ethernet port. */
	for (q = 0; q < tx_rings; q++) {        //����ָ�����ڵ�����tx����

			//���벢����һ����������
			//ָ�����ڣ�ָ�����У�ָ������RING��С��ָ��SOCKET_ID�ţ�ָ��ѡ�NULLΪĬ�ϣ�
			//??TXΪ��ûָ���ڴ�أ��������д��о�
		retval = rte_eth_tx_queue_setup(port, q, TX_RING_SIZE,  //���벢����һ����������
			rte_eth_dev_socket_id(port), NULL);
		if (retval < 0)
			return retval;
	}

	/* Start the Ethernet port. */
	retval = rte_eth_dev_start(port);       //��������
	if (retval < 0)
		return retval;

	/* Display the port MAC address. */
	struct ether_addr addr;
	rte_eth_macaddr_get(port, &addr);       //��ȡ������MAC��ַ������ӡ
	printf("Port %u MAC: %02" PRIx8 " %02" PRIx8 " %02" PRIx8
		" %02" PRIx8 " %02" PRIx8 " %02" PRIx8 "\n",
		(unsigned)port,
		addr.addr_bytes[0], addr.addr_bytes[1],
		addr.addr_bytes[2], addr.addr_bytes[3],
		addr.addr_bytes[4], addr.addr_bytes[5]);

	/* Enable RX in promiscuous mode for the Ethernet device. */
	rte_eth_promiscuous_enable(port);       //��������Ϊ����ģʽ

	return 0;
}

/*
 * The lcore main. This is the main thread that does the work, reading from
 * an input port and writing to an output port.
 */
 /*
 //ҵ������ڵ�
 //__attribute__((noreturn))�÷�
 //���������޷���ֵ
 //��������lcore_main����������lcore_main�޷���ֵ
 */
 /*
 //1�����CPU�������Ƿ�ƥ��
 //2������ʹ�ñ���CPU�ͽ������������������
 //3�����ݽ��ա����͵�while(1)
 */
static __attribute__((noreturn)) void
lcore_main(void)
{
	const uint8_t nb_ports = rte_eth_dev_count();   //��������
	uint8_t port;                                   //��ʱ���������ں�
	uint32_t pack_num;



	/*
	 * Check that the port is on the same NUMA node as the polling thread
	 * for best performance.
	 * ���CPU�������Ƿ�ƥ��
	 * ��������������⣬���ҿ�����һ��������
	 */
	for (port = 0; port < nb_ports; port++)                 //������������
		if (rte_eth_dev_socket_id(port) > 0 &&          //����IF���
			rte_eth_dev_socket_id(port) !=
			(int)rte_socket_id())
			printf("WARNING, port %u is on remote NUMA node to "
				"polling thread.\n\tPerformance will "
				"not be optimal.\n", port);

	printf("\nCore %u forwarding packets. [Ctrl+C to quit]\n",
		rte_lcore_id());


	struct ipv4_hdr * ipv4_header;
	struct tcp_hdr * tcp_header;
	struct udp_hdr * udp_header;
	struct ether_hdr * ether_header;
	/* Run until the application is quit or killed. */
	/*���� ֱ�� Ӧ�ó��� �˳� �� ��kill*/
	for (;;) {
		/*
		 * Receive packets on a port and forward them on the paired
		 * port. The mapping is 0 -> 1, 1 -> 0, 2 -> 3, 3 -> 2, etc.
		 * ��Eth�������ݰ� �������͵� ETH�ϡ�
		 * ����˳��Ϊ��0 �Ľ��� �� 1�� ���ͣ�
		 *              1 �Ľ��� �� 0�� ����
		 * ÿ�����˿�Ϊһ��
		 */
		for (port = 0; port < nb_ports; port++) {       //������������

				/* Get burst of RX packets, from first port of pair. */
			struct rte_mbuf *bufs[BURST_SIZE];

			//�հ������յ�nb_tx����
			//�˿ڣ����У��հ����У����д�С
			const uint16_t nb_rx = rte_eth_rx_burst(port, 0,
				bufs, BURST_SIZE);

			if (unlikely(nb_rx == 0))
				continue;
			shunt_TCP(bufs,nb_rx)
			/*for (pack_num = 0; pack_num < nb_rx; pack_num++) {
				struct rte_mbuf *m = bufs[pack_num];
				shunt_TCP();
				rte_pktmbuf_free(m);
			}*/
		}
	}
	del_tcp_flow(0);
}

/*
 * The main function, which does initialization and calls the per-lcore
 * functions.
 */
int main(int argc, char *argv[])
{
	struct rte_mempool *mbuf_pool;  //ָ���ڴ�ؽṹ��ָ�����
	unsigned nb_ports;              //���ڸ���
	uint8_t portid;                 //���ںţ���ʱ�ı�Ǳ���

	/* Initialize the Environment Abstraction Layer (EAL). */
	int ret = rte_eal_init(argc, argv);     //��ʼ��
	if (ret < 0)
		rte_exit(EXIT_FAILURE, "Error with EAL initialization\n");

	argc -= ret;                    //??������Ī������,�ƺ�һ���ô�Ҳû��
	argv += ret;                    //??������Ī������,�ƺ�һ���ô�Ҳû��

	/* Check that there is an even number of ports to send/receive on. */
	nb_ports = rte_eth_dev_count(); //��ȡ��ǰ��Ч���ڵĸ���
	if (nb_ports < 2 || (nb_ports & 1))     //�����Ч������С��2����Ч������Ϊ����0�������
		rte_exit(EXIT_FAILURE, "Error: number of ports must be even\n");

	/* Creates a new mempool in memory to hold the mbufs. */
	/*����һ���µ��ڴ��*/
	//"MBUF_POOL"�ڴ����, NUM_MBUFS * nb_ports������,
	//�˺���Ϊrte_mempoll_create()�ķ�װ
	mbuf_pool = rte_pktmbuf_pool_create("MBUF_POOL", NUM_MBUFS * nb_ports,
		MBUF_CACHE_SIZE, 0, RTE_MBUF_DEFAULT_BUF_SIZE, rte_socket_id());
	if (mbuf_pool == NULL)
		rte_exit(EXIT_FAILURE, "Cannot create mbuf pool\n");

	//��ʼ�����е�����
	/* Initialize all ports. */
	for (portid = 0; portid < nb_ports; portid++)   //������������
		if (port_init(portid, mbuf_pool) != 0)  //��ʼ��ָ�����ڣ���Ҫ���ںź��ڴ�أ��˺���Ϊ�Զ��庯������ǰ�涨��
			rte_exit(EXIT_FAILURE, "Cannot init port %"PRIu8 "\n",
				portid);

	//����߼���������>1 ,��ӡ������Ϣ���˳����ò��϶���߼�����
	//�߼����Ŀ���ͨ�����ݲ��� -c �߼�������������
	if (rte_lcore_count() > 1)
		printf("\nWARNING: Too many lcores enabled. Only 1 used.\n");

	/* Call lcore_main on the master core only. */
	//ִ��������
	lcore_main();
	return 0;
}
