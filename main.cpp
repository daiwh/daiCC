#include "daicc.h"

#include <string.h>
#include <Windows.h>

int main()
{
	fin=fopen("sourse.cpp","rb");
	if(!fin)
	{
		printf("源文件打开失败");
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
