#include<iostream>
#include<Windows.h>
#include<vector>
#include<cstring>
#include<Psapi.h>
#include<sstream>
#include<math.h>

//zombie addresses
//static DWORD_PTR ZOMBIE_ENTITY_LIST = 0x14E74D0;
static DWORD_PTR ZOMBIE_ENTITY_LIST = 0x14E7448;
static DWORD_PTR ZOMBIE_OFFSET = 0x88;
static DWORD_PTR ZOMBIE_HP_OFFSET = 0x1C8;
static DWORD_PTR ZOMBIE_X_OFFSET = 0x18;
static DWORD_PTR ZOMBIE_Y_OFFSET = 0x1C;
static DWORD_PTR ZOMBIE_Z_OFFSET = 0x20;
static DWORD_PTR FIRST_ZOMBIE_X_OFFSET = 0x390;
static DWORD_PTR FIRST_ZOMBIE_Y_OFFSET = 0x394;
static DWORD_PTR FIRST_ZOMBIE_Z_OFFSET = 0x398;

//player addresses
static DWORD_PTR PLAYER_ENTITY = 0x312000C;
static DWORD_PTR PLAYER_HP = 0x1C8;
static DWORD_PTR PLAYER_X_OFFSET = 0x60;
static DWORD_PTR PLAYER_Y_OFFSET = 0x84;
static DWORD_PTR PLAYER_Z_OFFSET = 0x88;
static DWORD_PTR PLAYER_AIM_X = 0x2C7D6D4;
static DWORD_PTR PLAYER_AIM_Y = 0x2C7D6D0;

//executable name
static std::wstring exeName = L"CoDWaW.exe";



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

//Returns the players coordinates
std::vector<float> getPlayerInfo(HANDLE handle, DWORD_PTR base)
{
	DWORD_PTR playerEntity = base + PLAYER_ENTITY;
	DWORD_PTR playerYZBase;
	DWORD_PTR playerXBase;
	float playerX;
	float playerY;
	float playerZ;

	std::vector<float> result; 

	//Put the player base pointer for y and z into playerYZbase
	if (ReadProcessMemory(handle, (LPCVOID*)playerEntity, &playerYZBase, sizeof(playerYZBase), NULL))
	{
		ReadProcessMemory(handle, (LPCVOID*)(playerYZBase + PLAYER_Y_OFFSET), &playerY, sizeof(playerYZBase), NULL);
		ReadProcessMemory(handle, (LPCVOID*)(playerYZBase + PLAYER_Z_OFFSET), &playerZ, sizeof(playerYZBase), NULL);

		//Put player base pointer for X into playerXbase
		if (ReadProcessMemory(handle, (LPCVOID*)playerYZBase, &playerXBase, sizeof(playerXBase), NULL))
		{
			ReadProcessMemory(handle, (LPCVOID*)(playerXBase + PLAYER_X_OFFSET), &playerX, sizeof(playerXBase), NULL);
			
			result.push_back(playerX);
			result.push_back(playerY);
			result.push_back(playerZ);
		}
		else
		{
			std::cout << "Error finding x coordinate" << std::endl;
		}
	}
	else
	{
		std::cout << "Error finding y/z coordinate" << std::endl;
	}
	return result;
}

//Returns a vector of vectors containg zombie coordinates
std::vector<std::vector<float>> getZombieInfo(HANDLE handle, DWORD_PTR base)
{
	DWORD_PTR zombieEntityList = base + ZOMBIE_ENTITY_LIST;
	DWORD_PTR zombieEntity;
	DWORD_PTR zombie;
	int zombieHp;
	float zombieX;
	float zombieY;
	float zombieZ;

	std::vector<std::vector<float>> result;
	
	for (int i = 0; i < 1000; i++)
	{
		//Offset for each zombie in the list
		zombieEntity = zombieEntityList + (i * ZOMBIE_OFFSET);
		std::vector<float> zombieResult;

		if (ReadProcessMemory(handle, (LPCVOID*)(zombieEntity), &zombie, sizeof(zombie), NULL))
		{
			if (zombie < base ) 
			{
				return result; 
			}

			if (ReadProcessMemory(handle, (LPCVOID*)(zombie + ZOMBIE_HP_OFFSET), &zombieHp, sizeof(zombieHp), NULL))
			{
				//Only return the coordinates of zombies that are still alive
				if (zombieHp > 0)
				{
					//The first zombie in the entity list is different from the rest
					if (i == 0) 
					{
						ReadProcessMemory(handle, (LPCVOID*)(zombie + FIRST_ZOMBIE_X_OFFSET), &zombieX, sizeof(zombieX), NULL);
						ReadProcessMemory(handle, (LPCVOID*)(zombie + FIRST_ZOMBIE_Y_OFFSET), &zombieY, sizeof(zombieY), NULL);
						ReadProcessMemory(handle, (LPCVOID*)(zombie + FIRST_ZOMBIE_Z_OFFSET), &zombieZ, sizeof(zombieZ), NULL);

						zombieResult.push_back(zombieX);
						zombieResult.push_back(zombieY);
						zombieResult.push_back(zombieZ);
						result.push_back(zombieResult);
					}
					else 
					{
						ReadProcessMemory(handle, (LPCVOID*)(zombie + ZOMBIE_X_OFFSET), &zombieX, sizeof(zombieX), NULL);
						ReadProcessMemory(handle, (LPCVOID*)(zombie + ZOMBIE_Y_OFFSET), &zombieY, sizeof(zombieY), NULL);
						ReadProcessMemory(handle, (LPCVOID*)(zombie + ZOMBIE_Z_OFFSET), &zombieZ, sizeof(zombieZ), NULL);

						zombieResult.push_back(zombieX);
						zombieResult.push_back(zombieY);
						zombieResult.push_back(zombieZ);
						result.push_back(zombieResult);
					}
					
				}
			}
			else 
			{
				std::cout << "{" << i << "}" << "Unable to read zombie hp value" << std::endl;
				break;
			}
		}
		else
		{
			std::cout << "Unable to read zombie entity address" << std::endl;
			break;
		}
	}
	return result;
}

//This function returns the vector that points from the player to the zombie and the magnitude
std::vector<float> makeVector(std::vector<float> player, std::vector<float> zombie)
{
	std::vector<float> result;
	float mag = 0;
	int diff; 
	//vec from player to zombie
	for (int i = 0; i < 3; i++) 
	{
		diff = player[i] - zombie[i];
		result.push_back(diff);
	}

	mag = sqrt((result[0] * result[0]) + (result[1] * result[1]) + (result[2] * result[2]));
	result.push_back(mag);

	return result; 
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

		DWORD_PTR baseAddress = getModule(handle, exeName);
		
		std::vector<float> player;
		std::vector<std::vector<float>> zombies;

		while (1)
		{
			player = getPlayerInfo(handle, baseAddress);
			zombies = getZombieInfo(handle, baseAddress);
			std::vector<std::vector<float>> vectors;
			//print player xyz
			//printf("(%f,%f,%f)|||", player[0], player[1], player[2]);
			
			printf("===========================================================================\n");

			for (unsigned int i = 0; i < zombies.size(); i++)
			{
				//printf("(%f,%f,%f)", zombies[i][0], zombies[i][1], zombies[i][2]);
				vectors.push_back(makeVector(player, zombies[i]));
			}

			for (unsigned int i = 0; i < vectors.size(); i++)
			{
				printf("(%f,%f,%f,%f)", vectors[i][0], vectors[i][1], vectors[i][2], vectors[i][3]);
			}
			
			printf("\n");
			Sleep(100);
		}
		CloseHandle(handle);
	}
	return 0;
}