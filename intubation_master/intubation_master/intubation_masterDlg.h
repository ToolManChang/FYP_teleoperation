
// intubation_masterDlg.h : ͷ�ļ�
//

#pragma once

#include "opencv2/opencv.hpp"
#include <WinSock2.h>
#include <Windows.h>
#include "afxwin.h"
#include <stdio.h> 
#include "stdafx.h"



#pragma comment (lib, "ws2_32.lib")  //���� ws2_32.dll
#pragma comment (lib, "Winmm.lib")
#pragma comment (lib, "hid.lib")  //���� hid lib
#pragma comment (lib, "setupapi.lib")
#pragma warning(disable : 4996)

//������ͼ��Ĭ�ϴ�СΪ 640*480�����޸�
#define IMG_WIDTH 640	// �贫��ͼ��Ŀ�
#define IMG_HEIGHT 480	// �贫��ͼ��ĸ�
#define InBlocks 4 //�洢������Ƶ���ݵĵ�Ԫ��
#define OutBlocks 4  //�洢�����Ƶ���ݵĵ�Ԫ��
#define  INP_BUFFER_SIZE 16384
#define  PORT 8000
//Ĭ�ϸ�ʽΪCV_8UC3
#define BUFFER_SIZE IMG_WIDTH*IMG_HEIGHT*3/32 

struct CAudioData
{
	PBYTE lpdata;
	DWORD dwLength;
};
// Cintubation_masterDlg �Ի���
class Cintubation_masterDlg : public CDialogEx
{
// ����
public:
	Cintubation_masterDlg(CWnd* pParent = NULL);	// ��׼���캯��

// �Ի�������
	enum { IDD = IDD_INTUBATION_MASTER_DIALOG };

	//�����豸��ȡ
	HANDLE hReadHandle; //�豸��ȡ���
	CStringArray m_pathArray; //�洢�豸·��
	BOOL m_bOpen; //�豸��״̬
	JOYINFO joyinfo;//����joystick��Ϣ�ṹ��
	JOYINFOEX joyinfoex;

	//�������紫��
	SOCKET servSock;
	SOCKET clntSock;
	WSADATA wsaData;

	//������Ƶ����
	HWAVEIN hWaveIn;    //��������
	HWAVEOUT hWaveOut;  //����豸

	PWAVEHDR pWaveHdr1,pWaveHdr2;    //���"ͷ�ṹ��"          
	PWAVEHDR pWaveHdrOut;		
	WAVEFORMATEX m_waveformin,m_waveformout;    

	PBYTE pBuffer1,pBuffer2;//�����豸���û�����
	//�����ݴ�¼���Ҫ���͵ļ����յ��ļ���Ҫ���ŵ������ļ���ѭ�����У�
	CAudioData m_AudioDataIn[InBlocks],m_AudioDataOut[OutBlocks];
   	int   nAudioIn, nSend, //¼�롢����ָ��
	      nAudioOut, nReceive;//���ա�����ָ��
        //����¼���ͷ��������ں������ͬ�����⣬��Ҫ����Щָ�����Э��

	void SearchForDevices();
	void openDevice();
	void vibration(int);
	void stopVibra();
	void openRec();
	void openPlay();
	

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV ֧��

	


// ʵ��
protected:
	HICON m_hIcon;



	// ���ɵ���Ϣӳ�亯��
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
