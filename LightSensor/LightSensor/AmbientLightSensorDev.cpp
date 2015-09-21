#include "AmbientLightSensorDev.h"

//Constructor for the AmbientLightSensorDev class. 
AmbientLightSensorDev::AmbientLightSensorDev()
{
	//Retrieve the GUID for the HID class.
	HidD_GetHidGuid(&HidGuid);

	EnumerateDevs();
}

//Enumerates the devices that are attached to the system, pulls the relevent data, and checks the device path.
//If the device path matches the supported device path, the function will return TRUE.
BOOL AmbientLightSensorDev::EnumerateDevs()
{
	HANDLE								DeviceInfoSet;				//Handle to the device info set.
	DWORD								MemberIndex = 0;			//The index of the device set to be tested.
	SP_DEVICE_INTERFACE_DATA			MyDeviceInterfaceData;		//Contains the device interface data.
	BOOL								Result;						//Result of the Enumeration and detail functions.
	ULONG								Length;						//Length of the DataDetail structure.
	_HIDD_ATTRIBUTES					Attributes;					//HID Device attributes.

	for (int i = 0; i < 65; i++)
	{
		InputBuffer[i]	= 0x00;
		OutputBuffer[i] = 0x00;
	}

	//This function call checks for all device interfaces that are present and belong to the HidGuid class.
	//HidGuid is the GUID for Human Interface Devices.
	DeviceInfoSet = SetupDiGetClassDevs(&HidGuid,
										NULL,
										NULL,
										DIGCF_PRESENT | DIGCF_INTERFACEDEVICE);
	
	MyDeviceInterfaceData.cbSize = sizeof(MyDeviceInterfaceData);
	
	//This section of code checks all of the devices that are stored in the device set that was returned
	//	in the previous SetupDiGetClassDevs function call.
	while (Result = SetupDiEnumDeviceInterfaces(DeviceInfoSet,
		0,
		&HidGuid,
		MemberIndex,
		&MyDeviceInterfaceData))
	{
		//SetupDiGetDeviceInterfaceDetail must be called once to obtain the required size, which is stored
		//	in Length. The function will return FALSE because it doesn't contain a structure to store the
		//	detail data, and it doesn't specify a length.
		Result = SetupDiGetDeviceInterfaceDetail(DeviceInfoSet,
			&MyDeviceInterfaceData,
			NULL,
			0,
			&Length,
			NULL);

		//Allocates memory for DataDetail in the size that was returned in the previous function call,
		//	then specifies the size of the DetailData structure.
		DetailData = (PSP_DEVICE_INTERFACE_DETAIL_DATA)malloc(Length);
		DetailData->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);

		//The second time SetipDiGetDeviceInterfaceDetail is called, all of the required information is 
		//	included, so DetailData will contain the correct information and the function will return
		// TRUE to the Result member.
		Result = SetupDiGetDeviceInterfaceDetail(DeviceInfoSet,
			&MyDeviceInterfaceData,
			DetailData,
			Length,
			&Length,
			NULL);

		//Save the device path into the MyDevicePath variable, and use that variable to create the DeviceHandle.
		
		MyDevicePath = DetailData->DevicePath;
		DeviceHandle = CreateFile(MyDevicePath,
			GENERIC_READ | GENERIC_WRITE,
			FILE_SHARE_READ | FILE_SHARE_WRITE,
			(LPSECURITY_ATTRIBUTES)NULL,
			OPEN_EXISTING,
			0,
			NULL);

		//Set the size of the Attributes structure and call the function to fill the structure.
		Attributes.Size = sizeof(Attributes);
		Result = HidD_GetAttributes(DeviceHandle, &Attributes);

		//Check if the VendorID and ProductID match the supported Vendor and Product IDs.
		if (Attributes.VendorID == SupportedVID)
		{
			if (Attributes.ProductID == SupportedPID)
			{
				//Vendor and product ID match
				return TRUE;
			}
			else
			{
				//Product ID doesn't match.
				RemoveDevice();
				MemberIndex++;
			}
		}
		else
		{
			//Vendor ID does not match.
			RemoveDevice();
			MemberIndex++;
		}
	}
	return FALSE;
}

//Accessor method for retrieving the device path.
LPCTSTR AmbientLightSensorDev::GetMyDevicePath()
{
	return MyDevicePath;
}

//Closes the device handle and cleans the device path.
void AmbientLightSensorDev::RemoveDevice()
{
	CloseHandle(DeviceHandle);
	DeviceHandle = NULL;
	MyDevicePath = NULL;
}

void AmbientLightSensorDev::SetOutputBuffer(int index, BYTE value)
{

}

