/*
 ============================================================================
 Name        : connect.c
 Author      : zengfei
 Version     :
 Copyright   : Your copyright notice
 Description : Mysql
 ============================================================================
*/
#include"DBoperate.h"
#include"DPDKHandle.h"
MYSQL *g_conn; // mysql 连接
MYSQL_RES *g_res; // mysql 记录集
MYSQL_ROW g_row; // 字符串数组，mysql 记录行

#define MAX_BUF_SIZE 1024 // 缓冲区最大字节数

const char *g_host_name = "localhost";
const char *g_user_name = "root";
const char *g_password = "123456";
const char *g_db_name = "dpdk";
const unsigned int g_db_port = 3306;

void print_mysql_error(const char *);
int init_mysql();
int InsertPackage(char *);
int QueryRecords(char *);
void PrintRes();

void print_mysql_error(const char *msg) { // 打印最后一次错误
	if (msg)
		printf("%s: %s\n", msg, mysql_error(g_conn));
	else
		puts(mysql_error(g_conn));

}


int init_mysql() { // 初始化连接
	// init the database connection
	g_conn = mysql_init(NULL);
	/*设置字符编码,可能会乱码*/
	mysql_query(g_conn, "set names gbk");
	/* connect the database */
	if (!mysql_real_connect(g_conn, g_host_name, g_user_name, g_password, g_db_name, g_db_port, NULL, 0)) // 如果失败
		return -1;

	return 0; // 返回成功
}


int InsertPackage(char * p_sql)
{
	mysql_free_result(g_res); // 释放结果集
//	char sql[MAX_BUF_SIZE];   
//	sprintf(sql, "insert into streamtable(protocol, stream_avg, package_num) values('aaa', 3, 4)");  //初始化sql语句

	if (mysql_query(g_conn, p_sql))  //插入记录
		return -1;
	else
		return 0;
}


int QueryRecords(char * p_sql)
{
	mysql_free_result(g_res); // 释放结果集
//	char sql[MAX_BUF_SIZE];
//	sprintf(sql, "select * from streamtable");  //初始化sql语句
	if (mysql_query(g_conn, p_sql))
		return -1;
	else
		return 0;

}

void PrintRes()
{
	g_res = mysql_store_result(g_conn); // 从服务器传送结果集至本地，mysql_use_result直接使用服务器上的记录集

	int iNum_rows = mysql_num_rows(g_res); // 得到记录的行数
	int iNum_fields = mysql_num_fields(g_res); // 得到记录的列数

//    printf("共%d个记录，每个记录%d字段\n", iNum_rows, iNum_fields);

	puts("stream_id\tprotocol\tstream_avg\tpackage_num\t\n");

	while ((g_row = mysql_fetch_row(g_res))) // 打印结果集
		printf("%s\t\t%s\t\t%s\t\t%s\n", g_row[0], g_row[1], g_row[2], g_row[3]); // 字段
}


int connectMysql(struct packageTable *temppackageTable, int length, struct streamTable *tempstreamTable) {
	int i; 
	//char a[] = {"bcd"};
	if (init_mysql());
	print_mysql_error(NULL);

	char sql[MAX_BUF_SIZE], *p_sql;

	p_sql = sql;
	sprintf(sql, "insert into streamtable(flow, stream_avg, package_num) values('%f', '%d', '%d')", tempstreamTable->flow, tempstreamTable->stream_avg, tempstreamTable->package_num);  //初始化sql语句
	if (InsertPackage(p_sql))  //需要的参数后面再加
		printf("insert fail:%s", mysql_error(g_conn)); 

	sprintf(sql, "select * from streamtable order by stream_id DESC limit 1");         //查询数据库中刚插入的流表号stream_id
	g_res = mysql_store_result(g_conn);
	g_row = mysql_fetch_row(g_res);
	int temp_stream_belong = g_row[0];


	for (i = 0; i < length; i++)
	{
		struct packageTable *temp = &temppackageTable[i];
		sprintf(sql, "insert into packagetable(source_ip, dst_ip, source_port, dst_port, protocol, protocol_version, total_length, ip_checksum, tcp_checksum, reserved_field, fragment_flag, fragment_offset, arrive_time, stream_belong) \
			values('%s', '%s', '%d', '%d', '%d', '%s', '%d', '%s', '%s', '%s', '%s', '%d', '%s', '%d')",temp->source_ip, temp->dst_ip,temp->source_port,temp->dst_port,temp->protocol,temp->protocol_version,temp->total_length,temp->ip_checksum,temp->tcp_checksum,temp->arrive_time, temp_stream_belong);  //初始化sql语句
		if(InsertPackage(p_sql))  //需要的参数后面再加
			printf("insert fail:%s",mysql_error(g_conn));
	}

	sprintf(sql, "select * from streamtable");

	if (QueryRecords(p_sql))  //需要的参数后面再加
	{
		printf("select fail\n");
		puts(mysql_error(g_conn));
		printf("\n");
	}

	PrintRes();

	mysql_free_result(g_res); // 释放结果集

	mysql_close(g_conn); // 关闭链接

	return 0;
}
