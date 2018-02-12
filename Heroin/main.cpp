#include "needle.h"

#ifdef _WIN64
const char szDllFile[] = "c:\\Users\\Chris\\source\\repos\\Test\\x64\\Release\\Test.dll";
const char szProc[] = "notepad.exe";
#else
const char szDllFile[] = "PATH TO x86 DLL";
const char szProc[] = "x86 EXE";
#endif

bool IsCorrectTargetArchitecture(HANDLE hProc)
{
	BOOL bTarget = FALSE;
	if (!IsWow64Process(hProc, &bTarget))
	{
		printf("Architecture Error: 0x%X\n", GetLastError());
		return false;
	}

	BOOL bHost = FALSE;
	IsWow64Process(GetCurrentProcess(), &bHost);

	return (bTarget == bHost);
}

int main()
{
	PROCESSENTRY32 PE32{ 0 };
	PE32.dwSize = sizeof(PE32);

	HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (hSnap == INVALID_HANDLE_VALUE)
	{
		DWORD Err = GetLastError();
		printf("FAILED CreateToolhelp32Snapshot(): 0x%X\n", Err);
		getchar();
		return 0;
	}

	DWORD PID = 0;
	BOOL bRet = Process32First(hSnap, &PE32);
	while (bRet)
	{
		if (!strcmp(szProc, PE32.szExeFile))
		{
			PID = PE32.th32ProcessID;
			break;
		}
		bRet = Process32Next(hSnap, &PE32);
	}

	CloseHandle(hSnap);

	HANDLE hProc = OpenProcess(PROCESS_ALL_ACCESS, FALSE, PID);
	if (!hProc)
	{
		DWORD Err = GetLastError();
		printf("FAILED OpenProcess(): 0x%X\n", Err);
		getchar();
		return 0;
	}

	if (!IsCorrectTargetArchitecture(hProc))
	{
		printf("TARGET ARCHITECTURE FAIL..Invalid Process?\n");
		CloseHandle(hProc);
		getchar();
		return 0;
	}

	if (!ManualMap(hProc, szDllFile))
	{
		CloseHandle(hProc);
		printf("Failed to map :[\n");
		getchar();
		return 0;
	}

	CloseHandle(hProc);
	printf("Mapping Done\n");
	getchar();
	return 0;
}