#include <iostream>
#include <fstream>
#include <windows.h>
using namespace std;
void trim(wstring& s)
{//ȥ��ǰ��Ŀո��Լ�ʵ����һ��
    int from = s.find_first_not_of(' ');
    int rear = s.find_last_not_of(' ');
    if(from == rear)    return;//��������ж϶��ڡ� ������ַ����ᱨ����֪Ϊ��
    s = s.substr(from, rear - from + 1);
}

void makeLower(wstring& s)
{//תΪСд��������ʵ����һ����������ʲô�߼��ķ�������
    wchar_t* tmp = new wchar_t[s.length()+1];
    wcscpy(tmp,s.c_str());
    for(unsigned short i = 0; i < s.length(); i++)
        if(tmp[i]>='A'&&tmp[i]<='Z')
            tmp[i] += 'a'-'A';
    s = tmp;
}
wstring fileReName(wstring strFileName, wstring desDir)
{//para1 path including fileName, but para2 not
	wstring desPath = desDir + L'\\' +strFileName;
	wstring strResult = strFileName;
	WIN32_FIND_DATAW FindFileData;
    HANDLE hFind = FindFirstFileW(desPath.c_str(),&FindFileData);
	if(hFind!=INVALID_HANDLE_VALUE)
	{//�ҵ�
        int nIndex = strFileName.rfind('.');
        strFileName.insert(nIndex, 1, '1');
        strResult = fileReName(strFileName, desDir);
	}
    return strResult;
}
void moveNoCover(wstring srcPath, wstring desDir)
{//���⸲��ͬ�����ǲ�ͬ���ݵ�ͼƬ���򵥻�������Ӧ������"abc.jpg"-->"abc(3).jpg"
	size_t nIndex = srcPath.rfind('\\');
	wstring strFileName = srcPath.substr(nIndex+1, srcPath.length()-nIndex-1);
	strFileName = fileReName(strFileName, desDir);
	wstring desPath = desDir + L'\\' + strFileName;
	//�ı��ƶ�����ļ����������Ͳ��ظ���
	MoveFileW(srcPath.c_str(), desPath.c_str());
}
int main()
{
    wfstream cfgFile;
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
	wstring line;//��ȡ�����ļ�����ʱ����
	wstring srcDir;//������ͼƬ��ԴĿ¼
	wstring gifDir;//gifĿ��Ŀ¼
	wstring picDir;//��ͨͼƬ��Ŀ��Ŀ¼
	wstring dirDir;//�����ļ���ͼƬ��Ŀ��Ŀ¼
	while(getline(cfgFile,line))//��ʵ��EOF������࣬��֪�����д����û��EOFһ����ë��
	{//��ȡ�����ļ�
		size_t pos = line.find('=');
		if(pos == wstring::npos)//not found������ע�ͣ����������������
            continue;
		wstring key = line.substr(0,pos);
		wstring value = line.substr(pos+1, line.length()-pos-1);
		trim(key);
		trim(value);
        if(key == L"srcDir")
            srcDir = value;
        else if(key == L"gifDir")
            gifDir = value;
        else if(key == L"picDir")
            picDir = value;
        else if(key == L"dirDir")
            dirDir = value;
	}
	//��ʼ���ļ�����������·�����ļ�Ŀ¼��ṹ�����
	wstring searchPath = srcDir+L"\\*.*";
	WIN32_FIND_DATAW FindFileData;
	HANDLE hFind = FindFirstFileW((wchar_t*)searchPath.c_str(), &FindFileData);
	do{
		//������ļ���(����".."��".")������������������������ֹ�ƶ�����ҳͼƬ�ļ���
		if(FindFileData.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)
			continue;
		//else all are files
		//get fileName
		wstring strFileName = FindFileData.cFileName;
        //get File's Suffix
		int nIndex = strFileName.rfind('.');
		wstring strSuffix = strFileName.substr(nIndex+1, strFileName.length()-nIndex-1);
		makeLower(strSuffix);
		//gif
		if(strSuffix == L"gif")
		{
			moveNoCover(srcDir+L'\\'+strFileName, gifDir);
		}
		//pic
		if(strSuffix == L"jpg" ||
           strSuffix == L"jpeg" ||
           strSuffix == L"png" ||
           strSuffix == L"bmp")
		{
			moveNoCover(srcDir+L'\\'+strFileName, picDir);
		}
		//dir����Ϊ������ҳ������ͼƬȫ�����и���ҳ�ļ�������һ���ļ��д��ͼƬ��
		if(strSuffix == L"htm" || strSuffix == L"html")
		{
			//ȡ���ļ������֣���Ҫ����Ŀ��Ŀ¼����Ϊmovefile����ֻ���ļ��������Լ�����Ŀ¼�ġ�
			//get fileTitle. eg:"Tony" is the fileTitle of "Tony.txt".
			wstring strFileTitle = strFileName.substr(0, nIndex);
			wstring strDesDir = dirDir+L'\\'+strFileTitle+L"_files";
			CreateDirectoryW(strDesDir.c_str(), NULL);
			//��ʼ���ļ�����������·����Ŀ¼��ṹ�����
			WIN32_FIND_DATAW tmpFFD;
			wstring tmpSreachPath = srcDir+L'\\'+strFileTitle+L"_files\\*.*";
			HANDLE tmpHFind = FindFirstFileW(tmpSreachPath.c_str(),&tmpFFD);
			//��ΪFindFirstFileW�ĳ�ʼ�����ʣ�����do-while�ṹ
			do
			{
				//in this app, no dir����Ԥ�軷���£�������Ƕ��Ŀ¼������Ҳ�����еݹ�
				if(tmpFFD.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)
					continue;
				wstring strSrcFilePath = srcDir+L'\\'+strFileTitle+L"_files\\"+tmpFFD.cFileName;
				wstring strDesFilePath = strDesDir+L'\\'+tmpFFD.cFileName;
				//����ƶ����ɹ�����ʾ�Ѿ������ظ��ģ�ֱ��ɾ��ԭͼ���ɡ�����ƶ��ɹ����Ͳ���ɾ��Դͼ�ˡ�
				if(!MoveFileW(strSrcFilePath.c_str(), strDesFilePath.c_str()))
					DeleteFileW(strSrcFilePath.c_str());
			}while(FindNextFileW(tmpHFind, &tmpFFD));
			//���Ҫɾ��ԭ����Ŀ¼
			wstring strSrcDir = srcDir+L'\\'+strFileTitle+L"_files";
			RemoveDirectoryW(strSrcDir.c_str());
			//����ҳ�ļ�Ҳ�ƶ���ȥ
			wstring strSrcHtml = srcDir+L'\\'+strFileName;
			wstring strDesHtml = dirDir+L'\\'+strFileName;
			if(!MoveFileW(strSrcHtml.c_str(), strDesHtml.c_str()))
				DeleteFileW(strSrcHtml.c_str());
		}
	}while(FindNextFileW(hFind, &FindFileData));
}
