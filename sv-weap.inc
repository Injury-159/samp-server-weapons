#if defined _B_SERVER_HACK_
	#endinput
#endif
#define _B_WEAPON_HACK_

#include <a_samp>
#include <Pawn.RakNet>

/*Enum*/
enum B_Weapon_Data_Enum
{
	bool:B_Weapon[46],
	B_Ammo[46],
	B_Spawn_Weapon[3],
	B_Spawn_Ammo[3]
}

/*Variables*/
static B_Weapon_Data[MAX_PLAYERS][B_Weapon_Data_Enum];
	
/*Incoming Packet IDs*/
const IPacket_DriverSync = 200;
const IPacket_OnFootSync = 207;
const IPacket_PassengerSync = 211;

/*Outgoing RPC IDs*/
const ORPC_ResetPlayerWeapons = 21;
const ORPC_GivePlayerWeapon = 22;
const ORPC_SetSpawnInfo = 68;
const ORPC_RequestClass = 128;
const ORPC_SetPlayerAmmo = 145;

/*OnPlayerConnect*/
public OnPlayerConnect(playerid)
{
	B_Weapon_Data[playerid][B_Spawn_Weapon][0] = 0;
	B_Weapon_Data[playerid][B_Spawn_Weapon][1] = 0;
	B_Weapon_Data[playerid][B_Spawn_Weapon][2] = 0;
	
	#if defined BWH_OnPlayerConnect
		return BWH_OnPlayerConnect(playerid);
	#else
		return 1;
	#endif
}

/*OnPlayerSpawn*/
public OnPlayerSpawn(playerid)
{	
	// Reset weapons and ammo
	BWH_memset(B_Weapon_Data[playerid][B_Weapon], 0);
	BWH_memset(B_Weapon_Data[playerid][B_Ammo], 0);
	
	// Allow spawn weapons
	if(B_Weapon_Data[playerid][B_Spawn_Weapon][0] > 0)
	{
		B_Weapon_Data[playerid][B_Weapon][B_Weapon_Data[playerid][B_Spawn_Weapon][0]] = true;
		B_Weapon_Data[playerid][B_Ammo][B_Weapon_Data[playerid][B_Spawn_Weapon][0]] = B_Weapon_Data[playerid][B_Spawn_Ammo][0];
	}
	
	if(B_Weapon_Data[playerid][B_Spawn_Weapon][1] > 0)
	{
		B_Weapon_Data[playerid][B_Weapon][B_Weapon_Data[playerid][B_Spawn_Weapon][1]] = true;
		B_Weapon_Data[playerid][B_Ammo][B_Weapon_Data[playerid][B_Spawn_Weapon][1]] = B_Weapon_Data[playerid][B_Spawn_Ammo][1];
	}
	
	if(B_Weapon_Data[playerid][B_Spawn_Weapon][2] > 0)
	{
		B_Weapon_Data[playerid][B_Weapon][B_Weapon_Data[playerid][B_Spawn_Weapon][2]] = true;
		B_Weapon_Data[playerid][B_Ammo][B_Weapon_Data[playerid][B_Spawn_Weapon][2]] = B_Weapon_Data[playerid][B_Spawn_Ammo][2];
	}
	
	#if defined BWH_OnPlayerSpawn
		return BWH_OnPlayerSpawn(playerid);
	#else
		return 1;
	#endif
}

/*OnPlayerWeaponShot*/
public OnPlayerWeaponShot(playerid, weaponid, hittype, hitid, Float:fX, Float:fY, Float:fZ)
{			
	if(!B_Weapon_Data[playerid][B_Weapon][weaponid]) // Not given by the server? Don't sync the bullets.
	{
		return 0;
	}
	else // Otherwise...
	{
		new BWH_Ammo = GetPlayerAmmo(playerid); 
				
		if(BWH_Ammo > B_Weapon_Data[playerid][B_Ammo][weaponid] && (weaponid != 28 && weaponid != 32 && weaponid != 38)) // These weapons are problematic, let's exclude them for now.
		{
			return 0;
		}
		else if(BWH_Ammo < B_Weapon_Data[playerid][B_Ammo][weaponid])
		{
			B_Weapon_Data[playerid][B_Ammo][weaponid] = BWH_Ammo;
		}
		else if(B_Weapon_Data[playerid][B_Ammo][weaponid] > 1)
		{
			B_Weapon_Data[playerid][B_Ammo][weaponid]--;
		}
		else
		{
			B_Weapon_Data[playerid][B_Weapon][weaponid] = false;
		}
	}
			
	#if defined BWH_OnPlayerWeaponShot
		return BWH_OnPlayerWeaponShot(playerid, weaponid, hittype, hitid, fX, fY, fZ);
	#else
		return 1;
	#endif
}

