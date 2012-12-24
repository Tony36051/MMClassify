#include <iostream>
#include <fstream>
#include <windows.h>
using namespace std;
int kk = 0;
void trim(string& s)
{//ȥ��ǰ��Ŀո��Լ�ʵ����һ��
    int from = s.find_first_not_of(' ');
    int rear = s.find_last_not_of(' ');
    if(from == rear)    return;//��������ж϶��ڡ� ������ַ����ᱨ����֪Ϊ��
    s = s.substr(from, rear - from + 1);
}

void makeLower(string& s)
{//תΪСд��������ʵ����һ����������ʲô�߼��ķ�������
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
	{//�ҵ�
        int nIndex = strFileName.rfind('.');
        strFileName.insert(nIndex, 1, '1');
        strResult = fileReName(strFileName, desDir);
	}
    return strResult;
}
void moveNoCover(string srcPath, string desDir)
{//���⸲��ͬ�����ǲ�ͬ���ݵ�ͼƬ���򵥻�������Ӧ������"abc.jpg"-->"abc(3).jpg"
	size_t nIndex = srcPath.rfind('\\');
	string strFileName = srcPath.substr(nIndex+1, srcPath.length()-nIndex-1);
	strFileName = fileReName(strFileName, desDir);
	string desPath = desDir + '\\' + strFileName;
	//�ı��ƶ�����ļ����������Ͳ��ظ���
	MoveFile(srcPath.c_str(), desPath.c_str());
}
int main()
{
    fstream cfgFile;
    //ͬĿ¼�����������ļ�
	cfgFile.open("MMClassify.ini");
	if(!cfgFile.is_open())
	{//�ļ������ڣ�����ԭʼ�����ļ�����������1����֪����û
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
	string line;//��ȡ�����ļ�����ʱ����
	string srcDir;//������ͼƬ��ԴĿ¼
	string gifDir;//gifĿ��Ŀ¼
	string picDir;//��ͨͼƬ��Ŀ��Ŀ¼
	string dirDir;//�����ļ���ͼƬ��Ŀ��Ŀ¼
	while(getline(cfgFile,line))//��ʵ��EOF������࣬��֪�����д����û��EOFһ����ë��
	{//��ȡ�����ļ�
		size_t pos = line.find('=');
		if(pos == string::npos)//not found������ע�ͣ����������������
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
	//��ʼ���ļ�����������·�����ļ�Ŀ¼��ṹ�����
	string searchPath = srcDir+"\\*.*";
	WIN32_FIND_DATA FindFileData;
	HANDLE hFind = FindFirstFile((char*)searchPath.c_str(), &FindFileData);
	do{
		//������ļ���(����".."��".")������������������������ֹ�ƶ�����ҳͼƬ�ļ���
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
		//dir����Ϊ������ҳ������ͼƬȫ�����и���ҳ�ļ�������һ���ļ��д��ͼƬ��
		if(strSuffix == "htm" || strSuffix == "html")
		{
			//ȡ���ļ������֣���Ҫ����Ŀ��Ŀ¼����Ϊmovefile����ֻ���ļ��������Լ�����Ŀ¼�ġ�
			//get fileTitle. eg:"Tony" is the fileTitle of "Tony.txt".
			string strFileTitle = strFileName.substr(0, nIndex);
			string strDesDir = dirDir+'\\'+strFileTitle+"_files";
			CreateDirectory(strDesDir.c_str(), NULL);
			//��ʼ���ļ�����������·����Ŀ¼��ṹ�����
			WIN32_FIND_DATA tmpFFD;
			string tmpSreachPath = srcDir+'\\'+strFileTitle+"_files\\*.*";
			HANDLE tmpHFind = FindFirstFile(tmpSreachPath.c_str(),&tmpFFD);
			//��ΪFindFirstFile�ĳ�ʼ�����ʣ�����do-while�ṹ
			do
			{
				//in this app, no dir����Ԥ�軷���£�������Ƕ��Ŀ¼������Ҳ�����еݹ�
				if(tmpFFD.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)
					continue;
				string strSrcFilePath = srcDir+'\\'+strFileTitle+"_files\\"+tmpFFD.cFileName;
				string strDesFilePath = strDesDir+'\\'+tmpFFD.cFileName;
				//����ƶ����ɹ�����ʾ�Ѿ������ظ��ģ�ֱ��ɾ��ԭͼ���ɡ�����ƶ��ɹ����Ͳ���ɾ��Դͼ�ˡ�
				if(!MoveFile(strSrcFilePath.c_str(), strDesFilePath.c_str()))
					DeleteFile(strSrcFilePath.c_str());
			}while(FindNextFile(tmpHFind, &tmpFFD));
			//���Ҫɾ��ԭ����Ŀ¼
			string strSrcDir = srcDir+'\\'+strFileTitle+"_files";
			RemoveDirectory(strSrcDir.c_str());
			//����ҳ�ļ�Ҳ�ƶ���ȥ
			string strSrcHtml = srcDir+'\\'+strFileName;
			string strDesHtml = dirDir+'\\'+strFileName;
			if(!MoveFile(strSrcHtml.c_str(), strDesHtml.c_str()))
				DeleteFile(strSrcHtml.c_str());
		}
	}while(FindNextFile(hFind, &FindFileData));
}
