// DumpWindowsDeviceInfo.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"
#include <iostream>

#include <Windows.h>
#pragma comment(lib,"user32.lib")

#include <intrin.h>

#include <wbemidl.h>
#include <comdef.h>
#pragma comment(lib, "wbemuuid.lib")
void dumpDeviceA(DISPLAY_DEVICEA* displayDevice, size_t nSpaceCount)
{
	printf("%*sDevice Name: %s\n", nSpaceCount, "", displayDevice->DeviceName);
	printf("%*sDevice String: %s\n", nSpaceCount, "", displayDevice->DeviceString);
	printf("%*sState Flags: %x\n", nSpaceCount, "", displayDevice->StateFlags);
	if (displayDevice->StateFlags & DISPLAY_DEVICE_ACTIVE) { printf(" ACTIVE"); }
	if (displayDevice->StateFlags & DISPLAY_DEVICE_MIRRORING_DRIVER) { printf(" MIRRORING_DRIVER"); }
	if (displayDevice->StateFlags & DISPLAY_DEVICE_MODESPRUNED) { printf(" MODESPRUNED"); }
	if (displayDevice->StateFlags & DISPLAY_DEVICE_PRIMARY_DEVICE) { printf(" PRIMARY_DEVICE"); }
	if (displayDevice->StateFlags & DISPLAY_DEVICE_REMOVABLE) { printf(" REMOVABLE"); }
	if (displayDevice->StateFlags & DISPLAY_DEVICE_VGA_COMPATIBLE) { printf(" VGA_COMPATIBLE"); }

	printf("\n");
	printf("%*sDeviceID: %s\n", nSpaceCount, "", displayDevice->DeviceID);
	printf("%*sDeviceKey: ...%s\n\n", nSpaceCount, "", displayDevice->DeviceKey + 42);
}

void dumpDisplaySettings(DISPLAY_DEVICEA* displayDevice, const char* indent = "")
{
	int i = 0;
	DEVMODEA mode;
	mode.dmSize = sizeof(DEVMODEA);
	mode.dmDriverExtra = 0;

	printf("---------- Display Current Mode  ----------\n");

	if (EnumDisplaySettingsA(displayDevice->DeviceName, ENUM_CURRENT_SETTINGS, &mode))
	{
		printf("%sBitsPerPixel: %d\n", indent, mode.dmBitsPerPel);
		printf("%sWidth: %d\n", indent, mode.dmPelsWidth);
		printf("%sHeight: %d\n", indent, mode.dmPelsHeight);
		printf("%sFrequency: %d\n", indent, mode.dmDisplayFrequency);
		printf("-------------------------------------------\n");
	}

	while (EnumDisplaySettingsA(displayDevice->DeviceName, i, &mode))
	{
		//printf("%s----- Display Available Mode %d -----\n", indent, i);
		//printf("%sBitsPerPixel: %d\n", indent, mode.dmBitsPerPel);
		//printf("%sWidth: %d\n", indent, mode.dmPelsWidth);
		//printf("%sHeight: %d\n", indent, mode.dmPelsHeight);
		//printf("%sFrequency: %d\n", indent, mode.dmDisplayFrequency);
		//printf("\n");
		i++;
	}
	printf("%sTotal display modes: %d\n", indent, i);
}

void dumpDisplayDevicesA() 
{
	DISPLAY_DEVICEA dd;

	dd.cb = sizeof(DISPLAY_DEVICEA);

	DWORD deviceNum = 0;
	while (EnumDisplayDevicesA(NULL, deviceNum, &dd, 0)) {
		dumpDeviceA(&dd, 0);
		dumpDisplaySettings(&dd, "\t");
		DISPLAY_DEVICEA newdd = { 0 };
		newdd.cb = sizeof(DISPLAY_DEVICEA);
		DWORD monitorNum = 0;
		while (EnumDisplayDevicesA(dd.DeviceName, monitorNum, &newdd, 0))
		{
			dumpDeviceA(&newdd, 4);
			dumpDisplaySettings(&newdd, "\t\t");
			monitorNum++;
		}
		puts("");
		deviceNum++;
	}
}


