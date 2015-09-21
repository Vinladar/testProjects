//-----------------Light Sensor Project-----------------------
#include <Windows.h>
#include "AmbientLightSensorDev.h"

HINSTANCE							ghMainInstance;						//Main application instance.
HWND								ghMainWnd;							//Main window instance.
TCHAR								AppName[12] = TEXT("LightSensor");	//Application name.
DEV_BROADCAST_DEVICEINTERFACE		DevBroadcastDeviceInt;				//Device broadcast interface structure.
HDEVNOTIFY							DevNotificationHandle;				//Device notification handle.
AmbientLightSensorDev*				Dev;								//Pointer to the ambient light sensor device.

#define ID_INPUTWND 65
#define ID_WRITEBUTTON 66
#define ID_READBUTTON 67

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);					//Prototype to the window procedure.

//Application entry point.
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstance, PSTR CmdLine, int ShowCmd)
{
	WNDCLASS	wc;
	MSG			msg;
	ghMainInstance = hInstance;

	//Window class structure definition.
	wc.style			= CS_HREDRAW | CS_VREDRAW;
	wc.cbClsExtra		= 0;
	wc.cbWndExtra		= 0;
	wc.hbrBackground	= (HBRUSH)GetStockObject(LTGRAY_BRUSH);
	wc.hCursor			= LoadCursor(NULL, IDC_ARROW);
	wc.hIcon			= LoadIcon(NULL, IDI_APPLICATION);
	wc.lpfnWndProc		= WndProc;
	wc.lpszClassName	= AppName;
	wc.lpszMenuName		= NULL;
	wc.hInstance		= ghMainInstance;

	//Window class registration.
	if (!RegisterClass(&wc))
	{
		MessageBox(NULL, TEXT("WNDCLASS Registration Failed!"), TEXT("Failure!"), MB_OK);
		return 0;
	}

	//Window creation.
	ghMainWnd = CreateWindow(AppName, TEXT("Main Window"), WS_OVERLAPPEDWINDOW,
								CW_USEDEFAULT, CW_USEDEFAULT,
								1060, 500,
								NULL, NULL, hInstance, NULL);
	//Show and update the window
	ShowWindow(ghMainWnd, ShowCmd);
	UpdateWindow(ghMainWnd);

	//Message loop.
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return msg.wParam;
}