/*OnOutcomingRPC*/
public OnOutcomingRPC(playerid, rpcid, BitStream:bs)
{
	switch(rpcid)
	{
		case ORPC_ResetPlayerWeapons: // ResetPlayerWeapons
		{
			BWH_memset(B_Weapon_Data[playerid][B_Weapon], 0);
			BWH_memset(B_Weapon_Data[playerid][B_Ammo], 0);
		}
		case ORPC_GivePlayerWeapon: // GivePlayerWeapon
		{
			new BWH_weaponid, BWH_ammo, BWH_slot;
			
			BS_ReadValue(
				bs,
				PR_UINT32, BWH_weaponid,
				PR_UINT32, BWH_ammo
			);
			
			switch(BWH_weaponid) // Let's get the slot id of the weapon we are going to give belongs to.
			{
				case 0, 1: BWH_slot = 0;
				case 2..9: BWH_slot = 1;
				case 10..15: BWH_slot = 10;
				case 16..18, 39: BWH_slot = 8;
				case 22..24: BWH_slot = 2; 
				case 25..27: BWH_slot = 3;
				case 28, 29, 32: BWH_slot = 4;
				case 30, 31: BWH_slot = 5;
				case 33, 34: BWH_slot = 6;
				case 35..38: BWH_slot = 7;
				case 40: BWH_slot = 12;
				case 41..43: BWH_slot = 9;
				case 44..46: BWH_slot = 11;
			}
			
			new BWH_WeaponData_weap_temp, BWH_WeaponData_ammo_temp;
				
			GetPlayerWeaponData(playerid, BWH_slot, BWH_WeaponData_weap_temp, BWH_WeaponData_ammo_temp); // Let's get the current weapon id on that slot.
			
			if(BWH_weaponid == BWH_WeaponData_weap_temp) // We sum the ammo if weapon is the same.
			{				
				if(B_Weapon_Data[playerid][B_Ammo][BWH_WeaponData_weap_temp] > 0) // If ammo in that slot is above 0, we sum it (Take into account that empty weapons remain in weapon data, so this is actually needed).
				{
					BWH_ammo += B_Weapon_Data[playerid][B_Ammo][BWH_WeaponData_weap_temp]; 
				}
			}
			else if(BWH_slot < 6 || BWH_slot > 2) // If slot is 3, 4 or 5 and if it got into this case, that means different weapons can sum the current ammo. We sum the ammo in such case.
			{
				if(B_Weapon_Data[playerid][B_Ammo][BWH_WeaponData_weap_temp] > 0) // If ammo in that slot is above 0, we sum it (Take into account that empty weapons remain in weapon data, so this is actually needed).
				{
					BWH_ammo += B_Weapon_Data[playerid][B_Ammo][BWH_WeaponData_weap_temp]; 
					
					B_Weapon_Data[playerid][B_Weapon][BWH_WeaponData_weap_temp] = false; // Since the weapon id is different (even tho it is the same slot), we remove it.
				}
			}
			
			if(B_Weapon_Data[playerid][B_Ammo][BWH_WeaponData_weap_temp] < BWH_WeaponData_ammo_temp || B_Weapon_Data[playerid][B_Ammo][BWH_WeaponData_weap_temp] > BWH_WeaponData_ammo_temp) // In case the player has hacked ammo, let's try to force it to the value stored. If they NOP it, the bullets still aren't going to be synced, so it won't be any problem.
			{
				SetPlayerAmmo(playerid, BWH_WeaponData_weap_temp, B_Weapon_Data[playerid][B_Ammo][BWH_WeaponData_weap_temp]);
			}
			
			B_Weapon_Data[playerid][B_Weapon][BWH_weaponid] = true;
			B_Weapon_Data[playerid][B_Ammo][BWH_weaponid] = BWH_ammo;
			
			BS_ResetReadPointer(bs);
		}
		case ORPC_SetSpawnInfo: // SetSpawnInfo
		{
			BS_SetReadOffset(bs, 176);
			
			new 
				BWH_SpawnWeapons1, 
				BWH_SpawnWeapons2, 
				BWH_SpawnWeapons3;
				
			// Since the spawn info is going to be set, we must remove the other weapons
			B_Weapon_Data[playerid][B_Spawn_Weapon][0] = 0;
			B_Weapon_Data[playerid][B_Spawn_Weapon][1] = 0;
			B_Weapon_Data[playerid][B_Spawn_Weapon][2] = 0;
			
			BS_ReadValue(
				bs,
				PR_INT32, BWH_SpawnWeapons1,
				PR_INT32, BWH_SpawnWeapons2,
				PR_INT32, BWH_SpawnWeapons3
			);
			
			if(BWH_SpawnWeapons1 > 0)
			{
				B_Weapon_Data[playerid][B_Spawn_Weapon][0] = BWH_SpawnWeapons1;
				BS_ReadInt32(bs, B_Weapon_Data[playerid][B_Spawn_Ammo][0]);
			}
			else
			{
				BS_SetReadOffset(bs, 208);
			}
				
			if(BWH_SpawnWeapons2 > 0)
			{
				B_Weapon_Data[playerid][B_Spawn_Weapon][1] = BWH_SpawnWeapons2;
				BS_ReadInt32(bs, B_Weapon_Data[playerid][B_Spawn_Ammo][1]);
			}
			else
			{
				BS_SetReadOffset(bs, 240);
			}
				
			if(BWH_SpawnWeapons3 > 0)
			{
				B_Weapon_Data[playerid][B_Spawn_Weapon][2] = BWH_SpawnWeapons3;
				BS_ReadInt32(bs, B_Weapon_Data[playerid][B_Spawn_Ammo][2]);
			}
			
			BS_ResetReadPointer(bs);
		}
		case ORPC_RequestClass: // RequestClass
		{
			new BWH_RequestOutcome; // If false, the class id is invalid, so the rest of the bitstream is empty 
			
			BS_ReadValue(
				bs,
				PR_UINT8, BWH_RequestOutcome
			);

			if(BWH_RequestOutcome)
			{
				// Since the spawn info is going to be set, we must remove the other weapons
				B_Weapon_Data[playerid][B_Spawn_Weapon][0] = 0;
				B_Weapon_Data[playerid][B_Spawn_Weapon][1] = 0;
				B_Weapon_Data[playerid][B_Spawn_Weapon][2] = 0;
			
				new 
					BWH_SpawnWeapons1, 
					BWH_SpawnWeapons2, 
					BWH_SpawnWeapons3;
				
				BS_SetReadOffset(bs, 184);
								
				BS_ReadValue(
					bs,
					PR_INT32, BWH_SpawnWeapons1,
					PR_INT32, BWH_SpawnWeapons2,
					PR_INT32, BWH_SpawnWeapons3
				);
				
				if(BWH_SpawnWeapons1 > 0)
				{
					B_Weapon_Data[playerid][B_Spawn_Weapon][0] = BWH_SpawnWeapons1;
					BS_ReadInt32(bs, B_Weapon_Data[playerid][B_Spawn_Ammo][0]);
				}
				else
				{
					BS_SetReadOffset(bs, 216);
				}
				
				if(BWH_SpawnWeapons2 > 0)
				{
					B_Weapon_Data[playerid][B_Spawn_Weapon][1] = BWH_SpawnWeapons2;
					BS_ReadInt32(bs, B_Weapon_Data[playerid][B_Spawn_Ammo][1]);
				}
				else
				{
					BS_SetReadOffset(bs, 248);
				}
				
				if(BWH_SpawnWeapons3 > 0)
				{
					B_Weapon_Data[playerid][B_Spawn_Weapon][2] = BWH_SpawnWeapons3;
					BS_ReadInt32(bs, B_Weapon_Data[playerid][B_Spawn_Ammo][2]);
				}
			}

			BS_ResetReadPointer(bs);
		}
		case ORPC_SetPlayerAmmo: // SetPlayerAmmo
		{
			new BWH_weaponid;
			
			BS_ReadValue(
				bs,
				PR_UINT8, BWH_weaponid,
				PR_UINT16, B_Weapon_Data[playerid][B_Ammo][BWH_weaponid]
			);
						
			BS_ResetReadPointer(bs);
		}
	}
	
    #if defined BWH_OnOutcomingRPC
		return BWH_OnOutcomingRPC(playerid, rpcid, BitStream:bs);
	#else
		return 1;
	#endif
}

