
// intubation_masterDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "intubation_master.h"
#include "intubation_masterDlg.h"
#include "afxdialogex.h"
#include <stdio.h>
#include <stdlib.h>

extern "C"
{
#include "hidsdi.h"
#include "setupapi.h"
};

using namespace cv;



#ifdef _DEBUG
#define new DEBUG_NEW
#endif

struct recvbuf
{
	char buf[BUFFER_SIZE];
	int flag;
};

struct sendbuf
{
	char buf[4];
	int flag;
};
recvbuf data_recv;
sendbuf data_send;
Mat recieveMat(SOCKET sockServer);
void showMatImgToWnd(CDC* , const Mat, LPVOID);
DWORD xPos;
DWORD yPos;
DWORD uPos;
char feedbackData[8];
int feedBackValue; //反馈值
CString text;
int firstTime;

// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 对话框数据
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// Cintubation_masterDlg 对话框



Cintubation_masterDlg::Cintubation_masterDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(Cintubation_masterDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void Cintubation_masterDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMBO1, m_comboDevice);
}

BEGIN_MESSAGE_MAP(Cintubation_masterDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_MESSAGE(MM_WIM_DATA,OnMM_WIM_DATA)
	ON_MESSAGE(MM_WIM_CLOSE,OnMM_WIM_CLOSE)
	ON_MESSAGE(MM_WOM_OPEN,OnMM_WOM_OPEN)
	ON_MESSAGE(MM_WOM_DONE,OnMM_WOM_DONE)
	ON_MESSAGE(MM_WOM_CLOSE,OnMM_WOM_CLOSE)
	ON_BN_CLICKED(IDC_BUTTON_LISTEN, &Cintubation_masterDlg::OnBnClickedButtonListen)
	ON_BN_CLICKED(IDC_BUTTON_OPENDEVICE, &Cintubation_masterDlg::OnBnClickedButtonOpendevice)
END_MESSAGE_MAP()


// Cintubation_masterDlg 消息处理程序

BOOL Cintubation_masterDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 设置此对话框的图标。当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码
	SearchForDevices();
	text="";
	GetDlgItem(IDC_BUTTON_LISTEN)->EnableWindow(false);
	//allocate memory for wave header
    pWaveHdr1=reinterpret_cast<PWAVEHDR>(malloc(sizeof(WAVEHDR)));
	pWaveHdr2=reinterpret_cast<PWAVEHDR>(malloc(sizeof(WAVEHDR)));
	pWaveHdrOut=reinterpret_cast<PWAVEHDR>(malloc(sizeof(WAVEHDR)));
	//allocate memory for save buffer
	for(int i=0;i<InBlocks;i++)
	{
		m_AudioDataIn[i].dwLength = 0;
		m_AudioDataIn[i].lpdata = reinterpret_cast<PBYTE>(malloc(1));

	}

	for(int i =0;i<OutBlocks;i++)
	{
		m_AudioDataOut[i].dwLength = 0;
		m_AudioDataOut[i].lpdata = reinterpret_cast<PBYTE>(malloc(1));
	}
	nAudioIn = 0;
	nAudioOut = 0;
	nSend = 0;
	nReceive = 0;

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void Cintubation_masterDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void Cintubation_masterDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR Cintubation_masterDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

//显示图像函数
void showMatImgToWnd(CDC* pdc, const Mat disimg, LPVOID lpParameter)//Mat图像加载到picture control函数
{
	Cintubation_masterDlg *pdlg = (Cintubation_masterDlg *)lpParameter; 
	
	if (disimg.empty())	return ;
	static BITMAPINFO *bitMapinfo = NULL;
	static bool First = TRUE;
	if (First)
	{
		BYTE *bitBuffer = new BYTE[40 + 4 * 256];//开辟一个内存区域
		if (bitBuffer == NULL)
		{
			return;
		}
		First = FALSE;
		memset(bitBuffer, 0, 40 + 4 * 256);
		bitMapinfo = (BITMAPINFO *)bitBuffer;
		bitMapinfo->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
		bitMapinfo->bmiHeader.biPlanes = 1;
		for (int i = 0; i<256; i++)
		{ //颜色的取值范围 (0-255)
			bitMapinfo->bmiColors[i].rgbBlue = bitMapinfo->bmiColors[i].rgbGreen = bitMapinfo->bmiColors[i].rgbRed = (BYTE)i;
		}
	}
	bitMapinfo->bmiHeader.biHeight = -disimg.rows;
	bitMapinfo->bmiHeader.biWidth = disimg.cols;
	bitMapinfo->bmiHeader.biBitCount = disimg.channels() * 8;
 
	CRect drect;
	    //pWnd指向CWnd类的一个指针 
	pdlg->GetDlgItem(IDC_STATIC_CAMERA1)->GetClientRect(&drect);
	HDC hDC = pdc->GetSafeHdc();                  //HDC是Windows的一种数据类型，是设备描述句柄；
	SetStretchBltMode(hDC, COLORONCOLOR);
	StretchDIBits(hDC,
		0,
		0,
		drect.right,  //显示窗口宽度
		drect.bottom,  //显示窗口高度
		0,
		0,
		disimg.cols,     //图像宽度
		disimg.rows,     //图像高度
		disimg.data,
		bitMapinfo,
		DIB_RGB_COLORS,
		SRCCOPY);
}
//查询设备,默认5个设备
void Cintubation_masterDlg::SearchForDevices()
{
	m_pathArray.RemoveAll();

	GUID HidGuid; //定义一个GUID的结构体保存HID设备的接口类GUID
	HidD_GetHidGuid(&HidGuid); //调用HidD_GetHidGuid函数获取HID设备的GUID

	HDEVINFO hDevInfoSet; //定义一个DEVINFO的句柄保存获取到的设备信息集合句柄
	hDevInfoSet = SetupDiGetClassDevs(&HidGuid, NULL, NULL, DIGCF_PRESENT|DIGCF_DEVICEINTERFACE); //使用完毕后要用SetupDiDestroyDeviceInfoList销毁
	if (INVALID_HANDLE_VALUE == hDevInfoSet)
	{
		MessageBox(_T("SetupDiGetClassDevs error"), _T("错误"), MB_OK|MB_ICONWARNING);
		return;
	}

	DWORD MemberIndex = 0; //对设备集合中每个设备进行列举
	SP_DEVICE_INTERFACE_DATA DevInterfaceData; //保存设备的驱动接口信息
	DevInterfaceData.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);	
	BOOL bSuccess = FALSE;
	do //在设备信息集合中获取编号MemberIndex的设备信息
	{
		bSuccess = SetupDiEnumDeviceInterfaces(hDevInfoSet, NULL, &HidGuid, MemberIndex, &DevInterfaceData);
		if (!bSuccess)
		{
			break;
		}

		CString strTemp;
		strTemp+="Device";
		//m_listLog.InsertString(-1, " ");	
		//m_listLog.InsertString(-1, strTemp);

		DWORD RequiredSize = 0; //接收保存详细信息的缓冲长度
		SetupDiGetDeviceInterfaceDetail(hDevInfoSet, &DevInterfaceData, NULL, 0, &RequiredSize, NULL); //获取信息成功则继续获取设备详细信息
		PSP_DEVICE_INTERFACE_DETAIL_DATA pDevDetailData; //定义一个指向设备详细信息的结构体指针
		pDevDetailData = (PSP_DEVICE_INTERFACE_DETAIL_DATA) malloc(RequiredSize); //分配大小为RequiredSize缓冲区用来保存设备详细信息
		if (NULL == pDevDetailData)
		{
			SetupDiDestroyDeviceInfoList(hDevInfoSet);
			//m_listLog.InsertString(-1, "malloc error");
			MemberIndex++;
			continue;
		}
		pDevDetailData->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);
		if (!SetupDiGetDeviceInterfaceDetail(hDevInfoSet, &DevInterfaceData, pDevDetailData, RequiredSize, NULL, NULL)) //再次调用获取设备详细信息
		{		
			SetupDiDestroyDeviceInfoList(hDevInfoSet);
			free(pDevDetailData); //然后销毁刚刚申请的内存
			//m_listLog.InsertString(-1, "SetupDiGetDeviceInterfaceDetail error");
			MemberIndex++;
			continue;
		}
		CString strDevicePath = pDevDetailData->DevicePath; //将设备路径复制出来后销毁申请的内存
		m_pathArray.Add(strDevicePath); 
		free(pDevDetailData);

		//使用CreateFile函数获取设备的属性，包括VID、PID、版本号等
		HANDLE hDevHandle = CreateFile(strDevicePath, NULL, FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if (INVALID_HANDLE_VALUE == hDevHandle)
		{		
			SetupDiDestroyDeviceInfoList(hDevInfoSet);
			//m_listLog.InsertString(-1, "CreateFile error");
			MemberIndex++;
			continue;
		}
		else
		{
			HIDD_ATTRIBUTES DevAttributes; //定义HIDD_ATTRIBUTES结构体保存设备属性
			DevAttributes.Size = sizeof(HIDD_ATTRIBUTES);
			if (!HidD_GetAttributes(hDevHandle, &DevAttributes)) //获取设备属性
			{
				CloseHandle(hDevHandle); //关闭刚刚打开的设备
				SetupDiDestroyDeviceInfoList(hDevInfoSet);
				//m_listLog.InsertString(-1, "HidD_GetAttributes error");
			}
			//strTemp.Format("VID: %04X    PID: %04X    VNum: %04X", DevAttributes.VendorID, DevAttributes.ProductID, DevAttributes.VersionNumber);
			//m_listLog.InsertString(-1, strTemp);
			WCHAR mString[256]; 
			char Buffer[256]; 
			HidD_GetProductString(hDevHandle, mString, sizeof(mString)); //获取产品名称
			if (wcstombs(Buffer, mString, 256) != -1)
			{
				//m_listLog.InsertString(-1, "产品名称: "+CString(Buffer));
				strTemp += " ";
				strTemp += CString(Buffer);
			}
			else
			{
				//m_listLog.InsertString(-1, "HidD_GetProductString error");
				strTemp += " ";
				strTemp += "HidD_GetProductString error";
			}
		/*	PHIDP_PREPARSED_DATA pHidpPreparsedData;
			if (!HidD_GetPreparsedData(hDevHandle, &pHidpPreparsedData))
			{
				SetupDiDestroyDeviceInfoList(hDevInfoSet);
				CloseHandle(hDevHandle);
				m_listLog.InsertString(-1, "HidD_GetPreparsedData error");
			}
			HIDP_CAPS hidpCaps;
			if (HIDP_STATUS_SUCCESS == HidP_GetCaps(pHidpPreparsedData, &hidpCaps))
			{
				strTemp.Format("InputByteLength:  %d      OutputByteLength:  %d", 	hidpCaps.InputReportByteLength, hidpCaps.OutputReportByteLength);
				m_listLog.InsertString(-1, strTemp);
			}  */		
			CloseHandle(hDevHandle);

			
			m_comboDevice.InsertString(-1, strTemp); //添加设备到选择框
			MemberIndex++;
		}		
	}while (bSuccess);

	//m_listLog.SetTopIndex(m_listLog.GetCount()-1);
	SetupDiDestroyDeviceInfoList(hDevInfoSet);
}

