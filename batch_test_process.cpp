#include <opencv2/opencv.hpp>
#include <iostream>
#include <cstdlib>
#include <string>
#include <fstream>
#include <vector>
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
	if (srcDirPath[srcDirPath.size() - 1] != '/')
	{
		srcDirPath += '/';
	}
	const string resultPath = "./files.tmp";
	const string command = "ls "+ (recursive?"-R":"") + srcDirPath+" | sed \"s:^:`pwd`/: \" >" + resultPath;
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
				if (recursive)
				{
					fileList.push_back(line);
				}
				else{
					fileList.push_back(srcDirPath + line);
				}
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

int main(int argc,char* argv[])
{
	if(argc!=2)
	{
		cerr<<"usage:\n\t"<<argv[0]<<" [images_dir]"<<endl;
		return -1;
	}
	//scan dir
	string dir=argv[1];
	const vector<string> images = getTestImages(dir,false);
	for(size_t i=0;i<images.size();i++)
	{
		cout<<"progress = "<<i<<"/"<<images.size()<<endl;
		const string filepath=images[i];
		//todo
	}
	return 0;
}
