#include<iostream>
#include<thread>
#include<pthread.h>
#include<string>
#include<unistd.h>

void funcForStd(std::string str, int count)
{
	for (int i = 0; i < count; ++i)
	{
		std::this_thread::sleep_for(std::chrono::seconds(1));

		std::cout << "子线程：" << std::this_thread::get_id() << " " << str << std::endl;
	}
}

struct tData
{
	int count;
	std::string str;
};

// pthread 的线程函数格式必须是：void* (*)(void*)，所以参数和返回值都使用 void*
void* funcForPthread(void* arg)
{
	tData* data = static_cast<tData*>(arg);

	for (int i = 0; i < data->count; ++i)
	{
		sleep(1);
		std::cout << "子线程：" << pthread_self() << " " << data->str << std::endl;
	}

	return NULL;
}

int main() 
{
	// 获取当前机器支持的硬件并发线程数
	// 注意：返回值只是一个提示，不一定等于真实可用线程数
	int num = std::thread::hardware_concurrency();
	std::cout << num << std::endl;

	// 将 "hello std::thread!!" 字符串字面量转为 std::string，显示转换，可以直接写 "hello std::thread!!"
	std::thread th1(funcForStd, static_cast<std::string>("hello std::thread!!"), 5);

	tData data;
	data.count = 5;
	data.str = "hello pthread!!";

	pthread_t tid1;
	pthread_create(&tid1, NULL, funcForPthread, &data);

	th1.join();
	pthread_join(tid1, NULL);

	return 0;
}