/*
BOOL AmbientLightSensorDev::GetDeviceCaps()
{
	BOOL					Result;
	HIDP_CAPS				DevCaps;
	PHIDP_PREPARSED_DATA	PreparsedData;

	CmdSequence = 0x00;

	Result = HidD_GetPreparsedData(DeviceHandle, &PreparsedData);
	Result = HidP_GetCaps(PreparsedData, &DevCaps);


	InReportLength = DevCaps.InputReportByteLength;
	OutReportLength = DevCaps.OutputReportByteLength;

	Result = SetupReport();

	return TRUE;
}

BOOL AmbientLightSensorDev::SendCmd()
{
	DWORD	BytesWritten = 0;
	BOOL	Result;

	//OutputBuffer[2] = CmdSequence;
	CmdSequence++;
	
	if (CmdSequence > 255)
		CmdSequence = 0;

	Result = WriteFile(DeviceHandle, OutputBuffer, OutReportLength, &BytesWritten, NULL);

	return TRUE;
}

BOOL AmbientLightSensorDev::ReadInput()
{
	DWORD		BytesRead;
	DWORD		Result;
	HANDLE		hEventObject;
	OVERLAPPED	HIDOverlapped;

	hEventObject = CreateEvent((LPSECURITY_ATTRIBUTES)NULL,
								FALSE,
								TRUE,
								TEXT(""));

	HIDOverlapped.hEvent		= hEventObject;
	HIDOverlapped.Offset		= 0;
	HIDOverlapped.OffsetHigh	= 0;

	//SetInputReport();

	ReadHandle = CreateFile
		(DetailData->DevicePath,
		GENERIC_READ | GENERIC_WRITE,
		FILE_SHARE_READ | FILE_SHARE_WRITE,
		(LPSECURITY_ATTRIBUTES)NULL,
		OPEN_EXISTING,
		FILE_FLAG_OVERLAPPED,
		NULL);

	Result = ReadFile(ReadHandle,
						InputBuffer,
						InReportLength,
						&BytesRead,
						(LPOVERLAPPED)&HIDOverlapped);

	Result = WaitForSingleObject(hEventObject, 3000);

	switch (Result)
	{
	case WAIT_OBJECT_0:
		//Success
		break;

	case WAIT_TIMEOUT:

		CancelIo(ReadHandle);
		SendCmd();
		Sleep(2000);
		ReadInput();
		break;

	default:
		CancelIo(ReadHandle);
		break;
	}

	return TRUE;
}


BOOL AmbientLightSensorDev::SetTimingCmd()
{
	BOOL Result;

	OutputBuffer[0]		= 0x00;
	OutputBuffer[1]		= 0x01;
	OutputBuffer[2]		= CmdSequence;
	OutputBuffer[3]		= 0x11;
	OutputBuffer[4]		= 0x10;
	OutputBuffer[5]		= 0x39;
	OutputBuffer[6]		= 0x00;
	OutputBuffer[7]		= 0x02;
	OutputBuffer[8]		= 0xA1;
	OutputBuffer[9]		= 0x00;
	OutputBuffer[10]	= 0x0A;

	Result = SendCmd();

	OutputBuffer[0]		= 0x00;
	OutputBuffer[1]		= 0x01;
	OutputBuffer[2]		= CmdSequence;
	OutputBuffer[3]		= 0x11;
	OutputBuffer[4]		= 0x10;
	OutputBuffer[5]		= 0x39;
	OutputBuffer[6]		= 0x01;
	OutputBuffer[7]		= 0x01;
	OutputBuffer[8]		= 0x81;
	OutputBuffer[9]		= 0x00;
	OutputBuffer[10]	= 0x0A;

	Result = SendCmd();

	return TRUE;
}

BOOL AmbientLightSensorDev::SetupReport()
{
	BOOL Result;

	Result = SetTimingCmd();
	Result = SetInterruptControl();
	Result = SetInterruptSource();
	Result = SetADC_PowerCmd();

	Result = SetReportRequest();

	return TRUE;
}

BOOL AmbientLightSensorDev::SetInterruptControl()
{
	BOOL Result;

	OutputBuffer[0]		= 0x00;
	OutputBuffer[1]		= 0x01;
	OutputBuffer[2]		= CmdSequence;
	OutputBuffer[3]		= 0x11;
	OutputBuffer[4]		= 0x10;
	OutputBuffer[5]		= 0x39;
	OutputBuffer[6]		= 0x00;
	OutputBuffer[7]		= 0x02;
	OutputBuffer[8]		= 0xA2;
	OutputBuffer[9]		= 0x20;
	OutputBuffer[10]	= 0x0A;

	Result = SendCmd();

	OutputBuffer[0]		= 0x00;
	OutputBuffer[1]		= 0x01;
	OutputBuffer[2]		= CmdSequence;
	OutputBuffer[3]		= 0x11;
	OutputBuffer[4]		= 0x10;
	OutputBuffer[5]		= 0x39;
	OutputBuffer[6]		= 0x01;
	OutputBuffer[7]		= 0x01;
	OutputBuffer[8]		= 0x82;
	OutputBuffer[9]		= 0x20;
	OutputBuffer[10]	= 0x0A;

	Result = SendCmd();

	return TRUE;
}

BOOL AmbientLightSensorDev::SetInterruptSource()
{
	BOOL Result;

	OutputBuffer[0]		= 0x00;
	OutputBuffer[1]		= 0x01;
	OutputBuffer[2]		= CmdSequence;
	OutputBuffer[3]		= 0x11;
	OutputBuffer[4]		= 0x10;
	OutputBuffer[5]		= 0x39;
	OutputBuffer[6]		= 0x00;
	OutputBuffer[7]		= 0x02;
	OutputBuffer[8]		= 0xA3;
	OutputBuffer[9]		= 0x03;
	OutputBuffer[10]	= 0x0A;

	SendCmd();

	OutputBuffer[0]		= 0x00;
	OutputBuffer[1]		= 0x01;
	OutputBuffer[2]		= CmdSequence;
	OutputBuffer[3]		= 0x11;
	OutputBuffer[4]		= 0x10;
	OutputBuffer[5]		= 0x39;
	OutputBuffer[6]		= 0x01;
	OutputBuffer[7]		= 0x01;
	OutputBuffer[8]		= 0x83;
	OutputBuffer[9]		= 0x03;
	OutputBuffer[10]	= 0x0A;

	SendCmd();

	return TRUE;
}

BOOL AmbientLightSensorDev::SetADC_PowerCmd()
{
	BOOL Result;

	OutputBuffer[0]		= 0x00;
	OutputBuffer[1]		= 0x01;
	OutputBuffer[2]		= CmdSequence;
	OutputBuffer[3]		= 0x11;
	OutputBuffer[4]		= 0x10;
	OutputBuffer[5]		= 0x39;
	OutputBuffer[6]		= 0x00;
	OutputBuffer[7]		= 0x02;
	OutputBuffer[8]		= 0xA0;
	OutputBuffer[9]		= 0x03;
	OutputBuffer[10]	= 0x0A;

	Result = SendCmd();

	OutputBuffer[0]		= 0x00;
	OutputBuffer[1]		= 0x01;
	OutputBuffer[2]		= CmdSequence;
	OutputBuffer[3]		= 0x11;
	OutputBuffer[4]		= 0x10;
	OutputBuffer[5]		= 0x39;
	OutputBuffer[6]		= 0x01;
	OutputBuffer[7]		= 0x01;
	OutputBuffer[8]		= 0x80;
	OutputBuffer[9]		= 0x03;
	OutputBuffer[10]	= 0x0A;

	Result = SendCmd();

	return TRUE;
}

BOOL AmbientLightSensorDev::SetReportRequest()
{
	BOOL Result;

	OutputBuffer[0]		= 0x00;
	OutputBuffer[1]		= 0x01;
	OutputBuffer[2]		= CmdSequence;
	OutputBuffer[3]		= 0x11;
	OutputBuffer[4]		= 0x10;
	OutputBuffer[5]		= 0x39;
	OutputBuffer[6]		= 0x02;
	OutputBuffer[7]		= 0x01;
	OutputBuffer[8]		= 0xB6;
	OutputBuffer[9]		= 0x03;
	OutputBuffer[10]	= 0x0A;

	InputBuffer[0] = OutputBuffer[0];
	InputBuffer[1] = OutputBuffer[1];
	InputBuffer[2] = OutputBuffer[2];
	InputBuffer[3] = OutputBuffer[3];
	InputBuffer[4] = OutputBuffer[4];
	InputBuffer[5] = OutputBuffer[5];
	InputBuffer[6] = OutputBuffer[6];

	SendCmd();
	Sleep(1000);

	OutputBuffer[0] = 0x00;
	OutputBuffer[1] = 0x01;
	OutputBuffer[2] = CmdSequence;
	OutputBuffer[3] = 0x11;
	OutputBuffer[4] = 0x10;
	OutputBuffer[5] = 0x39;
	OutputBuffer[6] = 0x02;
	OutputBuffer[7] = 0x01;
	OutputBuffer[8] = 0x96;
	OutputBuffer[9] = 0x03;
	OutputBuffer[10] = 0x0A;

	InputBuffer[0] = OutputBuffer[0];
	InputBuffer[1] = OutputBuffer[1];
	InputBuffer[2] = OutputBuffer[2];
	InputBuffer[3] = OutputBuffer[3];
	InputBuffer[4] = OutputBuffer[4];
	InputBuffer[5] = OutputBuffer[5];
	InputBuffer[6] = OutputBuffer[6];

	Result = ReadInput();

	return TRUE;
}

BOOL AmbientLightSensorDev::SetADC_PowerOff()
{
	BOOL Result;

	OutputBuffer[0]		= 0x00;
	OutputBuffer[1]		= 0x01;
	OutputBuffer[2]		= CmdSequence;
	OutputBuffer[3]		= 0x11;
	OutputBuffer[4]		= 0x10;
	OutputBuffer[5]		= 0x39;
	OutputBuffer[6]		= 0x00;
	OutputBuffer[7]		= 0x02;
	OutputBuffer[8]		= 0x80;
	OutputBuffer[9]		= 0x00;
	OutputBuffer[10]	= 0x0A;

	SendCmd();

	return TRUE;
}
*/