//振动函数
void Cintubation_masterDlg::vibration(int strength)
{
	int nCurSel = m_comboDevice.GetCurSel();
	HANDLE hWriteHandle = CreateFile(m_pathArray.GetAt(nCurSel), GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL); 
	if (hWriteHandle == INVALID_HANDLE_VALUE)
	{
		MessageBox(_T("打开设备失败"), _T("错误"), MB_OK|MB_ICONWARNING);
		return;
	}

	DWORD nWriteBytes = 0; 	
	BYTE pWriteReport1[8] = {0x00, 0x51, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; //一次发送8个字节
	BYTE pWriteReport2[8] = {0x00, 0xfa, 0xfe, 0x00, 0x00, 0x00, 0x00, 0x00};
	pWriteReport1[5] = (int)(2.5 * strength); //震动强度
	if(strength>500)
		pWriteReport1[5]=(int)(2.5 * 500);
	if(strength<0)
		pWriteReport1[5]=(int)(0);
	if (!WriteFile(hWriteHandle, pWriteReport1, 8, &nWriteBytes, NULL))
	{
		MessageBox(_T("发送命令失败"), _T("错误"), MB_OK|MB_ICONWARNING);
		return;
	}	
	if (!WriteFile(hWriteHandle, pWriteReport2, 8, &nWriteBytes, NULL))
	{
		MessageBox(_T("发送命令失败"), _T("错误"), MB_OK|MB_ICONWARNING);
		return;
	}
	CloseHandle(hWriteHandle);
}

//停止振动函数
void Cintubation_masterDlg::stopVibra()
{
	// TODO: 
	int nCurSel = m_comboDevice.GetCurSel();
	HANDLE hWriteHandle = CreateFile(m_pathArray.GetAt(nCurSel), GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL); 
	if (hWriteHandle == INVALID_HANDLE_VALUE)
	{
		MessageBox(_T("打开设备失败"),_T("错误"), MB_OK|MB_ICONWARNING);
		return;
	}

	DWORD nWriteBytes = 0; 
	BYTE pWriteReport[8] = {0x00, 0xf3, 0x00, 0x80, 0x00, 0x80, 0x00, 0x00}; //一次发送8个字节
	if (!WriteFile(hWriteHandle, pWriteReport, 8, &nWriteBytes, NULL))
	{
		MessageBox(_T("发送命令失败"), _T("错误"), MB_OK|MB_ICONWARNING);
		return;
	}	
	CloseHandle(hWriteHandle);
}

//接收手柄数据线程
DWORD WINAPI RecvKeypress(LPVOID lpParameter)
{
	Cintubation_masterDlg *pdlg = (Cintubation_masterDlg *)lpParameter;

	DWORD nReadBytes = 0; 
	
	pdlg->joyinfoex.dwSize = sizeof(JOYINFOEX);
	pdlg->joyinfoex.dwFlags = JOY_RETURNALL;

	//轮询运动反馈
	while (pdlg->m_bOpen)	
	{	
		
		//读取手柄信息
		UINT joyNums;
		joyNums = joyGetNumDevs();
//		printf("当前手柄数量:%d \n",joyNums);
		if (joyNums>=1)
		{
			MMRESULT joyreturn = joyGetPosEx(JOYSTICKID1, &pdlg->joyinfoex);
			if(joyreturn == JOYERR_NOERROR)
			{
				/*
				BYTE *buffer = new BYTE(32);
				memcpy(buffer,&pdlg->joyinfoex.dwXpos,sizeof(DWORD));
				data_send.buf[0] = (char) (((buffer[0] & 0xFF) << 8) | (buffer[1] & 0xFF));
				data_send.buf[1] = (char) (((buffer[1] & 0xFF) << 8) | (buffer[2] & 0xFF));
				memcpy(buffer,&pdlg->joyinfoex.dwYpos,sizeof(DWORD));
				data_send.buf[2] = (char) (((buffer[0] & 0xFF) << 8) | (buffer[1] & 0xFF));
				data_send.buf[3] = (char) (((buffer[1] & 0xFF) << 8) | (buffer[2] & 0xFF));
				*/
				xPos = pdlg->joyinfoex.dwXpos;
				yPos = pdlg->joyinfoex.dwYpos;
				uPos = pdlg->joyinfoex.dwRpos;

				//如果手柄姿态不满足条件，则振动
				pdlg->vibration(feedBackValue);

				
			}else
			{
				switch(joyreturn) 
				{
				case JOYERR_PARMS :
					printf("bad parameters\n");
					break;
				case JOYERR_NOCANDO :
					printf("request not completed\n");
					break;
				case JOYERR_UNPLUGGED :
					printf("joystick is unplugged\n");
					break;
				default:
					printf("未知错误\n");
					break;
				}
			}
			Sleep(20); 
		}
	}
	CloseHandle(pdlg->hReadHandle);

	return 0;
}

//打开设备函数
void Cintubation_masterDlg::openDevice()
{
	int nCurSel = m_comboDevice.GetCurSel();
	hReadHandle = CreateFile(m_pathArray.GetAt(nCurSel), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL); 
	
		if (hReadHandle == INVALID_HANDLE_VALUE)
		{
			MessageBox(_T("打开设备失败"), _T("错误"), MB_OK|MB_ICONWARNING);
			return;
		}
		else
		{
			HANDLE hRecvThreadHandle = CreateThread(NULL, 0, RecvKeypress, this, 0, NULL); //创建接收线程
			CloseHandle(hRecvThreadHandle);	
			m_bOpen = TRUE;
			GetDlgItem(IDC_BUTTON_LISTEN)->EnableWindow(true);
		}
}

//接收图像函数
Mat recieveMat(SOCKET sockServer) {
	Mat img(IMG_HEIGHT, IMG_WIDTH, CV_8UC3, cv::Scalar(0));
	int needRecv = sizeof(recvbuf);
	int count = 0;
	for (int i = 0; i < 32; i++) {
		int pos = 0;
		int len0 = 0;
		while (pos < needRecv) {
			len0 = recv(sockServer, (char*)(&data_recv) + pos, needRecv - pos, 0);
			pos += len0;
		}
		count = count + data_recv.flag;
		int num1 = IMG_HEIGHT / 32 * i;
		for (int j = 0; j < IMG_HEIGHT / 32; j++) {
			int num2 = j * IMG_WIDTH * 3;
			uchar* ucdata = img.ptr<uchar>(j + num1);
			for (int k = 0; k < IMG_WIDTH * 3; k++) {
				ucdata[k] = data_recv.buf[num2 + k];
			}
		}
		if (data_recv.flag == 2) {
			if (count == 33) {
				return img;
			}
			else {
				count = 0;
				i = 0;
			}
		}
	}
}

//接受线程函数
DWORD WINAPI RecvSocket(LPVOID lpParameter)
{
	Cintubation_masterDlg *pdlg = (Cintubation_masterDlg *)lpParameter;
	int ret;
	DWORD length_recv,length_send;
	DWORD dwSent = 0;
	while (true) {
		
		//接受图像
		Mat frame = recieveMat(pdlg->clntSock);
		Mat frame2 = recieveMat(pdlg->clntSock);

		//接收反馈信息
		recv(pdlg->clntSock,feedbackData,8,0);

		//接受音频
		recv(pdlg->clntSock,(char*)&length_recv,sizeof(DWORD),0);
		//接下来开辟length长的内存空间
		pdlg->m_AudioDataOut[pdlg->nReceive].lpdata =(PBYTE)realloc (0,length_recv);
		DWORD dwReceived = 0,dwret;
		//循环检测接受
		
		while(length_recv>dwReceived)
		{
			dwret = recv(pdlg->clntSock,(char*)(pdlg->m_AudioDataOut[pdlg->nReceive].lpdata+dwReceived),(length_recv-dwReceived),0);
			dwReceived +=dwret;
			if(dwReceived ==length_recv)
			{
                pdlg->m_AudioDataOut[pdlg->nReceive].dwLength = length_recv;
				break;
			}
				
		}
		 pdlg->nReceive=(pdlg->nReceive+1)%OutBlocks;

		//发送手柄姿态消息
		char dataX[8];
		char dataY[8];
		char dataU[8];
		ZeroMemory(dataX,8);
		ZeroMemory(dataY,8);
		ZeroMemory(dataU,8);
		ultoa(xPos,dataX,10);
		ultoa(yPos,dataY,10);
		ultoa(uPos,dataU,10);
		send(pdlg->clntSock,dataX,8,0);
		send(pdlg->clntSock,dataY,8,0);
		send(pdlg->clntSock,dataU,8,0);

		//发送音频
		/*
		length_send =pdlg->m_AudioDataIn[pdlg->nSend].dwLength;	
		if(length_send!=0)
		{
		send(pdlg->clntSock,(char*)&length_send,sizeof(DWORD),0);
		while(1)
		{
			ret=send(pdlg->clntSock,(char*)(pdlg->m_AudioDataIn[pdlg->nSend].lpdata+dwSent),(length_send-dwSent),0);
			dwSent += ret;
			if(dwSent ==length_send)
			{   
    		free(pdlg->m_AudioDataIn[pdlg->nSend].lpdata);
			pdlg->m_AudioDataIn[pdlg->nSend].dwLength = 0;
			break;
			}
		}
		}
		pdlg->nSend = (pdlg->nSend +1)% InBlocks;
		*/

		//获取窗口相关信息，显示图像
		CDC *pDC = pdlg->GetDlgItem(IDC_STATIC_CAMERA1)->GetDC();//根据ID获得窗口指针再获取与该窗口关联的上下文指针
		CDC *pDC2 = pdlg->GetDlgItem(IDC_STATIC_CAMERA2)->GetDC();
		
		if (frame.data) //显示图像
		{
			showMatImgToWnd(pDC,frame,(LPVOID)pdlg);
		}
		if (frame2.data) //显示图像
		{
			showMatImgToWnd(pDC2,frame2,(LPVOID)pdlg);
		}
		
		//处理传感数据类型
		feedBackValue = atoi(feedbackData);
		text+=feedbackData;
		pdlg->SetDlgItemTextW(IDC_EDIT1,(LPCTSTR)text);
		;
		//处理音频
		
		if (waitKey(1) >= 0)break;
		Sleep(100);
		
		
	}
	::closesocket(pdlg->servSock);
	::WSACleanup();

	return 0;
}

//录音播放初始化
void Cintubation_masterDlg::openRec()
{
	//allocate buffer memory
	pBuffer1=(PBYTE)malloc(INP_BUFFER_SIZE);
	pBuffer2=(PBYTE)malloc(INP_BUFFER_SIZE);
	if (!pBuffer1 || !pBuffer2) 
	{
		if (pBuffer1) free(pBuffer1);
		if (pBuffer2) free(pBuffer2);
		MessageBeep(MB_ICONEXCLAMATION);
		AfxMessageBox(_T("Memory erro!"));
		return ;
	}

	//open waveform audio for input
	
	m_waveformin.wFormatTag=WAVE_FORMAT_PCM;
	m_waveformin.nChannels=1;
	m_waveformin.nSamplesPerSec=11025;//采样频率
	m_waveformin.nAvgBytesPerSec=11025;
	m_waveformin.nBlockAlign=1;
	m_waveformin.wBitsPerSample=8;
	m_waveformin.cbSize=0;

	
	if (waveInOpen(&hWaveIn,WAVE_MAPPER,&m_waveformin,(DWORD)this->m_hWnd,NULL,CALLBACK_WINDOW))
	{   //打开录音设备函数 
		free(pBuffer1);
		free(pBuffer2);
		MessageBeep(MB_ICONEXCLAMATION);
		AfxMessageBox(_T("Audio can not be open!"));
	}
	pWaveHdr1->lpData=(LPSTR)pBuffer1;
	pWaveHdr1->dwBufferLength=INP_BUFFER_SIZE;
	pWaveHdr1->dwBytesRecorded=0;
	pWaveHdr1->dwUser=0;
	pWaveHdr1->dwFlags=0;
	pWaveHdr1->dwLoops=1;
	pWaveHdr1->lpNext=NULL;
	pWaveHdr1->reserved=0;
	
	waveInPrepareHeader(hWaveIn,pWaveHdr1,sizeof(WAVEHDR));
	
	pWaveHdr2->lpData=(LPSTR)pBuffer2;
	pWaveHdr2->dwBufferLength=INP_BUFFER_SIZE;
	pWaveHdr2->dwBytesRecorded=0;
	pWaveHdr2->dwUser=0;
	pWaveHdr2->dwFlags=0;
	pWaveHdr2->dwLoops=1;
	pWaveHdr2->lpNext=NULL;
	pWaveHdr2->reserved=0;
	
	waveInPrepareHeader(hWaveIn,pWaveHdr2,sizeof(WAVEHDR));
			
	// Add the buffers
	
	waveInAddBuffer (hWaveIn, pWaveHdr1, sizeof (WAVEHDR)) ;
	waveInAddBuffer (hWaveIn, pWaveHdr2, sizeof (WAVEHDR)) ;
		
	// Begin sampling
	waveInStart (hWaveIn) ;
}


void Cintubation_masterDlg::OnBnClickedButtonListen()
{
	// TODO: 在此添加控件通知处理程序代码
	::WSAStartup(MAKEWORD(2, 0), &wsaData);
	//创建套接字
	servSock = ::socket(AF_INET, SOCK_STREAM, 0);

	//绑定套接字
	sockaddr_in sockAddr;
	sockAddr.sin_family = AF_INET;
	sockAddr.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
	sockAddr.sin_port = htons(1234);
	::bind(servSock, (SOCKADDR*)&sockAddr, sizeof(SOCKADDR));
	//进入监听状态
	listen(servSock, 5);
//接收客户端请求
	SOCKADDR clntAddr;
	int nSize = sizeof(SOCKADDR);
	clntSock = accept(servSock, (SOCKADDR*)&clntAddr, &nSize);

	//录音初始化
	//openRec();
	//创建读取线程
	HANDLE hRecvSocketThreadHandle = CreateThread(NULL, 0, RecvSocket, this, 0, NULL); //创建接收线程
	openPlay();

}


void Cintubation_masterDlg::OnBnClickedButtonOpendevice()
{
	// TODO: 在此添加控件通知处理程序代码
	openDevice();
}


void Cintubation_masterDlg::openPlay()
{
	// TODO: Add your control notification handler code here
	m_waveformout.wFormatTag		=	WAVE_FORMAT_PCM;
	m_waveformout.nChannels		    =1;
	m_waveformout.nSamplesPerSec	=11025;
	m_waveformout.nAvgBytesPerSec   =11025;
	m_waveformout.nBlockAlign	    =1;
	m_waveformout.wBitsPerSample	=8;
	m_waveformout.cbSize			=0;
	
	
	if (waveOutOpen(&hWaveOut,WAVE_MAPPER,&m_waveformout,(DWORD)this->m_hWnd,NULL,CALLBACK_WINDOW)) {
		MessageBeep(MB_ICONEXCLAMATION);
		AfxMessageBox(_T("Audio output erro"));
	}
	
	return ;
}

LRESULT Cintubation_masterDlg::OnMM_WIM_DATA(UINT wParam,LONG lParam)
{
   
    int nextBlock = (nAudioIn+1)% InBlocks;	
	if(m_AudioDataIn[nextBlock].dwLength!=0)//下一“块”没发走
	{  //把PWAVEHDR(即pBUfferi)里的数据拷贝到当前“块”中
        
	 	m_AudioDataIn[nAudioIn].lpdata  
		= (PBYTE)realloc (m_AudioDataIn[nAudioIn].lpdata , (((PWAVEHDR) lParam)->dwBytesRecorded+m_AudioDataIn[nAudioIn].dwLength)) ;
		if (m_AudioDataIn[nAudioIn].lpdata == NULL)
		{
			waveInClose (hWaveIn) ;
			MessageBeep (MB_ICONEXCLAMATION) ;
			AfxMessageBox(_T("erro memory OnMM_WIM_DATA"));
			return 0;
		}
	    CopyMemory ((m_AudioDataIn[nAudioIn].lpdata+m_AudioDataIn[nAudioIn].dwLength), 
				   ((PWAVEHDR) lParam)->lpData,
				   ((PWAVEHDR) lParam)->dwBytesRecorded) ;//(*destination,*resource,nLen);
		
		m_AudioDataIn[nAudioIn].dwLength +=((PWAVEHDR) lParam)->dwBytesRecorded;
        
	}
	else //把PWAVEHDR(即pBUfferi)里的数据拷贝到下一“块”中
	{
		nAudioIn = (nAudioIn+1)% InBlocks;
		m_AudioDataIn[nAudioIn].lpdata = (PBYTE)realloc
			(0,((PWAVEHDR) lParam)->dwBytesRecorded);
		CopyMemory(m_AudioDataIn[nAudioIn].lpdata,
			    ((PWAVEHDR) lParam)->lpData,
				((PWAVEHDR) lParam)->dwBytesRecorded) ;
	   m_AudioDataIn[nAudioIn].dwLength =((PWAVEHDR) lParam)->dwBytesRecorded;

	}
  	// Send out a new buffer	
	waveInAddBuffer (hWaveIn, (PWAVEHDR) lParam, sizeof (WAVEHDR)) ;
	return 0;

	
}

LRESULT Cintubation_masterDlg::OnMM_WIM_CLOSE(UINT wParam,LONG lParam)
{

	waveInUnprepareHeader (hWaveIn, pWaveHdr1, sizeof (WAVEHDR)) ;
	waveInUnprepareHeader (hWaveIn, pWaveHdr2, sizeof (WAVEHDR)) ;
	
	free (pBuffer1) ;
	free (pBuffer2) ;

	return 0;
}



LRESULT Cintubation_masterDlg::OnMM_WOM_DONE(UINT wParam,LONG lParam)
{  
	free(m_AudioDataOut[nAudioOut].lpdata);
	m_AudioDataOut[nAudioOut].lpdata = reinterpret_cast<PBYTE>(malloc(1));
	m_AudioDataOut[nAudioOut].dwLength = 0;
 
    nAudioOut= (nAudioOut+1)%OutBlocks;
	((PWAVEHDR)lParam)->lpData          = (LPSTR)m_AudioDataOut[nAudioOut].lpdata ;
	((PWAVEHDR)lParam)->dwBufferLength  = m_AudioDataOut[nAudioOut].dwLength ;
    TRACE("the next length %d\n",((PWAVEHDR)lParam)->dwBufferLength);
	waveOutPrepareHeader (hWaveOut,(PWAVEHDR)lParam,sizeof(WAVEHDR));
    waveOutWrite(hWaveOut,(PWAVEHDR)lParam,sizeof(WAVEHDR));//cut
   return 0;

}
LRESULT Cintubation_masterDlg::OnMM_WOM_CLOSE(UINT wParam,LONG lParam)
{
	waveOutUnprepareHeader (hWaveOut, pWaveHdrOut, sizeof (WAVEHDR)) ;
		
	//release all the memory of the AudioData
	for(int i=0;i<InBlocks;i++)
	{
		if(m_AudioDataIn[i].dwLength != 0)
			free(m_AudioDataIn[i].lpdata);
		
	}
	for(int i=0;i<OutBlocks;i++)
	{
		if(m_AudioDataOut[i].dwLength != 0)
			free(m_AudioDataOut[i].lpdata);
	}

	return 0;
}


LRESULT Cintubation_masterDlg::OnMM_WOM_OPEN(UINT wParam,LONG lParam)
{   
	// Set up header    
	pWaveHdrOut->lpData          = (LPSTR)m_AudioDataOut[nAudioOut].lpdata ;
	pWaveHdrOut->dwBufferLength  = m_AudioDataOut[nAudioOut].dwLength ;
    pWaveHdrOut->dwBytesRecorded = 0 ;
	pWaveHdrOut->dwUser          = 0 ;
	pWaveHdrOut->dwFlags         = WHDR_BEGINLOOP ;
	pWaveHdrOut->dwLoops         = 1 ;
	pWaveHdrOut->lpNext          = NULL ;
	pWaveHdrOut->reserved        = 0 ;

    // Prepare and write
	waveOutPrepareHeader (hWaveOut, pWaveHdrOut, sizeof (WAVEHDR)) ;
	waveOutWrite (hWaveOut, pWaveHdrOut, sizeof (WAVEHDR)) ;

  return 0;
	
}