/*OnIncomingPacket*/
public OnIncomingPacket(playerid, packetid, BitStream:bs)
{
	switch(packetid)
	{
		case IPacket_OnFootSync:
		{
			new BWH_weaponid;
		
			BS_SetReadOffset(bs, 298);
			
			BS_ReadValue(
				bs,
				PR_BITS, BWH_weaponid, 6
			);
			
			BS_ResetReadPointer(bs);
			
			if(!B_Weapon_Data[playerid][B_Weapon][BWH_weaponid])
			{
				BS_SetWriteOffset(bs, 298);
				
				BS_WriteValue(
					bs,
					PR_BITS, 0, 6
				);
				
				BS_ResetWritePointer(bs); 
			}
		}
		case IPacket_PassengerSync:
		{
			new BWH_weaponid;
		
			BS_SetReadOffset(bs, 34);
			
			BS_ReadValue(
				bs,
				PR_BITS, BWH_weaponid, 6
			);
			
			BS_ResetReadPointer(bs);
			
			if(!B_Weapon_Data[playerid][B_Weapon][BWH_weaponid])
			{
				BS_SetWriteOffset(bs, 34);
				
				BS_WriteValue(
					bs,
					PR_BITS, 0, 6
				);
				
				BS_ResetWritePointer(bs); 
			}
		}
		case IPacket_DriverSync:
		{
			new BWH_weaponid;
		
			BS_SetReadOffset(bs, 442);
			
			BS_ReadValue(
				bs,
				PR_BITS, BWH_weaponid, 6
			);
			
			BS_ResetReadPointer(bs);
			
			if(!B_Weapon_Data[playerid][B_Weapon][BWH_weaponid])
			{
				BS_SetWriteOffset(bs, 442);
				
				BS_WriteValue(
					bs,
					PR_BITS, 0, 6
				);
				
				BS_ResetWritePointer(bs); 
			}
		}
	}
	
    #if defined BWH_OnIncomingPacket
        return BWH_OnIncomingPacket(playerid, packetid, bs);
    #else
        return 1;
    #endif
}

