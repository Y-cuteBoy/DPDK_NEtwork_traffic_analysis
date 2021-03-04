#include"DPDKHandle.h"
//判断TCP五元组
int cmp_5stuple(ipv45tuple a, ipv45tuple b) {
	if (a.proto != b.proto)return 0;
	if (a.ip_src == b.ip_src&&a.ip_dst == b.ip_dst&&a.port_src == b.port_src&&a.port_dst == b.port_dst) {
		return 1;
	}
	if (a.ip_src == b.ip_src&&a.ip_dst == b.ip_src&&a.port_src == b.port_dst&&a.port_dst == b.port_src) {
		return 1;
	}
	return 0;
}
//获取系统时间_秒数
long long get_sys_time_s() {
	time_t temp_time;
	time(&temp_time);
	long long ll_current_time = (long long)temp_time;
	return ll_current_time;
}


//获取系统时间_年月日_
recv_date get_sys_time() {
	struct recv_date temp_recv_date;
	time_t temp_time;
	struct tm *p;
	time(&temp_time);
	p= localtime(&temp_time);
	temp_recv_date.year = 1900 + p->tm_year;
	temp_recv_date.mon = 1 + p->tm_mon;
	temp_recv_date.day = p->tm_mday;
	temp_recv_date.hh = p->tm_hour;
	temp_recv_date.min = p->tm_min;
	temp_recv_date.ss = p->tm_sec;
	return temp_recv_date;
}

//加入节点
int push_queque(struct packet new_packet) {
	//O(n)遍历队列看是否存在该tcp流
	//遍历过程中标记第一个NULL的队列下标，以便未找到TCp流时创建TCp流
	//1、未找到TCp流时创建TCp流 2、没有NULL位置  ->执行删除队列操作   ->创建TCP流队列
	int first_NULL_flag = -1;
	struct packet *temp = (struct packet *)malloc(sizeof(struct packet));
	temp->ip_5tuple = new_packet.ip_5tuple;
	temp->paket_data = new_packet.paket_data;
	temp->date_s = new_packet.date_s;
	temp->next = NULL;
	for (int i = 0; i < MAX_QUEUE_SIZE; i++) {
		if (NULL == packet_line[i].next) {
			if (-1 == first_NULL_flag) first_NULL_flag == i;//找到第一个空NULL位置
			continue; 
		}
		if (1 == cmp_5stuple(new_packet.ip_5tuple, packet_line[i].ip_5tuple)) {
			//分配内存插入packed;尾插法
			packet_line[i].date_s = new_packet.date_s;
			packet_line[i].next->next = temp;
			packet_line[i].next = temp;
			return 1;
		}
	}
	if (-1 != first_NULL_flag) {
		packet_line[first_NULL_flag].date_s = new_packet.date_s;
		packet_line[first_NULL_flag].next = temp;
		packet_line[first_NULL_flag].head = temp;
		return 1;
	}
	else {
		//按输入时间间距删除的tcp流
		del_tcp_flow();
		for (int i = 0; i < MAX_QUEUE_SIZE; i++) {
			if (NULL == packet_line[i].next) {
				//创建tcp流
				packet_line[i].next = temp;
				packet_line[i].head = temp;
				packet_line[i].date_s = new_packet.date_s;
				return 1;
			}
		}
	}
}
//在删除一个TCP流的过程中进行分析TCP流特征
int del_tcp_flow(int ss=1800) {
	long long ll_current_time = get_sys_time_s();
	for (int i = 0; i < MAX_QUEUE_SIZE; i++) {
		if (ll_current_time - packet_line[i].date_s >= ss) {
			//分析函数
			struct packet *temp= packet_line[i].head;
			if (temp != NULL) {
				analyze_TCP(temp);//调用分析函数；
			}
			//temp->next = packet_line[i].head;
			struct packet *temp1;
			while(temp!=NULL){
				temp1 = temp->next;
				free(temp);
				rte_pktmbuf_free(temp->paket_data);//可能会出错；
				temp = temp1;
			}
			packet_line[i].head = NULL;
			packet_line[i].next = NULL;
			packet_line[i].ip_5tuple.ip_dst = 0;
			packet_line[i].ip_5tuple.ip_src = 0;
			packet_line[i].ip_5tuple.port_dst = 0;
			packet_line[i].ip_5tuple.port_src = 0;
			packet_line[i].ip_5tuple.proto = 0;
			packet_line[i].date_s = LLONG_MAX;
			packet_line[i].ip_5tuple.ip_dst = 0;
			
		}
	}
	return 1;
}