//Window procedure definition.
LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	LPCTSTR							DeviceName;				//String storing the device name.	
	PDEV_BROADCAST_HDR				lpdb;					//Device broadcast structure
	PDEV_BROADCAST_DEVICEINTERFACE	lpdbi;					//Device interface broadcast structure.
	const int						NumBoxesPerRow = 16;
	const int						Rows = 5;
	const int						NumBoxes = 65;
	HWND							TextWnd[NumBoxes];
	HWND							InputBox;
	HWND							Button[2];
	int								BoxWidth = 40;
	int								BoxHeight = 20;
	int								ButtonWidth = 100;
	int								ButtonHeight = 30;
	TCHAR							wndName[5];
	TCHAR							editName[5] = TEXT("edit");

	switch (msg)
	{

	//When the window is created, the constructor for Dev will run. After the contructor, the interface broadcast 
	//	structure is filled with the required details, then the application registers for broadcast events.	
	case WM_CREATE:
		Dev = new AmbientLightSensorDev();
		DevBroadcastDeviceInt.dbcc_size			= sizeof(DevBroadcastDeviceInt);
		DevBroadcastDeviceInt.dbcc_devicetype	= DBT_DEVTYP_DEVICEINTERFACE;
		DevBroadcastDeviceInt.dbcc_classguid	= Dev->HidGuid;

		DevNotificationHandle					= RegisterDeviceNotification(hWnd, 
													&DevBroadcastDeviceInt, 
													DEVICE_NOTIFY_WINDOW_HANDLE);
		TCHAR		str[10];
		HDC			hdc;
		PAINTSTRUCT ps;
		BYTE		EditBoxValue[65];
		HWND*		ActiveWnd;
		TCHAR		TextWndContents[100];

		for (int n = 0; n < NumBoxes; n++)
		{
			wsprintf(str, TEXT("%i"), n);
			TextWnd[n] = CreateWindow(TEXT("edit"),
				TEXT("00"),
				WS_CHILD | WS_VISIBLE | WS_BORDER,
				((BoxWidth + 20) * (n % NumBoxesPerRow)) + 50,
				((BoxHeight + 20) * (n / NumBoxesPerRow)) + 50,
				BoxWidth,
				BoxHeight,
				hWnd,
				(HMENU)n,
				NULL,
				NULL);
		}
		InputBox = CreateWindow(TEXT("edit"),
			NULL,
			WS_CHILD | WS_VISIBLE | WS_BORDER,
			50, 300,
			940, 100,
			hWnd,
			(HMENU)ID_INPUTWND,
			NULL,
			NULL);
		Button[0] = CreateWindow(TEXT("button"),
			TEXT("Write"),
			WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
			50, 420,
			ButtonWidth, ButtonHeight,
			hWnd,
			(HMENU)ID_WRITEBUTTON,
			NULL, NULL);
		Button[1] = CreateWindow(TEXT("button"),
			TEXT("Read"),
			WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
			180, 420,
			ButtonWidth, ButtonHeight,
			hWnd,
			(HMENU)ID_READBUTTON,
			NULL, NULL);
		return 0;
//--------------------------------------End WM_CREATE-----------------------------------

	case WM_COMMAND:
		switch (HIWORD(wParam))
		{
			GetWindowText(TextWnd[0], TextWndContents, 100);
		}
		return 0;

	case WM_PAINT:
		TCHAR BoxLabel[20];
		hdc = BeginPaint(hWnd, &ps);
		SetBkMode(hdc, TRANSPARENT);
		TextOut(hdc, 50, 285, TEXT("Input:"), 7);
		if (!(Dev->GetMyDevicePath() == NULL))
			TextOut(hdc, 10, 10, TEXT("Device Connected"), 16);
		else TextOut(hdc, 10, 10, TEXT("Cannot Detect Device!!!"), 22);
		for (int i = 0; i < 65; i++)
		{
			wsprintf((LPWSTR)BoxLabel, TEXT("%X"), i);
			TextOut(hdc,
				((BoxWidth + 20) * (i % NumBoxesPerRow)) + 50,
				((BoxHeight + 20) * (i / NumBoxesPerRow)) + 35,
				(LPWSTR)BoxLabel,
				2);
		}
		EndPaint(hWnd, &ps);
		return 0;

	case WM_DEVICECHANGE:

		switch (wParam)
		{
		//When a WM_DEVICECHANGE message is received, check to see type of message.
		//If it is a DBT_DEVTYP_DEVICEINTERFACE message, then save the device name into DeviceName.
		case DBT_DEVICEARRIVAL:
			lpdb = (PDEV_BROADCAST_HDR)lParam;
			if (lpdb->dbch_devicetype == DBT_DEVTYP_DEVICEINTERFACE)
			{
				lpdbi = (PDEV_BROADCAST_DEVICEINTERFACE)lParam;
				DeviceName = lpdbi->dbcc_name;
			}

			//If the device path in Dev is NULL, then enumerate the devices.
			if (Dev->GetMyDevicePath() == NULL)
			{
				Dev->EnumerateDevs();
			}
			InvalidateRect(hWnd, NULL, TRUE);
			break;
		
		//Check to see if the message is a DBT_DEVTYP_DEVICEINTERFACE message.
		//If it is, compare the DeviceName to the device path in Dev. If they match, call Dev->RemoveDevice().
		case DBT_DEVICEREMOVECOMPLETE:
			lpdb = (PDEV_BROADCAST_HDR)lParam;
			if (lpdb->dbch_devicetype == DBT_DEVTYP_DEVICEINTERFACE)
			{
				lpdbi = (PDEV_BROADCAST_DEVICEINTERFACE)lParam;
				DeviceName = lpdbi->dbcc_name;
				if (lstrcmpi(DeviceName, Dev->GetMyDevicePath()) == 0)
					Dev->RemoveDevice();
			}
			InvalidateRect(hWnd, NULL, TRUE);
			break;
		}
		return 0;

	case WM_DESTROY:
		Dev->RemoveDevice();
		delete Dev;
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProc(hWnd, msg, wParam, lParam);
}
