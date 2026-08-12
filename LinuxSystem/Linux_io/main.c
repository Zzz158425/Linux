#include "headfile.h"

#if 0
int main(int argc, char* argv[])
{
	// ./1_fread file1
	ARGS_CHECK(argc, 2);
	FILE* fp = fopen(argv[1], "r+");
	ERROR_CHECK(fp, NULL, "fopen");

	return 0;
}
#endif

#if 0
int main(int argc, char* argv[])
{
	// echo -n hello > file1
	// ./2_fread file1
	// -> buf = hello
	ARGS_CHECK(argc, 2);
	FILE* fp = fopen(argv[1], "r+");
	ERROR_CHECK(fp, NULL, "fopen");

	char buf[1024] = { 0 };
	fread(buf, 1, sizeof(buf), fp);
	printf("buf =%s\n", buf);

	return 0;
}
#endif

#if 0
int main(int argc, char* argv[])
{
	// ./3_chmod file1
	ARGS_CHECK(argc, 2);
	int ret = chmod(argv[1], 0777);
	ERROR_CHECK(ret, -1, "chmod");

	return 0;
}
#endif

#if 0
int main(int argc, char* argv[])
{
	// ./4_getcwd
	char path[1024] = { 0 };
	char* p = getcwd(path, 128);
	ERROR_CHECK(p, NULL, "getcwd");

	return 0;
}
#endif


#if 0
int main(int argc, char* argv[])
{
	// ./5_chdir dir1
	// /bin/bash cwd = day05（主进程） -> /5_chdir dir1 cwd = day05/dir1（子进程）
	// 注意，此处只能修改 ./5_chdir dir1 进程下的的路径，因为每一个命令都是主程序的一个子进程
	ARGS_CHECK(argc, 2);
	printf("before chdir, cwd = %s\n"，getcwd(NULL, 0));
	int ret = chdir(argv[1]);
	ERROR_CHECK(ret, -1, "chdir");
	printf("after chdir, cwd =%s\n", getcwd(NULL, 0));

	return 0;
}
#endif

#if 0
int main(int argc, char* argv[])
{
	// ./6_mkdir dir2
	ARGS_CHECK(argc, 2);
	int ret = mkdir(argv[1], 0777);// 生成的目录权限还要收到 umask 掩码的影响
	ERROR_CHECK(ret, -1, "mkdir");

	return 0;
}
#endif

#if 0
int main(int argc, char* argv[])
{
	// ./7_rmdir dir2
	ARGS_CHECK(argc, 2);
	int ret = rmdir(argv[1]);
	ERROR_CHECK(ret, -1, "rmdir");

	return 0;
}
#endif

#if 0
int main(int argc, char* argv[])
{
	// ./8_myls dir1
	ARGS_CHECK(argc, 2);
	DIR* dirp = opendir(argv[1]);
	ERROR_CHECK(ret, NULL, "opendir");
	struct dirent* pdirent;
	while ((pdirent = readdir(dirp)) != NULL)
	{
		printf("inide = %ld, reclen = %d, type = %d, name = %s\n",
			pdirent->d_ino, pdirent->d_reclen,
			pdirent->d_type, pdirent->d_name);
	}
	closedir(dirp);

	return 0;
}
#endif

#if 0
int main(int argc, char* argv[])
{
	// ./9_go_back dir1
	ARGS_CHECK(argc, 2);
	DIR* dirp = opendir(argv[1]);
	ERROR_CHECK(ret, NULL, "opendir");
	struct dirent* pdirent;
	long loc;
	while ((pdirent = readdir(dirp)) != NULL)
	{
		printf("inode = %ld, reclen = %d, type = %d, name = %s\n",
			pdirent->d_ino, pdirent->d_reclen,
			pdirent->d_type, pdirent->d_name);
		if (strcmp("dir2", pdirent->d_name) == 0)
		{
			loc = telldir(dirp);
		}
	}

	rewinddir(dirp);
	pdirent = readdir(dirp);
	printf("inode = %ld, reclen = %d, type = %d, name = %s\n",
		pdirent->d_ino, pdirent->d_reclen,
		pdirent->d_type, pdirent->d_name);
	closedir(dirp);

	return 0;
}
#endif

#if 0
int main(int argc, char* argv[])
{
	// ./10_myls_al dir1
	ARGS_CHECK(argc, 2);
	DIR* dirp = opendir(argv[1]);
	ERROR_CHECK(ret, NULL, "opendir");
	struct dirent* pdirent;
	while ((pdirent = readdir(dirp)) != NULL)
	{
		// 路径名拼接文件名
		char path[1024] = { 0 };
		sprintf(path, "%s%s%s", argv[1], "/", pdirent - d_name);

		struct stat statbuf;
		int ret = stat(path, &statbuf);
		ERROR_CHECK(ret, -1, "stat");

		printf("%o %ld %d %d %ld %s %s\n", 
			statbuf.st_mode,
			statbuf.st_nlink,
			statbuf.st_uid,
			statbuf.st_gid,
			statbuf.st_size,
			ctime(&statbuf.st_mtime),
			pdirent->d_name);
	}
	closedir(dirp);

	return 0;
}
#endif

#if 0
int main(int argc, char* argv[])
{
	// ./0_homework_ls_al dir1
	ARGS_CHECK(argc, 2);
	DIR* dirp = opendir(argv[1]);
	ERROR_CHECK(ret, NULL, "opendir");
	struct dirent* pdirent;
	chdir(argv[1]);
	while ((pdirent = readdir(dirp)) != NULL)
	{
		struct stat statbuf;
		int ret = stat(pdirent->d_name, &statbuf);
		ERROR_CHECK(ret, -1, "stat");
		char str[20] = { e }；//str 保存类型和权限

	}
	closedir(dirp);

	return 0;
}
#endif