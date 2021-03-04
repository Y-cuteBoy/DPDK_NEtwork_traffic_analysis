#pragma once
#ifndef DBoperate
#define DBoperate
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mysql/mysql.h>
//数据库操作文件

struct recv_date {
	int year;
	int mon;
	int day;
	int hh;
	int min;
	int ss;
};

struct packageTable {
	//int package_id;
	char source_ip[20];
	char dst_ip[20];
	int source_port;
	int dst_port;
	int protocol;
	char protocol_version[10];
	int total_length;
	int ip_checksum;
	int tcp_checksum;
	//char  reserved_field[10];
	//char fragment_flag[10];
	//int fragment_offset;
	struct recv_date arrive_time;
	//int stream_belong;
};

struct streamTable {
	//int stream_id;
	double flow;
	int stream_avg;
	int package_num;
};



#endif //DBoperate