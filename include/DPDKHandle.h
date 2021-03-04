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

//ip的包头信息
typedef struct ip_hdr
{
	int version : 4;//版本
	int header_len : 4;//首部长度
	unsigned char tos : 8;//服务类型
	int total_len : 16;//总长度
	int ident : 16;//标识（16）位
	int flags : 16;//3位标识+片偏移（13）位
	unsigned char ttl : 8;//ttl生存时间
	unsigned char protocol : 8;//协议类型
	int checksum : 16;//校验和
	unsigned char sourceIP[4];//源IP
	unsigned char destIP[4];//目的IP
}ip_hdr;

//tcp包头信息
typedef struct tcp_hdr
{
	unsigned short sport;//源端口
	unsigned short dport;//目的端口
	unsigned int seq;//序列号
	unsigned int ack;//确认号
	unsigned char head_len;//头部长度
	unsigned char flags;//
	unsigned short wind_size;//窗口大小
	unsigned short check_sum;//校验和
	unsigned short urg_ptr;//紧急指针
}tcp_hdr;

//udp包头信息
typedef struct udp_hdr
{
	unsigned short sport;//源端口
	unsigned short dport;//目的端口
	unsigned short tot_len;//数据长度
	unsigned short check_sum;//校验和
}udp_hdr;

typedef struct ipv45tuple
{
	unsigned int ip_dst;//源IP
	unsigned int ip_src;//目的IP
	unsigned short port_dst;//源端口
	unsigned short port_src;//目的端口
	unsigned char  proto;//协议类型
	//unsigned char sourceIP[4];
	//unsigned char destIP[4];
	//unsigned short sport;
	//unsigned short dport;
	//unsigned char protocol : 8;
}ipv45tuple;
//存储数据流最小单元；



struct packet {
	long long date_s;
	//struct tm date;
	struct recv_date date;
	ipv45tuple ip_5tuple;
	struct packet *next=NULL;
	struct rte_mbuf *paket_data;
};

//tcp流节点结构体定义
struct queue_head {
	long long date_s;
	ipv45tuple ip_5tuple;
	struct packet *next = NULL;
	struct packet *head = NULL;
};

//分流队列
struct queue_head packet_line[MAX_QUEUE_SIZE];

//声明比对5元组是否一致函数
//1:匹配成功
//0:匹配失败
int cmp_5stuple(ipv45tuple a, ipv45tuple b);

//packet入队函数
//加入或创建tcp流队列
//1:执行成功；
//0:执行失败；
//-1:内存失败；
int push_queque(struct packet);


//获取系统时间
long long get_sys_time_s();

recv_date get_sys_time();

//声明分流器函数；
//各参数作用为：pkts_burst为存放数据包指针,bn_rx为数据包
//return 1处理成功；  
//return 0处理失败；
int shunt_TCP(const rte_mbuf *pkts_burst, unsigned short nb_rx);


//声明实时更新分流队列函数
//当满队列时，采用最晚更新时间剔除法
int del_tcp_flow(int);



//声明分析包函数
int analyze_TCP(struct packet *temp);


//




#endif // endDPDPHandle

