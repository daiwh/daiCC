#include "daicc.h"

#include <string.h>
#include <Windows.h>

int main()
{
	fin=fopen("sourse.cpp","rb");
	if(!fin)
	{
		printf("Դ�ļ���ʧ��");
		system("pause");
		return 0;
	}
	init();
	get_token();
	translation_unit();
	fclose(fin);
	system("pause");
	return 1;
}
