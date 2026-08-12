#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <dirent.h>
#include <time.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/select.h>
#include <sys/wait.h>
#include <syslog.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <signal.h>
#include <sys/time.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>
#include <strings.h>
#include <sys/epoll.h>
#include <sys/sendfile.h>
#include <mysql/mysql.h>

#define ARGS_CHECK(agrc, num) {if(argc != num){fprintf(stderr,"args error!\n");return -1;}}
#define ERROR_CHECK(ret, num, msg) {if(ret == num){perror(msg);return -1;}}

int main(int argc, char* argv[])
{
	// 初始化
	MYSQL* mysql = mysql_init(NULL);

	// 连接
	MYSQL* cret = mysql_real_connect(mysql, "localhost", "app_user", "05bb3a9553bacc7ba8cd389acc3ab7f08d76b818711e1e7d", "54test", 0, NULL, 0);
	if (cret == NULL) {
		fprintf(stderr, "mysql_real_connect:%s\n", mysql_error(mysql));
		return -1;
	}

	// 执行 SQL 语句
	// 使用 select 后面必须接 mysql_store_result，否则再次 select 会报错
	char sql[4096] = "select * from event";
	int qret = mysql_query(mysql, sql);
	if (qret != 0) {
		fprintf(stderr, "mysql_query:%s\n", mysql_error(mysql));
		return -1;
	}

	// 获取返回的行数与列数
	MYSQL_RES* res = mysql_store_result(mysql);// 把 rows 存起来
	printf("total row = %lu, total col = %d\n", mysql_num_rows(res), mysql_num_fields(res));

	// 取出每一行的内容
	MYSQL_ROW row;// row 是某一行字符串数组
	while ((row = mysql_fetch_row(res)) != NULL) {
		for (unsigned int i = 0; i < mysql_num_fields(res); ++i) {
			printf("%s\t", row[i]);
		}
		printf("\n");
	}

	// 关闭连接
	mysql_close(mysql);

	return 0;
}