void dumpCPUInfo(std::string& cpuType, int& numberOfCores, int& numberOfLogicalProcessors, std::string& instructionSet)
{
	int CPUInfo[4] = { -1 };
	//############################# Get Number of Cores #############################//
	// Get vendor
	char vendor[12];
	__cpuid(CPUInfo, 0);
	((unsigned *)vendor)[0] = CPUInfo[1]; // EBX
	((unsigned *)vendor)[1] = CPUInfo[3]; // EDX
	((unsigned *)vendor)[2] = CPUInfo[2]; // ECX
	std::string cpuVendor = std::string(vendor, 12);

	// Get CPU features
	__cpuid(CPUInfo, 1);
	unsigned cpuFeatures = CPUInfo[3]; // EDX

	// Logical core count per CPU
	__cpuid(CPUInfo, 1);
	unsigned logical = (CPUInfo[1] >> 16) & 0xff; // EBX[23:16]
	numberOfLogicalProcessors = logical;
	unsigned cores = logical;

	if (cpuVendor == "GenuineIntel") {
		// Get DCP cache info
		__cpuid(CPUInfo, 4);
		cores = ((CPUInfo[0] >> 26) & 0x3f) + 1; // EAX[31:26] + 1

	}
	else if (cpuVendor == "AuthenticAMD") {
		// Get NC: Number of CPU cores - 1
		__cpuid(CPUInfo, 0x80000008);
		cores = ((unsigned)(CPUInfo[2] & 0xff)) + 1; // ECX[7:0] + 1
	}
	numberOfCores = cores;

	//############################# Get Instruction Set #############################//
	//  Misc.
	bool HW_MMX = false;
	bool HW_x64 = false;
	bool HW_ABM = false;      // Advanced Bit Manipulation
	bool HW_RDRAND = false;
	bool HW_BMI1 = false;
	bool HW_BMI2 = false;
	bool HW_ADX = false;
	bool HW_PREFETCHWT1 = false;

	//  SIMD: 128-bit
	bool HW_SSE = false;
	bool HW_SSE2 = false;
	bool HW_SSE3 = false;
	bool HW_SSSE3 = false;
	bool HW_SSE41 = false;
	bool HW_SSE42 = false;
	bool HW_SSE4a = false;
	bool HW_AES = false;
	bool HW_SHA = false;

	//  SIMD: 256-bit
	bool HW_AVX = false;
	bool HW_XOP = false;
	bool HW_FMA3 = false;
	bool HW_FMA4 = false;
	bool HW_AVX2 = false;

	//  SIMD: 512-bit
	bool HW_AVX512F = false;    //  AVX512 Foundation
	bool HW_AVX512CD = false;   //  AVX512 Conflict Detection
	bool HW_AVX512PF = false;   //  AVX512 Prefetch
	bool HW_AVX512ER = false;   //  AVX512 Exponential + Reciprocal
	bool HW_AVX512VL = false;   //  AVX512 Vector Length Extensions
	bool HW_AVX512BW = false;   //  AVX512 Byte + Word
	bool HW_AVX512DQ = false;   //  AVX512 Doubleword + Quadword
	bool HW_AVX512IFMA = false; //  AVX512 Integer 52-bit Fused Multiply-Add
	bool HW_AVX512VBMI = false; //  AVX512 Vector Byte Manipulation Instructions

	__cpuid(CPUInfo, 0);
	int nIds = CPUInfo[0];

	// Get extended ids.
	__cpuid(CPUInfo, 0x80000000);
	unsigned int nExIds = CPUInfo[0];

	//  Detect Features
	if (nIds >= 0x00000001) {
		__cpuid(CPUInfo, 0x00000001);
		HW_MMX = (CPUInfo[3] & ((int)1 << 23)) != 0;
		HW_SSE = (CPUInfo[3] & ((int)1 << 25)) != 0;
		HW_SSE2 = (CPUInfo[3] & ((int)1 << 26)) != 0;
		HW_SSE3 = (CPUInfo[2] & ((int)1 << 0)) != 0;

		HW_SSSE3 = (CPUInfo[2] & ((int)1 << 9)) != 0;
		HW_SSE41 = (CPUInfo[2] & ((int)1 << 19)) != 0;
		HW_SSE42 = (CPUInfo[2] & ((int)1 << 20)) != 0;
		HW_AES = (CPUInfo[2] & ((int)1 << 25)) != 0;

		HW_AVX = (CPUInfo[2] & ((int)1 << 28)) != 0;
		HW_FMA3 = (CPUInfo[2] & ((int)1 << 12)) != 0;

		HW_RDRAND = (CPUInfo[2] & ((int)1 << 30)) != 0;
	}
	if (nIds >= 0x00000007) {
		__cpuid(CPUInfo, 0x00000007);
		HW_AVX2 = (CPUInfo[1] & ((int)1 << 5)) != 0;

		HW_BMI1 = (CPUInfo[1] & ((int)1 << 3)) != 0;
		HW_BMI2 = (CPUInfo[1] & ((int)1 << 8)) != 0;
		HW_ADX = (CPUInfo[1] & ((int)1 << 19)) != 0;
		HW_SHA = (CPUInfo[1] & ((int)1 << 29)) != 0;
		HW_PREFETCHWT1 = (CPUInfo[2] & ((int)1 << 0)) != 0;

		HW_AVX512F = (CPUInfo[1] & ((int)1 << 16)) != 0;
		HW_AVX512CD = (CPUInfo[1] & ((int)1 << 28)) != 0;
		HW_AVX512PF = (CPUInfo[1] & ((int)1 << 26)) != 0;
		HW_AVX512ER = (CPUInfo[1] & ((int)1 << 27)) != 0;
		HW_AVX512VL = (CPUInfo[1] & ((int)1 << 31)) != 0;
		HW_AVX512BW = (CPUInfo[1] & ((int)1 << 30)) != 0;
		HW_AVX512DQ = (CPUInfo[1] & ((int)1 << 17)) != 0;
		HW_AVX512IFMA = (CPUInfo[1] & ((int)1 << 21)) != 0;
		HW_AVX512VBMI = (CPUInfo[2] & ((int)1 << 1)) != 0;
	}
	if (nExIds >= 0x80000001) {
		__cpuid(CPUInfo, 0x80000001);
		HW_x64 = (CPUInfo[3] & ((int)1 << 29)) != 0;
		HW_ABM = (CPUInfo[2] & ((int)1 << 5)) != 0;
		HW_SSE4a = (CPUInfo[2] & ((int)1 << 6)) != 0;
		HW_FMA4 = (CPUInfo[2] & ((int)1 << 16)) != 0;
		HW_XOP = (CPUInfo[2] & ((int)1 << 11)) != 0;
	}

	std::string instruction_set;
	//  Misc.
	if (HW_MMX) instruction_set += "MMX ";
	if (HW_x64) instruction_set += "x64 ";
	if (HW_ABM) instruction_set += "ABM ";
	if (HW_RDRAND) instruction_set += "RDRAND ";
	if (HW_BMI1) instruction_set += "BMI1 ";
	if (HW_BMI2) instruction_set += "BMI2 ";
	if (HW_ADX) instruction_set += "ADX ";
	if (HW_PREFETCHWT1) instruction_set += "PREFETCHWT1 ";

	//  SIMD: 128-bit
	if (HW_SSE) instruction_set += "SSE ";
	if (HW_SSE2) instruction_set += "SSE2 ";
	if (HW_SSE3) instruction_set += "SSE3 ";
	if (HW_SSSE3) instruction_set += "SSSE3 ";
	if (HW_SSE41) instruction_set += "SSE41 ";
	if (HW_SSE42) instruction_set += "SSE42 ";
	if (HW_SSE4a) instruction_set += "SSE4a ";
	if (HW_AES) instruction_set += "AES ";
	if (HW_SHA) instruction_set += "SHA ";

	//  SIMD: 256-bit
	if (HW_AVX) instruction_set += "AVX ";
	if (HW_XOP) instruction_set += "XOP ";
	if (HW_FMA3) instruction_set += "FMA3 ";
	if (HW_FMA4) instruction_set += "FMA4 ";
	if (HW_AVX2) instruction_set += "AVX2 ";

	//  SIMD: 512-bit
	if (HW_AVX512F) instruction_set += "AVX512F ";
	if (HW_AVX512CD) instruction_set += "AVX512CD ";
	if (HW_AVX512PF) instruction_set += "AVX512PF ";
	if (HW_AVX512ER) instruction_set += "AVX512ER ";
	if (HW_AVX512VL) instruction_set += "AVX512VL ";
	if (HW_AVX512BW) instruction_set += "AVX512BW ";
	if (HW_AVX512DQ) instruction_set += "AVX512DQ ";
	if (HW_AVX512IFMA) instruction_set += "AVX512IFMA ";
	if (HW_AVX512VBMI) instruction_set += "AVX512VBMI ";

	instructionSet = instruction_set;

	//############################# Get CPU Type #############################//
	// Get the information associated with each extended ID.
	char CPUBrandString[0x40] = { 0 };
	for (unsigned int i = 0x80000000; i <= nExIds; ++i)
	{
		__cpuid(CPUInfo, i);

		// Interpret CPU brand string and cache information.
		if (i == 0x80000002)
		{
			memcpy(CPUBrandString,
				CPUInfo,
				sizeof(CPUInfo));
		}
		else if (i == 0x80000003)
		{
			memcpy(CPUBrandString + 16,
				CPUInfo,
				sizeof(CPUInfo));
		}
		else if (i == 0x80000004)
		{
			memcpy(CPUBrandString + 32, CPUInfo, sizeof(CPUInfo));
		}
	}

	cpuType = std::string(CPUBrandString);
}

