#pragma once
#ifndef DPDKHandle
#define DPDKHandle
#include"string.h"
#include"DBoperate.h"
#include<rte_mbuf.h>
#include"rte_ip.h"
#include "rte_tcp.h"
#include"stdio.h"
#include"stdlib.h"
#include"time.h"
//#include"climits"

#define MAX_QUEUE_SIZE 10240
#define LLONG_MAX 1<<63-1

//ip�İ�ͷ��Ϣ
typedef struct ip_hdr
{
	int version : 4;//�汾
	int header_len : 4;//�ײ�����
	unsigned char tos : 8;//��������
	int total_len : 16;//�ܳ���
	int ident : 16;//��ʶ��16��λ
	int flags : 16;//3λ��ʶ+Ƭƫ�ƣ�13��λ
	unsigned char ttl : 8;//ttl����ʱ��
	unsigned char protocol : 8;//Э������
	int checksum : 16;//У���
	unsigned char sourceIP[4];//ԴIP
	unsigned char destIP[4];//Ŀ��IP
}ip_hdr;

//tcp��ͷ��Ϣ
typedef struct tcp_hdr
{
	unsigned short sport;//Դ�˿�
	unsigned short dport;//Ŀ�Ķ˿�
	unsigned int seq;//���к�
	unsigned int ack;//ȷ�Ϻ�
	unsigned char head_len;//ͷ������
	unsigned char flags;//
	unsigned short wind_size;//���ڴ�С
	unsigned short check_sum;//У���
	unsigned short urg_ptr;//����ָ��
}tcp_hdr;

//udp��ͷ��Ϣ
typedef struct udp_hdr
{
	unsigned short sport;//Դ�˿�
	unsigned short dport;//Ŀ�Ķ˿�
	unsigned short tot_len;//���ݳ���
	unsigned short check_sum;//У���
}udp_hdr;

typedef struct ipv45tuple
{
	unsigned int ip_dst;//ԴIP
	unsigned int ip_src;//Ŀ��IP
	unsigned short port_dst;//Դ�˿�
	unsigned short port_src;//Ŀ�Ķ˿�
	unsigned char  proto;//Э������
	//unsigned char sourceIP[4];
	//unsigned char destIP[4];
	//unsigned short sport;
	//unsigned short dport;
	//unsigned char protocol : 8;
}ipv45tuple;
//�洢��������С��Ԫ��



struct packet {
	long long date_s;
	//struct tm date;
	struct recv_date date;
	ipv45tuple ip_5tuple;
	struct packet *next=NULL;
	struct rte_mbuf *paket_data;
};

//tcp���ڵ�ṹ�嶨��
struct queue_head {
	long long date_s;
	ipv45tuple ip_5tuple;
	struct packet *next = NULL;
	struct packet *head = NULL;
};

//��������
struct queue_head packet_line[MAX_QUEUE_SIZE];

//�����ȶ�5Ԫ���Ƿ�һ�º���
//1:ƥ��ɹ�
//0:ƥ��ʧ��
int cmp_5stuple(ipv45tuple a, ipv45tuple b);

//packet��Ӻ���
//����򴴽�tcp������
//1:ִ�гɹ���
//0:ִ��ʧ�ܣ�
//-1:�ڴ�ʧ�ܣ�
int push_queque(struct packet);


//��ȡϵͳʱ��
long long get_sys_time_s();

recv_date get_sys_time();

//����������������
//����������Ϊ��pkts_burstΪ������ݰ�ָ��,bn_rxΪ���ݰ�
//return 1����ɹ���  
//return 0����ʧ�ܣ�
int shunt_TCP(const rte_mbuf *pkts_burst, unsigned short nb_rx);


//����ʵʱ���·������к���
//��������ʱ�������������ʱ���޳���
int del_tcp_flow(int);



//��������������
int analyze_TCP(struct packet *temp);


//




#endif // endDPDPHandle

