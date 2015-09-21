//-----------------Header file for the AmbientLightSensorDev Class-----------------------------------------
#include <Windows.h>
#include <SetupAPI.h>
#include <hidsdi.h>
#include <Dbt.h>
#include <usb.h>
#include <stdlib.h>

#pragma comment (lib, "SetupAPI")
#pragma comment (lib, "Hid")

class AmbientLightSensorDev
{
public:
//Constructors
	AmbientLightSensorDev();

//Methods
	BOOL	EnumerateDevs();
	LPCTSTR GetMyDevicePath();
	void	RemoveDevice();
	void	SetOutputBuffer(int index, BYTE value);
	BYTE	GetInputBuffer(int index);
	//BOOL	GetDeviceCaps();
	//BOOL	SendCmd();
	//BOOL	SetADC_PowerCmd();
	//BOOL	SetTimingCmd();
	//BOOL	ReadInput();
	//BOOL	SetupReport();
	//BOOL	SetInterruptControl();
	//BOOL	SetInterruptSource();
	//BOOL	SetReportRequest();
	//BOOL	SetADC_PowerOff();

//Members
	CONST unsigned int	SupportedVID	= 0x04D8;
	CONST unsigned int	SupportedPID	= 0xFB02;
	GUID				HidGuid;				//GUID for the HID class.
	

private:
//Methods

//Members
	LPCTSTR								MyDevicePath = NULL;		//String to the device path.
	HANDLE								DeviceHandle;				//Handle to the device.
	//HANDLE							ReadHandle;
	PSP_DEVICE_INTERFACE_DETAIL_DATA	DetailData;
	USHORT								InReportLength;
	USHORT								OutReportLength;
	BYTE								InputBuffer[65];
	BYTE								OutputBuffer[65];
	BYTE								CmdSequence;
};