float getTotalMemory(){
	MEMORYSTATUSEX statex;
	statex.dwLength = sizeof(statex);
	GlobalMemoryStatusEx(&statex);
	return statex.ullTotalPhys;
}

std::wstring getLinkSpeedNetInfo(std::wstring adapter)
{
	CoInitializeEx(0, 0);
	CoInitializeSecurity(0, -1, 0, 0, 0, 3, 0, 0, 0);
	IWbemLocator *locator = 0;
	CoCreateInstance(CLSID_WbemLocator, 0, CLSCTX_INPROC_SERVER, IID_IWbemLocator, (void **)&locator);
	IWbemServices * services = 0;

	BSTR name = SysAllocString(L"root\\cimv2");
	if (SUCCEEDED(locator->ConnectServer(name, 0, 0, 0, 0, 0, 0, &services))) {
		// printf("Connected!\n");

		//Lets get system information
		CoSetProxyBlanket(services, 10, 0, 0, 3, 3, 0, 0);
		BSTR language = SysAllocString(L"WQL");
		std::wstring strQuery = L"SELECT * FROM Win32_NetworkAdapter where NetEnabled ='TRUE' and Name = '" + adapter + L"'";

		BSTR query = SysAllocString(strQuery.c_str());

		IEnumWbemClassObject *e = 0;
		if (SUCCEEDED(services->ExecQuery(language, query, WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY, 0, &e))) {
			// printf("Query executed successfuly!\n");

			IWbemClassObject *object = 0;
			ULONG u = 0;
			//lets enumerate all data from this table
			while (e) {
				e->Next(WBEM_INFINITE, 1, &object, &u);
				if (!u) break;//no more data,end enumeration
				VARIANT data;
				/*
					Remember,if the type of the class members is on MSDN noted as [string],we can use
					the Variant's bstrVal member
					If the type is string something[],we have to use :
					1.((BSTR*)(data.parray->pvData))[0]
					OR
					2.The safe array element method
				*/
				//Network adapter name
				// if (SUCCEEDED(object->Get(L"Name", 0, &data, 0, 0))){
				//	if (V_VT(&data) == VT_BSTR)
				//		std::wstring www = std::wstring(data.bstrVal);
				//		wprintf(L"%s\n", std::wstring(data.bstrVal).c_str());
				// }

				// if (SUCCEEDED(object->Get(L"Description", 0, &data, 0, 0))){
				//	 if (V_VT(&data) == VT_BSTR)
				//		 wprintf(L"%s\n", std::wstring(data.bstrVal).c_str());
				// }

				if (SUCCEEDED(object->Get(L"Speed", 0, &data, 0, 0))) {
					if (V_VT(&data) == VT_BSTR) {
						return std::wstring(data.bstrVal);
					}
				}
				VariantClear(&data);
			}
		}
		else {
			printf("Error executing query!\n");
		}
	}
	else {
		printf("Connection error!\n");
	}
	//Close all used data
	services->Release();
	locator->Release();
	CoUninitialize();

	return L"";
}
int main()
{
	//// GPU & Display Devices
	printf("\n######################### GPU & Display #########################\n");
	dumpDisplayDevicesA();

	//// CPU Info
	printf("\n######################### CPU #########################\n");
	std::string cpu_type, cpu_instructions;
	int number_cores, number_logical_processors;
	dumpCPUInfo(cpu_type, number_cores, number_logical_processors, cpu_instructions);

	printf("CPU Type : %s\n", cpu_type.c_str());
	printf("Cores : %d\n", number_cores);
	printf("Logical Processors : %d\n", number_logical_processors);
	printf("Instructions : %s\n", cpu_instructions.c_str());

	//// Memory
	printf("\n######################### Memory #########################\n");
	printf("\nTotal Memory : %f\n", getTotalMemory());

	//// Network
	printf("\n######################### Network Link Speed#########################\n");
	wprintf(L"Network Link Speed : %s\n", getLinkSpeedNetInfo(L"Intel(R) Ethernet Connection (7) I219-V").c_str());
}
