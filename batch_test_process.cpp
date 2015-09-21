#include <iostream>
#include <cstdlib>
#include <string>
#include <sstream>
#include <fstream>
#include <vector>
#include <mutex>
#include <thread>
#include <functional>

using namespace std;
using namespace cv;

static string trimed(string& content)
{
	int s = content.find_first_not_of(" ");
	int e = content.find_last_not_of(" ");
	if (s != -1 && e != -1)
	{
		content = content.substr(s, e - s + 1);
	}
	return content;
}
static vector<string> getTestImages(string& srcDirPath,const bool recursive)
{
	//step1 : list images to temp file
#ifdef WIN32
	if (srcDirPath[srcDirPath.size() - 1] != '\\')
	{
		srcDirPath += '\\';
	}
	const string resultPath = ".\\files.tmp";
	const string command = "cmd /c dir " + srcDirPath + "*.* /a-d /b " + (recursive ?"/S":"")+" >" + resultPath;
	system(command.c_str());
#else
	assert(!recursive);
	if (srcDirPath[srcDirPath.size() - 1] != '/')
	{
		srcDirPath += '/';
	}
	const string resultPath = "./files.tmp";
	//find /home/allen/images/ -name "*"
	const string command = string("find ")+srcDirPath+" -name \"*\" >" + resultPath;
	system(command.c_str());
#endif //#ifdef WIN32

	//step2 : get image list from temp file
	vector<string> fileList;
	{
		ifstream infile;
		infile.open(resultPath.c_str(), ios::in);
		if (!infile.is_open())
		{
			return fileList;
		}
		while (!infile.eof())
		{
			string line;
			getline(infile, line, '\n');
			if (trimed(line).size() > 0)
			{
#ifdef WIN32
				if (recursive)
				{
					fileList.push_back(line);
				}
				else
				{
					fileList.push_back(srcDirPath + line);
				}
#else
				fileList.push_back(line);
#endif
			}
		}
	}

	//step3 : delete temp tile
#ifdef WIN32
	const string rmCmd = "cmd /c del " + resultPath;
	system(rmCmd.c_str());
#else
	const string rmCmd = "rm " + resultPath;
	system(rmCmd.c_str());
#endif //WIN32

	return fileList;
}

void thread_cb(int thread_id, mutex* mtx,const vector<string> images, int* count, int* total)
{
	auto lock_and_action = [&](function<void()> action_cb){
		lock_guard<std::mutex> locker(*mtx);
		action_cb();
	};
	lock_and_action([&](){cout << "thread[" << thread_id << "] begin , process size is " << images.size() << endl; });
	for (size_t i = 0; i < images.size(); i++)
	{
		lock_and_action([&](){
			//local progress
			cout << "thread["<<thread_id<<"] progress = " << (i+1) << "/" << images.size() << endl;
			//global progress
			cout << "global progress = " << (*count+1) << "/" << *total << endl;
			(*count)++;
		});
		const string filepath = images[i];
		//todo
		lock_and_action([&](){
			stringstream ss;
			ss<<"echo \"";
			ss<<filepath<<";";
			ss<<get_file_size(filepath.c_str())<<";";
			ss<<fontHeight<<";";
			ss<<"("<<grayImage.cols<<","<<grayImage.rows<<")"<<";";
			ss<<"\" >> image_info.txt";
			system(ss.str().c_str());
		});
	}
	lock_and_action([&](){cout << "thread[" << thread_id << "] end ." << endl; });
}
int main(int argc,char* argv[])
{
	if(argc!=3)
	{
		cerr<<"usage:\n\t"<<argv[0]<<" [images_dir] [thread_num]"<<endl;
		return -1;
	}
	const Mat src=imread("",0);
	//scan dir
	string dir=argv[1];
	int threadNum = atoi(argv[2]);
	if (threadNum <= 0)
	{
		cerr << "[thread_num] must be larger than 0!" << endl;
		return -1;
	}
	const vector<string> images = getTestImages(dir,false);
	if(images.empty())
	{
		cerr << "dir is empty!" << endl;
		return -1;
	}
	while(images.size() < threadNum)
	{
		threadNum--;
	}
	const int sizePerThread = images.size() / threadNum + 1;
	vector<thread> threads;
	//global thread vars
	int count = 0;
	int total = images.size();
	mutex mtx;
	for(int i=0;i<threadNum;i++)
	{
		int startPos = i *sizePerThread;
		int stopPos = startPos+sizePerThread;
		if (stopPos > images.size())
		{
			stopPos = images.size();
		}
		vector<string> threadProcessImages;
		std::copy(images.begin() + startPos, images.begin() + stopPos, std::back_inserter(threadProcessImages));
		threads.push_back(std::thread(thread_cb, i, &mtx,threadProcessImages, &count, &total));
	}
	for (size_t i = 0; i < threads.size();i++)
	{
		threads[i].join();
	}
	cout << "all done!" << endl;
	return 0;
}
