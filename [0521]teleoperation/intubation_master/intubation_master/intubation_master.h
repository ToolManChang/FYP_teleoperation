
// intubation_master.h : PROJECT_NAME Ӧ�ó������ͷ�ļ�
//

#pragma once

#ifndef __AFXWIN_H__
	#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
#endif

#include "resource.h"		// ������


// Cintubation_masterApp:
// �йش����ʵ�֣������ intubation_master.cpp
//

class Cintubation_masterApp : public CWinApp
{
public:
	Cintubation_masterApp();

// ��д
public:
	virtual BOOL InitInstance();

// ʵ��

	DECLARE_MESSAGE_MAP()
};

extern Cintubation_masterApp theApp;