//分析特征
int analyze_TCP(struct packet *temp) {
	struct packageTable temppackageTable[50];//只分析TCP流前50条
	struct streamTable tempstreamTable;//一条流的综合特征；
	int i = -1;
	int package_avg_length=0;
	struct ipv4_hdr * ipv4_header;
	struct ether_hdr * ether_header;
	struct tcp_hdr * tcp_header;
	int packet_start_time, packet_end_time;
	if (temp != NULL) {
		packet_start_time = temp->date_s;
	}
	while (temp != NULL) {
		packet_end_time = temp->date_s;
		++i;
		struct rte_mbuf *m = temp->paket_data;
		temppackageTable[i].total_length= m->pkt_len-sizeof(struct ipv4_hdr)-sizeof(struct ether_hdr)-sizeof(struct tcp_hdr);
		ipv4_header= (struct ipv4_hdr *)((char *)m->buf_addr + m->data_off + sizeof(struct ether_hdr));
		strcpy(temppackageTable[i].source_ip, ip_transform(rte_be_to_cpu_32(ipv4_header->src_addr)));
		strcpy(temppackageTable[i].dst_ip, ip_transform(rte_be_to_cpu_32(ipv4_header->dst_addr)));
		temppackageTable[i].ip_checksum = rte_be_to_cpu_16(ipv4_header->hdr_checksum);
		temppackageTable[i].source_port = temp->ip_5tuple.port_src;
		temppackageTable[i].dst_port = temp->ip_5tuple.port_dst;
		temppackageTable[i].protocol = 6;
		ether_header = (struct ether_hdr *)((char *)m->buf_addr + m->data_off);
		if (ether_header->ether_type == 8) {
			strcpy(temppackageTable[i].protocol_version, "IPV4");
		}
		else{
			strcpy(temppackageTable[i].protocol_version, "OTHERS");
		}
		tcp_header=(struct tcp_hdr *)((char *)m->buf_addr + m->data_off + sizeof(struct ether_hdr) + sizeof(struct ipv4_hdr));
		temppackageTable[i].tcp_checksum= tcp_header->cksum;
		temppackageTable[i].arrive_time= temp->date;
		package_avg_length += temppackageTable->total_length;
	}
	tempstreamTable.flow = (packet_end_time - packet_start_time)*1.0 / i;
	tempstreamTable.stream_avg = package_avg_length / i;
	tempstreamTable.package_num = i;
	//存数据库操作；
	connectMysql(temppackageTable, i, tempstreamTable);




}
//分流
int shunt_TCP(struct rte_mbuf **pkts_burst, uint32_t nb_rx) {
	struct ipv4_hdr * ipv4_header;
	struct ether_hdr * ether_header;
	struct tcp_hdr * tcp_header;
	for (int i = 0; i < nb_rx; i++) {
		struct packet temp;
		struct rte_mbuf *m = pkts_burst[i];
		m->buf_addr = (char *)m + sizeof(struct rte_mbuf);
		ipv4_header = (struct ipv4_hdr *)((char *)m->buf_addr + m->data_off + sizeof(struct ether_hdr));
		tcp_header = (struct tcp_hdr *)((char *)m->buf_addr + m->data_off + sizeof(struct ether_hdr) + sizeof(struct ipv4_hdr));
		if (6 != ipv4_header->next_proto_id) { continue; }//过滤非TCP包；
		ipv45tuple *ipv4_5tuple;
		temp.date_s = get_sys_time_s();
		temp.date = get_sys_time();
		temp.ip_5tuple.ip_src = ipv4_header->src_addr;
		temp.ip_5tuple.ip_dst = ipv4_header->dst_addr;
		temp.ip_5tuple.port_src = tcp_header->src_port;
		temp.ip_5tuple.port_dst = tcp_header->dst_port;
		temp.ip_5tuple.proto=6;
		temp.next = NULL;
		temp.paket_data = m; //可能会错
		push_queque(temp);
	}
	del_tcp_flow(0);//分析结束，处理所有的TCP流
}
