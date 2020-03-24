
// intubation_masterDlg.h : 头文件
//

#pragma once

#include "opencv2/opencv.hpp"
#include <WinSock2.h>
#include <Windows.h>
#include "afxwin.h"
#include <stdio.h> 
#include "stdafx.h"



#pragma comment (lib, "ws2_32.lib")  //加载 ws2_32.dll
#pragma comment (lib, "Winmm.lib")
#pragma comment (lib, "hid.lib")  //加载 hid lib
#pragma comment (lib, "setupapi.lib")
#pragma warning(disable : 4996)

//待传输图像默认大小为 640*480，可修改
#define IMG_WIDTH 640	// 需传输图像的宽
#define IMG_HEIGHT 480	// 需传输图像的高
#define InBlocks 4 //存储输入音频数据的单元数
#define OutBlocks 4  //存储输出音频数据的单元数
#define  INP_BUFFER_SIZE 16384
#define  PORT 8000
//默认格式为CV_8UC3
#define BUFFER_SIZE IMG_WIDTH*IMG_HEIGHT*3/32 

struct CAudioData
{
	PBYTE lpdata;
	DWORD dwLength;
};
// Cintubation_masterDlg 对话框
class Cintubation_masterDlg : public CDialogEx
{
// 构造
public:
	Cintubation_masterDlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
	enum { IDD = IDD_INTUBATION_MASTER_DIALOG };

	//用于设备读取
	HANDLE hReadHandle; //设备读取句柄
	CStringArray m_pathArray; //存储设备路径
	BOOL m_bOpen; //设备打开状态
	JOYINFO joyinfo;//定义joystick信息结构体
	JOYINFOEX joyinfoex;

	//用于网络传输
	SOCKET servSock;
	SOCKET clntSock;
	WSADATA wsaData;

	//用于音频传输
	HWAVEIN hWaveIn;    //声音输入
	HWAVEOUT hWaveOut;  //输出设备

	PWAVEHDR pWaveHdr1,pWaveHdr2;    //相关"头结构体"          
	PWAVEHDR pWaveHdrOut;		
	WAVEFORMATEX m_waveformin,m_waveformout;    

	PBYTE pBuffer1,pBuffer2;//输入设备所用缓冲区
	//用于暂存录入后要发送的及接收到的即将要播放的声音文件的循环队列，
	CAudioData m_AudioDataIn[InBlocks],m_AudioDataOut[OutBlocks];
   	int   nAudioIn, nSend, //录入、发送指针
	      nAudioOut, nReceive;//接收、播放指针
        //对于录音和放音都存在和网络的同步问题，主要靠这些指针进行协调

	void SearchForDevices();
	void openDevice();
	void vibration(int);
	void stopVibra();
	void openRec();
	void openPlay();
	

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持

	


// 实现
protected:
	HICON m_hIcon;



	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg LRESULT OnMM_WOM_OPEN(UINT wParam,LONG lParam);
	afx_msg LRESULT OnMM_WOM_DONE(UINT wParam,LONG lParam);
	afx_msg LRESULT OnMM_WOM_CLOSE(UINT wParam,LONG lParam);
	afx_msg LRESULT OnMM_WIM_DATA(UINT wParam,LONG lParam);
	afx_msg LRESULT OnMM_WIM_CLOSE(UINT wParam,LONG lParam);
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedButtonListen();
	CComboBox m_comboDevice;
	afx_msg void OnBnClickedButtonOpendevice();
};
