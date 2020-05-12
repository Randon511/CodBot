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
static DWORD_PTR ZOMBIE_X_OFFSET = 0x180;
static DWORD_PTR ZOMBIE_Y_OFFSET = 0x184;
static DWORD_PTR ZOMBIE_Z_OFFSET = 0x20;

//player addresses
static DWORD_PTR PLAYER_ENTITY = 0x312000C;
static DWORD_PTR PLAYER_HP = 0x1C8;
static DWORD_PTR PLAYER_X_OFFSET = 0x60;
static DWORD_PTR PLAYER_Y_OFFSET = 0x84;
static DWORD_PTR PLAYER_Z_OFFSET = 0x88;

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
		DWORD_PTR player = baseAddress + PLAYER_ENTITY;
		DWORD_PTR playerXBase;
		DWORD_PTR playerYZBase;
		DWORD_PTR zombieEntity;
		int zombieHpValue;
		float zombieX;
		float zombieY;
		float zombieZ;
		float playerX;
		float playerY;
		float playerZ;
		
		//Grabbing pointer for player position
		if (ReadProcessMemory(handle, (LPCVOID*)player, &playerYZBase, sizeof(playerYZBase), NULL)) 
		{ 
			if (ReadProcessMemory(handle, (LPCVOID*)playerYZBase, &playerXBase, sizeof(playerXBase), NULL)) 
			{
			}
			else 
			{
				std::cout << "error reading x pointer" << std::endl;
			}
		}
		else 
		{
			std::cout << "error reading zy pointer" << std::endl;
		}

		while (1) 
		{
			//Put the player base pointer for y and z into playerXbase
			if (playerYZBase)
			{
				ReadProcessMemory(handle, (LPCVOID*)(playerYZBase + PLAYER_Y_OFFSET), &playerY, sizeof(playerYZBase), NULL);
				ReadProcessMemory(handle, (LPCVOID*)(playerYZBase + PLAYER_Z_OFFSET), &playerZ, sizeof(playerYZBase), NULL);

				//Put player base pointer for x into playerXbase
				if (playerXBase)
				{
					ReadProcessMemory(handle, (LPCVOID*)(playerXBase + PLAYER_X_OFFSET), &playerX, sizeof(playerXBase), NULL);
					std::cout << "(" << playerX << "," << playerY << "," << playerZ << ")" << std::endl;
				}
				else
				{
					std::cout << "error finding zy pointer" << std::endl;
				}
			}
			else 
			{
				std::cout << "error finding zy pointer" << std::endl; 
			}

			/*
			for (int i = 0; i < 10; i++)
			{
				DWORD_PTR zombie = baseAddress + ZOMBIE_ENTITY_LIST + (i * ZOMBIE_OFFSET);

				//Print zombie health and xyz positions
				if (ReadProcessMemory(handle, (LPCVOID*)(zombieEntity + ZOMBIE_HP_OFFSET), &zombieHpValue, sizeof(zombieHpValue), NULL))
				{
						
					if (zombieHpValue > 0)
					{
						ReadProcessMemory(handle, (LPCVOID*)(numericalAddressOfHp + ZOMBIE_X_OFFSET), &zombieX, sizeof(zombieX), NULL);
						ReadProcessMemory(handle, (LPCVOID*)(numericalAddressOfHp + ZOMBIE_Y_OFFSET), &zombieY, sizeof(zombieY), NULL);
						ReadProcessMemory(handle, (LPCVOID*)(numericalAddressOfHp + ZOMBIE_Z_OFFSET), &zombieZ, sizeof(zombieZ), NULL);
						std::cout << "| " << i << ":" << zombieHpValue << "(" << zombieX << "," << zombieY << "," << zombieZ << ") | ";			
					}
					else
					{
						//std::cout << "Unable to read hp value" << std::endl;
						break;
					}
				}
				else
				{
					std::cout << "Unable to read memory" << std::endl;
				}
			}
			*/

			Sleep(100);
		}

		CloseHandle(handle);
	}
	return 0;
}