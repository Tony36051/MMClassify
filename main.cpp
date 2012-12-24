#include <iostream>
#include <fstream>
#include <windows.h>
using namespace std;
int kk = 0;
void trim(string& s)
{//去除前后的空格，自己实现了一个
    int from = s.find_first_not_of(' ');
    int rear = s.find_last_not_of(' ');
    if(from == rear)    return;//不加这个判断对于“ ”这个字符串会报错。不知为何
    s = s.substr(from, rear - from + 1);
}

void makeLower(string& s)
{//转为小写，渣渣地实现了一个，不想用什么高级的方法调用
    char* tmp = new char[s.length()+1];
    strcpy(tmp,s.c_str());
    for(unsigned short i = 0; i < s.length(); i++)
        if(tmp[i]>='A'&&tmp[i]<='Z')
            tmp[i] += 'a'-'A';
    s = tmp;
}
string fileReName(string strFileName, string desDir)
{//para1 path including fileName, but para2 not
	string desPath = desDir+'\\'+strFileName;
	string strResult = strFileName;
	WIN32_FIND_DATA FindFileData;
    HANDLE hFind = FindFirstFile(desPath.c_str(),&FindFileData);
	if(hFind!=INVALID_HANDLE_VALUE)
	{//找到
        int nIndex = strFileName.rfind('.');
        strFileName.insert(nIndex, 1, '1');
        strResult = fileReName(strFileName, desDir);
	}
    return strResult;
}
void moveNoCover(string srcPath, string desDir)
{//以免覆盖同名但是不同内容的图片，简单化，本来应该做成"abc.jpg"-->"abc(3).jpg"
	size_t nIndex = srcPath.rfind('\\');
	string strFileName = srcPath.substr(nIndex+1, srcPath.length()-nIndex-1);
	strFileName = fileReName(strFileName, desDir);
	string desPath = desDir + '\\' + strFileName;
	//改变移动后的文件名，这样就不重复了
	MoveFile(srcPath.c_str(), desPath.c_str());
}
int main()
{
    fstream cfgFile;
    //同目录下生成配置文件
	cfgFile.open("MMClassify.ini");
	if(!cfgFile.is_open())
	{//文件不存在，创建原始配置文件，返回码设1，不知有用没
		ofstream ofile("MMClassify.ini");
		ofile<<"# sourcePath"<<endl;
		ofile<<"srcDir = \n";
		ofile<<"# gif Path (Destination)\n";
		ofile<<"gifDir = \n";
		ofile<<"# common picDir (such as \"unClassified\")\n";
		ofile<<"picDir = \n";
		ofile<<"# group picDir (for example capture from webpage)\n";
		ofile<<"dirDir = \n";
		cout<<"Complete \"MMClissify.ini\" in current directry"<<endl;
		return 1;
	}
	string line;//读取配置文件的临时变量
	string srcDir;//待处理图片的源目录
	string gifDir;//gif目标目录
	string picDir;//普通图片的目标目录
	string dirDir;//整个文件夹图片的目标目录
	while(getline(cfgFile,line))//其实跟EOF方法差不多，不知道这个写法有没跟EOF一样的毛病
	{//读取配置文件
		size_t pos = line.find('=');
		if(pos == string::npos)//not found，即是注释，或其他无意义语句
            continue;
		string key = line.substr(0,pos);
		string value = line.substr(pos+1, line.length()-pos-1);
		trim(key);
		trim(value);
        if(key == "srcDir")
            srcDir = value;
        else if(key == "gifDir")
            gifDir = value;
        else if(key == "picDir")
            picDir = value;
        else if(key == "dirDir")
            dirDir = value;
	}
	//初始化文件搜索，搜索路径，文件目录项结构，句柄
	string searchPath = srcDir+"\\*.*";
	WIN32_FIND_DATA FindFileData;
	HANDLE hFind = FindFirstFile((char*)searchPath.c_str(), &FindFileData);
	do{
		//如果是文件夹(包括".."和".")，不作处理。后面再做处理，防止移动非网页图片文件夹
		if(FindFileData.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)
			continue;
		//else all are files
		//get fileName
		string strFileName = FindFileData.cFileName;
        //get File's Suffix
		int nIndex = strFileName.rfind('.');
		string strSuffix = strFileName.substr(nIndex+1, strFileName.length()-nIndex-1);
		makeLower(strSuffix);
		//gif
		if(strSuffix == "gif")
		{
			moveNoCover(srcDir+'\\'+strFileName, gifDir);
		}
		//pic
		if(strSuffix == "jpg" ||
           strSuffix == "jpeg" ||
           strSuffix == "png" ||
           strSuffix == "bmp")
		{
			moveNoCover(srcDir+'\\'+strFileName, picDir);
		}
		//dir。因为保存网页下来的图片全都是有个网页文件，配套一个文件夹存放图片的
		if(strSuffix == "htm" || strSuffix == "html")
		{
			//取得文件夹名字，还要创建目标目录，因为movefile函数只对文件，不会自己建立目录的。
			//get fileTitle. eg:"Tony" is the fileTitle of "Tony.txt".
			string strFileTitle = strFileName.substr(0, nIndex);
			string strDesDir = dirDir+'\\'+strFileTitle+"_files";
			CreateDirectory(strDesDir.c_str(), NULL);
			//初始化文件搜索，还是路径、目录项结构、句柄
			WIN32_FIND_DATA tmpFFD;
			string tmpSreachPath = srcDir+'\\'+strFileTitle+"_files\\*.*";
			HANDLE tmpHFind = FindFirstFile(tmpSreachPath.c_str(),&tmpFFD);
			//因为FindFirstFile的初始化性质，采用do-while结构
			do
			{
				//in this app, no dir。在预设环境下，不会有嵌套目录，所以也不会有递归
				if(tmpFFD.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)
					continue;
				string strSrcFilePath = srcDir+'\\'+strFileTitle+"_files\\"+tmpFFD.cFileName;
				string strDesFilePath = strDesDir+'\\'+tmpFFD.cFileName;
				//如果移动不成功，表示已经有了重复的，直接删除原图即可。如果移动成功，就不用删除源图了。
				if(!MoveFile(strSrcFilePath.c_str(), strDesFilePath.c_str()))
					DeleteFile(strSrcFilePath.c_str());
			}while(FindNextFile(tmpHFind, &tmpFFD));
			//最后要删除原来的目录
			string strSrcDir = srcDir+'\\'+strFileTitle+"_files";
			RemoveDirectory(strSrcDir.c_str());
			//把网页文件也移动过去
			string strSrcHtml = srcDir+'\\'+strFileName;
			string strDesHtml = dirDir+'\\'+strFileName;
			if(!MoveFile(strSrcHtml.c_str(), strDesHtml.c_str()))
				DeleteFile(strSrcHtml.c_str());
		}
	}while(FindNextFile(hFind, &FindFileData));
}
