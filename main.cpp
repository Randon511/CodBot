#include<iostream>
#include<Windows.h>
#include<vector>
#include<cstring>
#include<Psapi.h>
#include<sstream>

//zombie addresses
static DWORD_PTR ZOMBIE_ENTITY_LIST = 0x14E74D0;
static DWORD_PTR ZOMBIE_OFFSET = 0x88;
static DWORD_PTR ZOMBIE_HP_OFFSET = 0x1C8;
static DWORD_PTR ZOMBIE_X_OFFSET = 0x148;
static DWORD_PTR ZOMBIE_Y_OFFSET = 0x14C;
static DWORD_PTR ZOMBIE_Z_OFFSET = 0x150;

//player addresses
DWORD_PTR getModule(HANDLE handle, std::wstring exeName)
{	
	DWORD_PTR base = 0;
	HMODULE hModules[1024];
	DWORD cbNeeded;
	if (EnumProcessModules(handle, hModules, sizeof(hModules), &cbNeeded))
	{
		//cbNeeded/sizeof(HMODULE) == number of modules
		for (unsigned int i = 0; i < (cbNeeded / sizeof(HMODULE)); i++)
		{
			//MAX_PATH is 260 chars 
			wchar_t modName[MAX_PATH];

			if (GetModuleFileNameEx(handle, hModules[i], modName, sizeof(modName)/sizeof(TCHAR)))
			{
				std::wstring modNameString = modName;
				
				if (modNameString.find(exeName))
				{
					base = (DWORD_PTR)hModules[i];
					break;
				}
			
			}
		}
	}
	return base; 
}

int main(void)
{
	std::vector<std::wstring> titles;
	HWND hwnd = FindWindowA(0, ("Call of Duty®"));

	if (hwnd == 0)
	{
		std::cout << "Error, no window found" << std::endl;
		return -1;
	}
	else
	{
		//Get processID
		DWORD procId;
		GetWindowThreadProcessId(hwnd, &procId);
		HANDLE handle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, procId);

		if (procId == 0)
		{
			std::cout << "Error, no proccess Id found" << std::endl;
			return -1;
		}

		std::wstring exeName = L"CoDWaW.exe";
		DWORD_PTR baseAddress = getModule(handle, exeName);
		DWORD_PTR zombieHpAddress = baseAddress + ZOMBIE_ENTITY_LIST;
		int numericalAddressOfHp;
		int zombieHpValue;
		
		if (ReadProcessMemory(handle, (LPCVOID*)zombieHpAddress, &numericalAddressOfHp, sizeof(numericalAddressOfHp), NULL))
		{
			if (ReadProcessMemory(handle, (LPCVOID*)(numericalAddressOfHp + ZOMBIE_HP_OFFSET), &zombieHpValue, sizeof(zombieHpValue), NULL))
			{
				std::cout << zombieHpValue << std::endl;
			}
			else 
			{
				std::cout << "Unable to read hp value" << std::endl;
			}
		}
		else 
		{
			std::cout << "Unable to read memory" << std::endl;
		}

		CloseHandle(handle);
	}
	return 0;
}