/*Internal*/
static BWH_memset(aArray[], iValue, iSize = sizeof(aArray))  // memset - Slice
{
	new iAddress;
	
	#emit LOAD.S.pri 12
	#emit STOR.S.pri iAddress
	
	iSize *= 4;
	
	while (iSize > 0) 
	{
		if (iSize >= 4096) 
		{
			#emit LOAD.S.alt iAddress
			#emit LOAD.S.pri iValue
			#emit FILL 4096
		
			iSize    -= 4096;
			iAddress += 4096;
		} 
		else if (iSize >= 1024) 
		{
			#emit LOAD.S.alt iAddress
			#emit LOAD.S.pri iValue
			#emit FILL 1024

			iSize    -= 1024;
			iAddress += 1024;
		} 
		else if (iSize >= 256) 
		{
			#emit LOAD.S.alt iAddress
			#emit LOAD.S.pri iValue
			#emit FILL 256

			iSize    -= 256;
			iAddress += 256;
		} 
		else if (iSize >= 64) 
		{
			#emit LOAD.S.alt iAddress
			#emit LOAD.S.pri iValue
			#emit FILL 64

			iSize    -= 64;
			iAddress += 64;
		} 
		else if (iSize >= 16) 
		{
			#emit LOAD.S.alt iAddress
			#emit LOAD.S.pri iValue
			#emit FILL 16

			iSize    -= 16;
			iAddress += 16;
		} 
		else 
		{
			#emit LOAD.S.alt iAddress
			#emit LOAD.S.pri iValue
			#emit FILL 4

			iSize    -= 4;
			iAddress += 4;
		}
	}
	
	#pragma unused aArray
}

/*Hooks*/
#if defined _ALS_OnOutcomingRPC
#undef OnOutcomingRPC
#else
#define _ALS_OnOutcomingRPC
#endif
#define OnOutcomingRPC BWH_OnOutcomingRPC

#if defined BWH_OnOutcomingRPC
forward BWH_OnOutcomingRPC(playerid, rpcid, BitStream:bs);
#endif

#if defined _ALS_OnIncomingPacket
#undef OnIncomingPacket
#else
#define _ALS_OnIncomingPacket
#endif
#define OnIncomingPacket BWH_OnIncomingPacket

#if defined BWH_OnIncomingPacket
forward BWH_OnIncomingPacket(playerid, packetid, BitStream:bs);
#endif

#if defined _ALS_OnPlayerConnect
#undef OnPlayerConnect
#else
#define _ALS_OnPlayerConnect
#endif
#define OnPlayerConnect BWH_OnPlayerConnect

#if defined BWH_OnPlayerConnect
forward BWH_OnPlayerConnect(playerid);
#endif

#if defined _ALS_OnPlayerSpawn
#undef OnPlayerSpawn
#else
#define _ALS_OnPlayerSpawn
#endif
#define OnPlayerSpawn BWH_OnPlayerSpawn

#if defined BWH_OnPlayerSpawn
forward BWH_OnPlayerSpawn(playerid);
#endif

#if defined _ALS_OnPlayerWeaponShot
#undef OnPlayerWeaponShot
#else
#define _ALS_OnPlayerWeaponShot
#endif
#define OnPlayerWeaponShot BWH_OnPlayerWeaponShot

#if defined BWH_OnPlayerWeaponShot
forward BWH_OnPlayerWeaponShot(playerid, weaponid, hittype, hitid, Float:fX, Float:fY, Float:fZ);
#endif