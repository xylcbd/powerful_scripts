#include <numeric>
#include <iostream>
#include <vector>
#include <fstream>
#include <iostream>
#include <locale>
#include <codecvt>
#include <thread>
#include <mutex>
#include <memory>

using namespace std;

struct TEST_RESULT
{
	//input
	std::shared_ptr<std::mutex> mtx;
	vector<string> images;
	//output
	size_t processed = 0;
};
void thread_cb(int thread_id, TEST_RESULT* result)
{
	auto lock_and_action = [&](function<void()> action_cb){
		lock_guard<std::mutex> locker(*result->mtx);
		action_cb();
	};
	lock_and_action([&](){cout << "thread[" << thread_id << "] begin , process size is " << result->images.size() << endl; });
	auto fetch_task = [&]()->string{
		lock_guard<std::mutex> locker(*result->mtx);
		if (result->processed < result->images.size())
		{
			return result->images[result->processed++];
		}
		else
		{
			return "";
		}
	};
	while (true)
	{
		lock_and_action([&](){
			//global progress
			cout << "global progress = " << (result->processed + 1) << "/" << result->images.size() << endl;
		});
		const string filepath = fetch_task();
		if (filepath.empty())
		{
			break;
		}
		//process
		//todo
	}
	lock_and_action([&](){cout << "thread[" << thread_id << "] end ." << endl; });
}
void batch_test(const vector<string>& images, int threadNum = 4)
{
	if (images.empty())
	{
		cerr << "images is empty!" << endl;
		return;
	}
	while (images.size() < threadNum)
	{
		threadNum--;
	}
	const int sizePerThread = images.size() / threadNum + 1;
	vector<thread> threads;
	//global thread vars
	TEST_RESULT result;
	result.mtx = std::make_shared<mutex>();
	result.images = images;
	for (int i = 0; i < threadNum; i++)
	{
		threads.push_back(std::thread(thread_cb, i, &result));
	}
	for (size_t i = 0; i < threads.size(); i++)
	{
		threads[i].join();
	}
	cout << "all done!" << endl;
}

int main(int argc, char* argv[])
{
	const auto testImages = vector<string>{"1.jpg","2.jpg"};
	batch_test(testImages);
	return 0;
}
