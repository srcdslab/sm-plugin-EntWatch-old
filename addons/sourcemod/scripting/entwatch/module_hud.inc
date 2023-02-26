#define EW_MODULE_HUD

#define MAXSHOWHUDITEMS 10
#define MAXSIZESTRINGFORITEM 40
#define TIMEROTORHUD 5

#include <bighud>

//-------------------------------------------------------
// Purpose: Plugin settings
//-------------------------------------------------------
ConVar	g_hCvar_DisplayEnabled,
		g_hCvar_DisplayCooldowns,
		g_hCvar_Admins_See,
		g_hCvar_HUD_Channel,
		g_hCvar_ZM_pry,
		g_hCvar_DefaultHudType,
		g_hCvar_MaxHudItemsCount,
		g_hCvar_HudRotorDefault;
//-------------------------------------------------------
// Purpose: Client settings
//-------------------------------------------------------
Handle	g_hCookie_Display		  = null,
		g_hCookie_HudType         = null,
		g_hCookie_HudColor		  = null,
		g_hCookie_HudPos		  = null,
		g_hCookie_HudName		  = null,
		g_hCookie_HudItemsCount   = null,
		g_hCookie_HudRotorTime    = null;
//-------------------------------------------------------
// Purpose: Plugin Local settings
//-------------------------------------------------------
bool	g_bDispEnabled = true,
		g_bDispCooldowns = true,
		g_bAdminsSee = true,
		g_bZombieNoItemPry = false;

int		g_iHUDChannel = 5;
int		g_iDefaultRotorTime = 3;
int		g_iMaxItemsCount = 10;

int		g_iRotation[MAXPLAYERS+1][3][2]; // [client][0] - Humans, [client][1] - Zombies, [client][2] - Admins, [x][0] - count Items, [client][x][1] - current # list
ArrayList	g_iHItemsArray;
ArrayList	g_iZMItemsArray;

class_ClientSettings_Hud g_CSettings_Hud[MAXPLAYERS+1];

stock void EWM_Hud_OnPluginStart()
{
	g_iHItemsArray = new ArrayList(32);
	g_iZMItemsArray = new ArrayList(32);
	g_hCvar_DisplayEnabled		= CreateConVar("entwatch_display_enable", "1", "Enable/Disable the display.", _, true, 0.0, true, 1.0);
	g_hCvar_DisplayCooldowns	= CreateConVar("entwatch_display_cooldowns", "1", "Show/Hide the cooldowns on the display.", _, true, 0.0, true, 1.0);
	g_hCvar_Admins_See			= CreateConVar("entwatch_admins_see", "1", "Enable/Disable admins see everything Items", _, true, 0.0, true, 1.0);
	g_hCvar_HUD_Channel			= CreateConVar("entwatch_hud_channel", "5", "Change HUD Channel/Group Dynamic channel.", _, true, 0.0, true, 5.0);
	g_hCvar_ZM_pry				= CreateConVar("entwatch_zm_noitem_pry", "0", "Enable/Disable zm pry human Items if zms without items", _, true, 0.0, true, 1.0);
	g_hCvar_DefaultHudType		= CreateConVar("entwatch_hud_type_default", "0", "Default type of hud (0 = KeyHintText, 1 = HUD).", _, true, 0.0, true, 10.0);
	g_hCvar_MaxHudItemsCount	= CreateConVar("entwatch_hud_items_count", "10", "Max/Default Items Count in hud to display.", _, true, 1.0, true, 10.0);
	g_hCvar_HudRotorDefault		= CreateConVar("entwatch_hud_rotor_time_default", "3", "Default Rotor time in seconds for hud.", _, true, 1.0, true, 5.0);
	
	EWM_Hud_GetConVars();

	RegConsoleCmd("sm_hud", EWM_Hud_Command_ToggleHUD);
	RegConsoleCmd("sm_hudtype", EWM_Hud_Command_ToggleHUDType);
	RegConsoleCmd("sm_hudname", EWM_Hud_Command_ToggleHUDName);
	RegConsoleCmd("sm_hudpos", EWM_Hud_Command_Hudpos);
	RegConsoleCmd("sm_hudcolor", EWM_Hud_Command_HudColor);
	RegConsoleCmd("sm_huditems", EWM_Hud_Command_HudItems);
	RegConsoleCmd("sm_hudtime", EWM_Hud_Command_HudRotor);

	HookConVarChange(g_hCvar_DisplayEnabled, Cvar_HUD_Changed);
	HookConVarChange(g_hCvar_DisplayCooldowns, Cvar_HUD_Changed);
	HookConVarChange(g_hCvar_Admins_See, Cvar_HUD_Changed);
	HookConVarChange(g_hCvar_HUD_Channel, Cvar_HUD_Changed);
	HookConVarChange(g_hCvar_ZM_pry, Cvar_HUD_Changed);

	g_hCookie_Display     = RegClientCookie("entwatch_display", "", CookieAccess_Private);
	g_hCookie_HudType     = RegClientCookie("entwatch_hudtype", "", CookieAccess_Private);
	g_hCookie_HudName     = RegClientCookie("entwatch_hudname", "", CookieAccess_Private);
	g_hCookie_HudColor    = RegClientCookie("entwatch_hudcolor_rgba", "", CookieAccess_Private);
	g_hCookie_HudPos      = RegClientCookie("entwatch_hudpos", "", CookieAccess_Private);
	g_hCookie_HudItemsCount	= RegClientCookie("entwatch_huditems_count", "", CookieAccess_Private);
	g_hCookie_HudRotorTime	= RegClientCookie("entwatch_hudrotor_time", "", CookieAccess_Private);

	CreateTimer(1.0, EWM_Hud_Timer_DisplayHUD, _, TIMER_REPEAT);
}

stock void EWM_Hud_OnConfigsExecuted()
{
	EWM_Hud_GetConVars();
}

stock void EWM_Hud_GetConVars()
{
	g_bDispEnabled = GetConVarBool(g_hCvar_DisplayEnabled);
	g_bDispCooldowns = GetConVarBool(g_hCvar_DisplayCooldowns);
	g_bAdminsSee = GetConVarBool(g_hCvar_Admins_See);
	g_iHUDChannel = GetConVarInt(g_hCvar_HUD_Channel);
	g_bZombieNoItemPry = GetConVarBool(g_hCvar_ZM_pry);
	g_iMaxItemsCount = GetConVarInt(g_hCvar_MaxHudItemsCount);
	g_iDefaultRotorTime = GetConVarInt(g_hCvar_HudRotorDefault);

	// Prevent this kind of issue: https://github.com/srcdslab/sm-plugin-EntWatch/issues/1#issuecomment-1445196178
	if (g_iMaxItemsCount > MAXSHOWHUDITEMS)
		g_iMaxItemsCount = MAXSHOWHUDITEMS;

	if (g_iDefaultRotorTime > TIMEROTORHUD)
		g_iDefaultRotorTime = TIMEROTORHUD;

}

public void Cvar_HUD_Changed(ConVar convar, const char[] oldValue, const char[] newValue)
{
	EWM_Hud_GetConVars();
}

stock void EWM_Hud_OnClientDisconnect(int iClient)
{
	g_iRotation[iClient][0][0] = 0;
	g_iRotation[iClient][0][1] = 0;
	
	g_iRotation[iClient][1][0] = 0;
	g_iRotation[iClient][1][1] = 0;
	
	g_iRotation[iClient][2][0] = 0;
	g_iRotation[iClient][2][1] = 0;
}

stock void EWM_Hud_OnClientCookiesCached(int iClient)
{
	char sBuffer_cookie[32];
	
	// Display
	GetClientCookie(iClient, g_hCookie_Display, sBuffer_cookie, sizeof(sBuffer_cookie));
	if (StrEqual(sBuffer_cookie,""))
	{
		SetClientCookie(iClient, g_hCookie_Display, "1");
		strcopy(sBuffer_cookie, sizeof(sBuffer_cookie), "1");
	}
	g_CSettings_Hud[iClient].Display = view_as<bool>(StringToInt(sBuffer_cookie));

	//Hud Type
	GetClientCookie(iClient, g_hCookie_HudType, sBuffer_cookie, sizeof(sBuffer_cookie));
	if (StrEqual(sBuffer_cookie,""))
	{
		char defaultValue[32];
		g_hCvar_DefaultHudType.GetString(defaultValue, sizeof(defaultValue));
		SetClientCookie(iClient, g_hCookie_HudType, defaultValue);
		strcopy(sBuffer_cookie, sizeof(sBuffer_cookie), defaultValue);
	}
	g_CSettings_Hud[iClient].Type = StringToInt(sBuffer_cookie);

	//Hud Names
	GetClientCookie(iClient, g_hCookie_HudName, sBuffer_cookie, sizeof(sBuffer_cookie));
	if (StrEqual(sBuffer_cookie,""))
	{
		SetClientCookie(iClient, g_hCookie_HudName, "1");
		strcopy(sBuffer_cookie, sizeof(sBuffer_cookie), "1");
	}
	g_CSettings_Hud[iClient].Name = view_as<bool>(StringToInt(sBuffer_cookie));
	
	// Colors RBGA
	GetClientCookie(iClient, g_hCookie_HudColor, sBuffer_cookie, sizeof(sBuffer_cookie));
	if (StrEqual(sBuffer_cookie,""))
	{
		Format(sBuffer_cookie, sizeof(sBuffer_cookie), "%i/%i/%i/%i", g_SchemeConfig.Color_HUD[0], g_SchemeConfig.Color_HUD[1], g_SchemeConfig.Color_HUD[2], g_SchemeConfig.Color_HUD[3]);
		SetClientCookie(iClient, g_hCookie_HudColor, sBuffer_cookie);
		g_CSettings_Hud[iClient].Color[0] = g_SchemeConfig.Color_HUD[0];
		g_CSettings_Hud[iClient].Color[1] = g_SchemeConfig.Color_HUD[1];
		g_CSettings_Hud[iClient].Color[2] = g_SchemeConfig.Color_HUD[2];
		g_CSettings_Hud[iClient].Color[3] = g_SchemeConfig.Color_HUD[3];
	}else 
	{
		char Explode_HudColor[4][32];
		ExplodeString(sBuffer_cookie, "/", Explode_HudColor, 4, 32);
		
		int Color_validate = StringToInt(Explode_HudColor[0]);
		if(Color_validate >= 0 && Color_validate <= 255) g_CSettings_Hud[iClient].Color[0] = Color_validate;
			else g_CSettings_Hud[iClient].Color[0] = g_SchemeConfig.Color_HUD[0];
			
		Color_validate = StringToInt(Explode_HudColor[1]);
		if(Color_validate >= 0 && Color_validate <= 255) g_CSettings_Hud[iClient].Color[1] = Color_validate;
			else g_CSettings_Hud[iClient].Color[1] = g_SchemeConfig.Color_HUD[1];
		
		Color_validate = StringToInt(Explode_HudColor[2]);
		if(Color_validate >= 0 && Color_validate <= 255) g_CSettings_Hud[iClient].Color[2] = Color_validate;
			else g_CSettings_Hud[iClient].Color[2] = g_SchemeConfig.Color_HUD[2];
			
		Color_validate = StringToInt(Explode_HudColor[3]);
		if(Color_validate >= 0 && Color_validate <= 255) g_CSettings_Hud[iClient].Color[3] = Color_validate;
			else g_CSettings_Hud[iClient].Color[3] = g_SchemeConfig.Color_HUD[3];
	}

	// Position
	GetClientCookie(iClient, g_hCookie_HudPos, sBuffer_cookie, sizeof(sBuffer_cookie));
	if (StrEqual(sBuffer_cookie,""))
	{
		FormatEx(sBuffer_cookie, sizeof(sBuffer_cookie), "%f/%f", g_SchemeConfig.Pos_HUD_X, g_SchemeConfig.Pos_HUD_Y);
		SetClientCookie(iClient, g_hCookie_HudPos, sBuffer_cookie);
		g_CSettings_Hud[iClient].Pos_X = g_SchemeConfig.Pos_HUD_X;
		g_CSettings_Hud[iClient].Pos_Y = g_SchemeConfig.Pos_HUD_Y;
	}else 
	{
		char Explode_HudPosition[2][32];
		ExplodeString(sBuffer_cookie, "/", Explode_HudPosition, 2, 32);
		
		float Pos_validate = StringToFloat(Explode_HudPosition[0]);
		if((Pos_validate >= 0.0 && Pos_validate <= 1.0) || Pos_validate == -1.0) g_CSettings_Hud[iClient].Pos_X = Pos_validate;
			else g_CSettings_Hud[iClient].Pos_X = g_SchemeConfig.Pos_HUD_X;
			
		Pos_validate = StringToFloat(Explode_HudPosition[1]);
		if((Pos_validate >= 0.0 && Pos_validate <= 1.0) || Pos_validate == -1.0) g_CSettings_Hud[iClient].Pos_Y = Pos_validate;
			else g_CSettings_Hud[iClient].Pos_Y = g_SchemeConfig.Pos_HUD_Y;
	}

	// Items count
	GetClientCookie(iClient, g_hCookie_HudItemsCount, sBuffer_cookie, sizeof(sBuffer_cookie));
	if (StrEqual(sBuffer_cookie,"")) 
	{
		FormatEx(sBuffer_cookie, sizeof(sBuffer_cookie), "%d", g_iMaxItemsCount);
		SetClientCookie(iClient, g_hCookie_HudItemsCount, sBuffer_cookie);
		g_CSettings_Hud[iClient].ItemsCount = g_iMaxItemsCount;
	}else
	{
		int count = StringToInt(sBuffer_cookie);
		g_CSettings_Hud[iClient].ItemsCount = count;
	}
	
	// rotor time
	GetClientCookie(iClient, g_hCookie_HudRotorTime, sBuffer_cookie, sizeof(sBuffer_cookie));
	if (StrEqual(sBuffer_cookie,"")) 
	{
		FormatEx(sBuffer_cookie, sizeof(sBuffer_cookie), "%d", g_iDefaultRotorTime);
		SetClientCookie(iClient, g_hCookie_HudRotorTime, sBuffer_cookie);
		g_CSettings_Hud[iClient].RotorTime = g_iDefaultRotorTime;
	}else
	{
		int time = StringToInt(sBuffer_cookie);
		g_CSettings_Hud[iClient].RotorTime = time;
	}
}

stock void EWM_Hud_LoadDefaultClientSettings(int iClient)
{
	g_CSettings_Hud[iClient].Display = true;
	g_CSettings_Hud[iClient].Color[0] = g_SchemeConfig.Color_HUD[0];
	g_CSettings_Hud[iClient].Color[1] = g_SchemeConfig.Color_HUD[1];
	g_CSettings_Hud[iClient].Color[2] = g_SchemeConfig.Color_HUD[2];
	g_CSettings_Hud[iClient].Color[3] = g_SchemeConfig.Color_HUD[3];
	g_CSettings_Hud[iClient].Pos_X = g_SchemeConfig.Pos_HUD_X;
	g_CSettings_Hud[iClient].Pos_Y = g_SchemeConfig.Pos_HUD_Y;
	g_CSettings_Hud[iClient].ItemsCount = g_iMaxItemsCount;
	g_CSettings_Hud[iClient].RotorTime = g_iDefaultRotorTime;
}

public void EWM_Hud_OnMapStart()
{
	g_iHItemsArray.Clear();
	g_iZMItemsArray.Clear();
	for(int j = 1; j <= MaxClients; j++)
	{
		if(!IsClientInGame(j) || IsClientSourceTV(j)) continue;
		
		for(int i = 0; i < 3; i++)
		{
			g_iRotation[j][i][0] = 0;
			g_iRotation[j][i][1] = 0;
		}
	}
}

public void EWM_Hud_DisplayCustomHUD(int client, int iHUDChannel, const char[] msg)
{
#if defined _DynamicChannels_included_
	iHUDChannel = GetDynamicChannel(iHUDChannel);
#endif

	if (g_CSettings_Hud[client].Type == 0)
	{
		Handle hBuffer = StartMessageOne("KeyHintText", client);
		if (CanTestFeatures() && GetFeatureStatus(FeatureType_Native, "GetUserMessageType") == FeatureStatus_Available && GetUserMessageType() == UM_Protobuf)
		{
			PbAddString(hBuffer, "hints", msg);
		}
		else
		{
			BfWriteByte(hBuffer, 1);
			BfWriteString(hBuffer, msg);
		}
		EndMessage();
	}
	else if (g_CSettings_Hud[client].Type == 1)
	{
		switch (g_evGameEngine)
		{
			case Engine_CSGO:
			{
				BigHud_ShowHudText(client, iHUDChannel, msg);
			}
			default:
			{
				ShowHudText(client, iHUDChannel, msg);
			}
		}
	}
}

public Action EWM_Hud_Timer_DisplayHUD(Handle timer, int client)
{
	static int iRotorTime[MAXPLAYERS + 1];
	
	if (g_bDispEnabled && g_bConfigLoaded)
	{
		g_iHItemsArray.Clear();
		g_iZMItemsArray.Clear();
		for(int i = 0; i<g_ItemList.Length; i++)
		{
			class_ItemList ItemTest;
			g_ItemList.GetArray(i, ItemTest, sizeof(ItemTest));
			if(ItemTest.Hud && ItemTest.OwnerID != INVALID_ENT_REFERENCE && IsValidEdict(ItemTest.WeaponID))
			{
				if(GetClientTeam(ItemTest.OwnerID)==3) g_iHItemsArray.Push(i);
				else if(GetClientTeam(ItemTest.OwnerID)==2) g_iZMItemsArray.Push(i);
			}
		}
		
		int iCountHItems = g_iHItemsArray.Length;
		int iCountZMItems = g_iZMItemsArray.Length;
		int iCountAItems = iCountHItems + iCountZMItems;
		
		if(iCountHItems > 0 || iCountZMItems > 0)
		{
			for(int i = 1; i <= MaxClients; i++)
			{
				//if(IsClientInGame(i) && !IsFakeClient(i) && g_CSettings_Hud[i].Display && (!IsVoteInProgress() || !IsClientInVotePool(i)) && GetClientMenu(i) == MenuSource_None)
				if(IsClientInGame(i) && !IsFakeClient(i) && g_CSettings_Hud[i].Display && GetClientMenu(i) == MenuSource_None)
				{
					bool bRotor = false;
					iRotorTime[i]++;
					if(iRotorTime[i]>=g_CSettings_Hud[i].RotorTime)
					{
						iRotorTime[i] = 0;
						bRotor = true;
					}
					
					int iMaxHList = RoundToCeil(float(iCountHItems)/float(g_CSettings_Hud[i].ItemsCount));
					int iMaxZMList = RoundToCeil(float(iCountZMItems)/float(g_CSettings_Hud[i].ItemsCount));
					int iMaxAList = RoundToCeil(float(iCountAItems)/float(g_CSettings_Hud[i].ItemsCount));
					
					if(iCountHItems != g_iRotation[i][0][0])
					{
						g_iRotation[i][0][0] = iCountHItems;
						g_iRotation[i][0][1] = 0;
					}else
					{
						if(bRotor)
						{
							g_iRotation[i][0][1] += 1;
							if(g_iRotation[i][0][1] > iMaxHList-1) g_iRotation[i][0][1] = 0;
						}
					}
					if(iCountZMItems != g_iRotation[i][1][0])
					{
						g_iRotation[i][1][0] = iCountZMItems;
						g_iRotation[i][1][1] = 0;
					}else
					{
						if(bRotor)
						{
							g_iRotation[i][1][1] += 1;
							if(g_iRotation[i][1][1] > iMaxZMList-1) g_iRotation[i][1][1] = 0;
						}
					}
					if(iCountAItems != g_iRotation[i][2][0])
					{
						g_iRotation[i][2][0] = iCountAItems;
						g_iRotation[i][2][1] = 0;
					}else
					{
						if(bRotor)
						{
							g_iRotation[i][2][1] += 1;
							if(g_iRotation[i][2][1] > iMaxAList-1) g_iRotation[i][2][1] = 0;
						}
					}
					
					char sMes_Humans[250];
					char sMes_Humans_woname[250];
					char sMes_Zombies[250];
					char sMes_Zombies_woname[250];
					char sMes_Admins[250];
					char sMes_Admins_woname[250];
					EWM_Hud_Construct_String(i, sMes_Humans, sizeof(sMes_Humans), sMes_Humans_woname, sizeof(sMes_Humans_woname), iCountHItems, iMaxHList, 0);
					EWM_Hud_Construct_String(i, sMes_Zombies, sizeof(sMes_Zombies), sMes_Zombies_woname, sizeof(sMes_Zombies_woname), iCountZMItems, iMaxZMList, 1);
					EWM_Hud_Construct_String(i, sMes_Admins, sizeof(sMes_Admins), sMes_Admins_woname, sizeof(sMes_Admins_woname), iCountAItems, iMaxAList, 2, iCountHItems);
					
					switch (g_evGameEngine)
					{
						case Engine_CSGO:
						{
							BigHud_SetHudTextParams(g_CSettings_Hud[i].Pos_X, g_CSettings_Hud[i].Pos_Y, 1.1, g_CSettings_Hud[i].Color[0], g_CSettings_Hud[i].Color[1], g_CSettings_Hud[i].Color[2], g_CSettings_Hud[i].Color[3], 0, 0.0, 0.0, 0.0);
						}
						default:
						{
							SetHudTextParams(g_CSettings_Hud[i].Pos_X, g_CSettings_Hud[i].Pos_Y, 1.1, g_CSettings_Hud[i].Color[0], g_CSettings_Hud[i].Color[1], g_CSettings_Hud[i].Color[2], g_CSettings_Hud[i].Color[3], 0, 0.0, 0.0, 0.0);
						}
					}

					if(g_bAdminsSee && g_bIsAdmin[i])
					{
						if(g_CSettings_Hud[i].Name)
						{
							EWM_Hud_DisplayCustomHUD(i, g_iHUDChannel, sMes_Admins);
						}else
						{
							EWM_Hud_DisplayCustomHUD(i, g_iHUDChannel, sMes_Admins_woname);
						}
					}else if((g_bTeamOnly && GetClientTeam(i)==3) || (g_bZombieNoItemPry && iCountZMItems<=0 && GetClientTeam(i)==2))
					{
						if(iCountHItems > 0)
						{
							if(g_CSettings_Hud[i].Name)
							{
								EWM_Hud_DisplayCustomHUD(i, g_iHUDChannel, sMes_Humans);
							}else
							{
								EWM_Hud_DisplayCustomHUD(i, g_iHUDChannel, sMes_Humans_woname);
							}
						}
					}else if(g_bTeamOnly && GetClientTeam(i)==2)
					{
						if(iCountZMItems > 0)
						{
							if(g_CSettings_Hud[i].Name)
							{
								EWM_Hud_DisplayCustomHUD(i, g_iHUDChannel, sMes_Zombies);
							}else
							{
								EWM_Hud_DisplayCustomHUD(i, g_iHUDChannel, sMes_Zombies_woname);
							}
						}
					}else
					{
						if(g_CSettings_Hud[i].Name)
						{
							EWM_Hud_DisplayCustomHUD(i, g_iHUDChannel, sMes_Admins);
						}else
						{
							EWM_Hud_DisplayCustomHUD(i, g_iHUDChannel, sMes_Admins_woname);
						}
					}
				}else
				{
					iRotorTime[i] = 0;
				}
			}
		}
	}
	return Plugin_Continue;
}

void EWM_Hud_Construct_String(int iClient, char[] sResult, int iSizeResult, char[] sResult_woname, int iSizeResult_woname, int iCountItems, int iMaxList, int iType, int iCountHItems = 0)
{
	if(iCountItems > 0)
	{
		int count = g_CSettings_Hud[iClient].ItemsCount;
		char[][] sBuf = new char[count][MAXSIZESTRINGFORITEM];
		char[][] sBuf_woname = new char[count][MAXSIZESTRINGFORITEM];
		
		int iLastIndex = (g_iRotation[iClient][iType][1]+1) * count;
		if(iLastIndex > iCountItems) iLastIndex = iCountItems;
		int iIndex = -1;
		for(int i = g_iRotation[iClient][iType][1]*count; i < iLastIndex; i++)
		{
			iIndex++;
			class_ItemList ItemTest;
			int iFixIndex = 0;
			if(iType==0) iFixIndex = g_iHItemsArray.Get(i);
			else if(iType==1) iFixIndex = g_iZMItemsArray.Get(i);
			else
			{
				if(i < iCountHItems) iFixIndex = g_iHItemsArray.Get(i);
				else iFixIndex = g_iZMItemsArray.Get(i-iCountHItems); 
			}
			g_ItemList.GetArray(iFixIndex, ItemTest, sizeof(ItemTest));
			UpdateTime();
			if(ItemTest.CheckDelay() > 0.0)
			{
				FormatEx(sBuf_woname[iIndex], MAXSIZESTRINGFORITEM, "%s[-%d]", ItemTest.ShortName, ItemTest.RoundDelay());
				FormatEx(sBuf[iIndex], MAXSIZESTRINGFORITEM, "%s: %N", sBuf_woname[iIndex], ItemTest.OwnerID);
			}
			else
			{
				if(g_bDispCooldowns)
				{
					if(ItemTest.LockButton) FormatEx(sBuf_woname[iIndex], MAXSIZESTRINGFORITEM, "%s[L]", ItemTest.ShortName);
					//else if(ItemTest.CheckWaitTime() > 0) FormatEx(sBuf_woname[iIndex], MAXSIZESTRINGFORITEM, "%s[%d]", ItemTest.ShortName, ItemTest.RoundWaitTime());
					else
					{
						switch(ItemTest.Mode)
						{
							case 2:
							{
								if (ItemTest.CheckCoolDown() > 0) FormatEx(sBuf_woname[iIndex], MAXSIZESTRINGFORITEM, "%s[%d]", ItemTest.ShortName, ItemTest.RoundCoolDown());
								else FormatEx(sBuf_woname[iIndex], MAXSIZESTRINGFORITEM, "%s[R]", ItemTest.ShortName);
							}
							case 3:
							{
								if (ItemTest.Uses < ItemTest.MaxUses) FormatEx(sBuf_woname[iIndex], MAXSIZESTRINGFORITEM, "%s[%d/%d]", ItemTest.ShortName, ItemTest.Uses, ItemTest.MaxUses);
								else FormatEx(sBuf_woname[iIndex], MAXSIZESTRINGFORITEM, "%s[E]", ItemTest.ShortName);
							}
							case 4:
							{
								if (ItemTest.CheckCoolDown() > 0) FormatEx(sBuf_woname[iIndex], MAXSIZESTRINGFORITEM, "%s[%d]", ItemTest.ShortName, ItemTest.RoundCoolDown());
								else
									if (ItemTest.Uses < ItemTest.MaxUses) FormatEx(sBuf_woname[iIndex], MAXSIZESTRINGFORITEM, "%s[%d/%d]", ItemTest.ShortName, ItemTest.Uses, ItemTest.MaxUses);
									else FormatEx(sBuf_woname[iIndex], MAXSIZESTRINGFORITEM, "%s[E]", ItemTest.ShortName);
							}
							case 5:
							{
								if (ItemTest.CheckCoolDown() > 0) FormatEx(sBuf_woname[iIndex], MAXSIZESTRINGFORITEM, "%s[%d]", ItemTest.ShortName, ItemTest.RoundCoolDown());
								else FormatEx(sBuf_woname[iIndex], MAXSIZESTRINGFORITEM, "%s[%d/%d]", ItemTest.ShortName, ItemTest.Uses, ItemTest.MaxUses);
							}
							case 6, 7:
							{
								if (ItemTest.MathValue > 0)
								{
									if(ItemTest.CoolDown > 0 && ItemTest.CheckCoolDown() > 0) FormatEx(sBuf_woname[iIndex], MAXSIZESTRINGFORITEM, "%s[%d]", ItemTest.ShortName, ItemTest.RoundCoolDown());
									else FormatEx(sBuf_woname[iIndex], MAXSIZESTRINGFORITEM, "%s[%d/%d]", ItemTest.ShortName, ItemTest.MathValue, ItemTest.MathValueMax);
								}
								else FormatEx(sBuf_woname[iIndex], MAXSIZESTRINGFORITEM, "%s[E]", ItemTest.ShortName);
							}
							default: FormatEx(sBuf_woname[iIndex], MAXSIZESTRINGFORITEM, "%s[+]", ItemTest.ShortName);
						}
					}
					if(ItemTest.LockButton2) Format(sBuf_woname[iIndex], MAXSIZESTRINGFORITEM, "%s[L]", sBuf_woname[iIndex]);
					else if(ItemTest.CheckWaitTime2() > 0) Format(sBuf_woname[iIndex], MAXSIZESTRINGFORITEM, "%s[%d]", sBuf_woname[iIndex], ItemTest.RoundWaitTime2());
					else
					{
						switch(ItemTest.Mode2)
						{
							case 2:
							{
								if (ItemTest.CheckCoolDown2() > 0) Format(sBuf_woname[iIndex], MAXSIZESTRINGFORITEM, "%s[%d]", sBuf_woname[iIndex], ItemTest.RoundCoolDown2());
								else Format(sBuf_woname[iIndex], MAXSIZESTRINGFORITEM, "%s[R]", sBuf_woname[iIndex]);
							}
							case 3:
							{
								if (ItemTest.Uses2 < ItemTest.MaxUses2) Format(sBuf_woname[iIndex], MAXSIZESTRINGFORITEM, "%s[%d/%d]", sBuf_woname[iIndex], ItemTest.Uses2, ItemTest.MaxUses2);
								else Format(sBuf_woname[iIndex], MAXSIZESTRINGFORITEM, "%s[E]", sBuf_woname[iIndex]);
							}
							case 4:
							{
								if (ItemTest.CheckCoolDown2() > 0) Format(sBuf_woname[iIndex], MAXSIZESTRINGFORITEM, "%s[%d]", sBuf_woname[iIndex], ItemTest.RoundCoolDown2());
								else
									if (ItemTest.Uses < ItemTest.MaxUses) Format(sBuf_woname[iIndex], MAXSIZESTRINGFORITEM, "%s[%d/%d]", sBuf_woname[iIndex], ItemTest.Uses2, ItemTest.MaxUses2);
									else Format(sBuf_woname[iIndex], MAXSIZESTRINGFORITEM, "%s[E]", sBuf_woname[iIndex]);
							}
							case 5:
							{
								if (ItemTest.CheckCoolDown2() > 0) Format(sBuf_woname[iIndex], MAXSIZESTRINGFORITEM, "%s[%d]", sBuf_woname[iIndex], ItemTest.RoundCoolDown2());
								else Format(sBuf_woname[iIndex], MAXSIZESTRINGFORITEM, "%s[%d/%d]", sBuf_woname[iIndex], ItemTest.Uses2, ItemTest.MaxUses2);
							}
							case 6, 7:
							{
								if (ItemTest.MathValue2 > 0)
								{
									if(ItemTest.CoolDown2 > 0 && ItemTest.CheckCoolDown2() > 0) Format(sBuf_woname[iIndex], MAXSIZESTRINGFORITEM, "%s[%d]", sBuf_woname[iIndex], ItemTest.RoundCoolDown2());
									else Format(sBuf_woname[iIndex], MAXSIZESTRINGFORITEM, "%s[%d/%d]", sBuf_woname[iIndex], ItemTest.MathValue2, ItemTest.MathValueMax2);
								}
								else Format(sBuf_woname[iIndex], MAXSIZESTRINGFORITEM, "%s[E]", sBuf_woname[iIndex]);
							}
					}
					}
					FormatEx(sBuf[iIndex], MAXSIZESTRINGFORITEM, "%s: %N", sBuf_woname[iIndex], ItemTest.OwnerID);
				}
				else
				{
					FormatEx(sBuf[iIndex], MAXSIZESTRINGFORITEM, "%s: %N", ItemTest.ShortName, ItemTest.OwnerID);
					FormatEx(sBuf_woname[iIndex], MAXSIZESTRINGFORITEM, "%s picked up", ItemTest.ShortName);
				}
			}
		}
		for(int i=0; i <= iIndex; i++)
		{
			StrCat(sResult, iSizeResult, sBuf[i]);
			StrCat(sResult_woname, iSizeResult_woname, sBuf_woname[i]);
			if(i != iIndex)
			{
				StrCat(sResult, iSizeResult, "\n");
				StrCat(sResult_woname, iSizeResult_woname, "\n");
			}else
			{
				if(iMaxList>1)
				{
					char sCurrentList[MAXSIZESTRINGFORITEM];
					FormatEx(sCurrentList, sizeof(sCurrentList), "\n\nList: %i/%i", g_iRotation[iClient][iType][1]+1, iMaxList);
					StrCat(sResult, iSizeResult , sCurrentList);
					StrCat(sResult_woname, iSizeResult_woname, sCurrentList);
				}
			}
		}
	}
}

public Action EWM_Hud_Command_ToggleHUD(int iClient, int iArgs)
{
	if((IsClientConnected(iClient) && IsClientInGame(iClient)))
	{
		if (g_CSettings_Hud[iClient].Display)
		{
			CPrintToChat(iClient, "%s%t %s%t %s%t", g_SchemeConfig.Color_Tag, "EW_Tag", g_SchemeConfig.Color_Warning, "Hud", g_SchemeConfig.Color_Disabled, "Disabled");
			g_CSettings_Hud[iClient].Display = false;
			SetClientCookie(iClient, g_hCookie_Display, "0");
		}
		else
		{
			CPrintToChat(iClient, "%s%t %s%t %s%t", g_SchemeConfig.Color_Tag, "EW_Tag", g_SchemeConfig.Color_Warning, "Hud", g_SchemeConfig.Color_Enabled, "Enabled");
			g_CSettings_Hud[iClient].Display = true;
			SetClientCookie(iClient, g_hCookie_Display, "1");
		}
	}
	return Plugin_Handled;
}

public Action EWM_Hud_Command_ToggleHUDType(int iClient, int iArgs)
{
	if((IsClientConnected(iClient) && IsClientInGame(iClient)))
	{
		if (g_CSettings_Hud[iClient].Type)
		{
			CPrintToChat(iClient, "%s%t %s%t %s%t", g_SchemeConfig.Color_Tag, "EW_Tag", g_SchemeConfig.Color_Warning, "Hud Type", g_SchemeConfig.Color_Enabled, "HUD");
			g_CSettings_Hud[iClient].Type = false;
			SetClientCookie(iClient, g_hCookie_HudType, "0");
		}
		else
		{
			CPrintToChat(iClient, "%s%t %s%t %s%t", g_SchemeConfig.Color_Tag, "EW_Tag", g_SchemeConfig.Color_Warning, "Hud Type", g_SchemeConfig.Color_Enabled, "KeyHintText");
			g_CSettings_Hud[iClient].Type = true;
			SetClientCookie(iClient, g_hCookie_HudType, "1");
		}
	}
	return Plugin_Handled;
}

public Action EWM_Hud_Command_ToggleHUDName(int iClient, int iArgs)
{
	if((IsClientConnected(iClient) && IsClientInGame(iClient)))
	{
		if (g_CSettings_Hud[iClient].Name)
		{
			CPrintToChat(iClient, "%s%t %s%t %s%t", g_SchemeConfig.Color_Tag, "EW_Tag", g_SchemeConfig.Color_Warning, "Hud Names", g_SchemeConfig.Color_Disabled, "Disabled");
			g_CSettings_Hud[iClient].Name = false;
			SetClientCookie(iClient, g_hCookie_HudName, "0");
		}
		else
		{
			CPrintToChat(iClient, "%s%t %s%t %s%t", g_SchemeConfig.Color_Tag, "EW_Tag", g_SchemeConfig.Color_Warning, "Hud Names", g_SchemeConfig.Color_Enabled, "Enabled");
			g_CSettings_Hud[iClient].Name = true;
			SetClientCookie(iClient, g_hCookie_HudName, "1");
		}
	}
	return Plugin_Handled;
}

public Action EWM_Hud_Command_Hudpos(int iClient, int iArgs)
{
	if((IsClientConnected(iClient) && IsClientInGame(iClient)))
	{
		if (GetCmdArgs() < 2)
		{
			CReplyToCommand(iClient, "%s%t %s%t: sm_hudpos <x> <y>. %t: %.3f %.3f", g_SchemeConfig.Color_Tag, "EW_Tag", g_SchemeConfig.Color_Warning, "Usage", "Default", g_SchemeConfig.Pos_HUD_X, g_SchemeConfig.Pos_HUD_Y);
			return Plugin_Handled;
		}
		char sBuffer[128];
		float HudPosX_validate;
		float HudPosY_validate;
		GetCmdArg(1, sBuffer, sizeof(sBuffer));
		HudPosX_validate = StringToFloat(sBuffer);

		GetCmdArg(2, sBuffer, sizeof(sBuffer));
		HudPosY_validate = StringToFloat(sBuffer);
		
		if(((HudPosX_validate >= 0.0 && HudPosX_validate <= 1.0) || HudPosX_validate == -1.0) && ((HudPosY_validate >= 0.0 && HudPosY_validate <= 1.0) || HudPosY_validate == -1.0))
		{
			g_CSettings_Hud[iClient].Pos_X = HudPosX_validate;
			g_CSettings_Hud[iClient].Pos_Y = HudPosY_validate;
			
			char sBuffer_cookie[32];
			FormatEx(sBuffer_cookie, sizeof(sBuffer_cookie), "%f/%f", HudPosX_validate, HudPosY_validate);
			SetClientCookie(iClient, g_hCookie_HudPos, sBuffer_cookie);
		} else 
		{
			CPrintToChat(iClient, "%s%t %s%t %s%t", g_SchemeConfig.Color_Tag, "EW_Tag", g_SchemeConfig.Color_Warning, "Hud Position", g_SchemeConfig.Color_Disabled, "Wrong");
			return Plugin_Handled;
		}

		CPrintToChat(iClient, "%s%t %s%t %s%t", g_SchemeConfig.Color_Tag, "EW_Tag", g_SchemeConfig.Color_Warning, "Hud Position", g_SchemeConfig.Color_Enabled, "Saved");
	}
	return Plugin_Handled;
}

public Action EWM_Hud_Command_HudColor(int iClient, int iArgs)
{
	if((IsClientConnected(iClient) && IsClientInGame(iClient)))
	{
		if (GetCmdArgs() < 4)
		{
			CReplyToCommand(iClient, "%s%t %s%t: sm_hudcolor <R> <G> <B> <A>. %t: %i %i %i %i", g_SchemeConfig.Color_Tag, "EW_Tag", g_SchemeConfig.Color_Warning, "Usage", "Default", g_SchemeConfig.Color_HUD[0], g_SchemeConfig.Color_HUD[1], g_SchemeConfig.Color_HUD[2], g_SchemeConfig.Color_HUD[3]);
			return Plugin_Handled;
		}
		char sBuffer[128];
		int iColor_validate[4];
		GetCmdArg(1, sBuffer, sizeof(sBuffer));
		iColor_validate[0] = StringToInt(sBuffer);

		GetCmdArg(2, sBuffer, sizeof(sBuffer));
		iColor_validate[1] = StringToInt(sBuffer);

		GetCmdArg(3, sBuffer, sizeof(sBuffer));
		iColor_validate[2] = StringToInt(sBuffer);

		GetCmdArg(4, sBuffer, sizeof(sBuffer));
		iColor_validate[3] = StringToInt(sBuffer);

		if(iColor_validate[0] >= 0 && iColor_validate[0] <= 255 && 
			iColor_validate[1] >= 0 && iColor_validate[1] <= 255 && 
			iColor_validate[2] >= 0 && iColor_validate[2] <= 255 && 
			iColor_validate[3] >= 0 && iColor_validate[3] <= 255)
		{
			g_CSettings_Hud[iClient].Color[0] = iColor_validate[0];
			g_CSettings_Hud[iClient].Color[1] = iColor_validate[1];
			g_CSettings_Hud[iClient].Color[2] = iColor_validate[2];
			g_CSettings_Hud[iClient].Color[3] = iColor_validate[3];
			char sBuffer_cookie[32];
			Format(sBuffer_cookie, sizeof(sBuffer_cookie), "%i/%i/%i/%i", iColor_validate[0], iColor_validate[1], iColor_validate[2], iColor_validate[3]);
			SetClientCookie(iClient, g_hCookie_HudColor, sBuffer_cookie);
		} else 
		{
			CPrintToChat(iClient, "%s%t %s%t %s%t", g_SchemeConfig.Color_Tag, "EW_Tag", g_SchemeConfig.Color_Warning, "Hud Color", g_SchemeConfig.Color_Disabled, "Wrong");
			return Plugin_Handled;
		}

		CPrintToChat(iClient, "%s%t %s%t %s%t", g_SchemeConfig.Color_Tag, "EW_Tag", g_SchemeConfig.Color_Warning, "Hud Color", g_SchemeConfig.Color_Enabled, "Saved");
	}
	return Plugin_Handled;
}

public Action EWM_Hud_Command_HudItems(int iClient, int iArgs)
{
	if(!iClient) return Plugin_Handled;
	
	if((IsClientConnected(iClient) && IsClientInGame(iClient)))
	{
		if (!AreClientCookiesCached(iClient)) return Plugin_Handled;
		
		if (GetCmdArgs() < 1)
		{
			CReplyToCommand(iClient, "%s%t %s%t: sm_huditems <1-%d>", g_SchemeConfig.Color_Tag, "EW_Tag", g_SchemeConfig.Color_Warning, "Usage", g_iMaxItemsCount);
			return Plugin_Handled;
		}
		
		char sArg[32];
		GetCmdArg(1, sArg, sizeof(sArg));
		
		int iCount;
		if (!StringToIntEx(sArg, iCount) || iCount <= 0 || iCount > g_iMaxItemsCount)
		{
			CReplyToCommand(iClient, "%s%t %s%t <1-%d>", g_SchemeConfig.Color_Tag, "EW_Tag", g_SchemeConfig.Color_Warning, "Invalid Amount", g_iMaxItemsCount);
			return Plugin_Handled;
		}
		
		char sBuffer_cookie[32];
		IntToString(iCount, sBuffer_cookie, sizeof(sBuffer_cookie));
		SetClientCookie(iClient, g_hCookie_HudItemsCount, sBuffer_cookie);
		
		g_CSettings_Hud[iClient].ItemsCount = iCount;
		CReplyToCommand(iClient, "%s%t %s%t %s%t", g_SchemeConfig.Color_Tag, "EW_Tag", g_SchemeConfig.Color_Warning, "Hud Items", g_SchemeConfig.Color_Enabled, "Saved")
	}
	
	return Plugin_Handled;
}

public Action EWM_Hud_Command_HudRotor(int iClient, int iArgs)
{
	if(!iClient) return Plugin_Handled;
	
	if((IsClientConnected(iClient) && IsClientInGame(iClient)))
	{
		if (!AreClientCookiesCached(iClient)) return Plugin_Handled;
		
		if (GetCmdArgs() < 1)
		{
			CReplyToCommand(iClient, "%s%t %s%t: sm_hudtime <1-%d>", g_SchemeConfig.Color_Tag, "EW_Tag", g_SchemeConfig.Color_Warning, "Usage", TIMEROTORHUD);
			return Plugin_Handled;
		}
		
		char sArg[32];
		GetCmdArg(1, sArg, sizeof(sArg));
		
		int iTime;
		if (!StringToIntEx(sArg, iTime) || iTime <= 0 || iTime > TIMEROTORHUD)
		{
			CReplyToCommand(iClient, "%s%t %s%t <1-%d>", g_SchemeConfig.Color_Tag, "EW_Tag", g_SchemeConfig.Color_Warning, "Invalid Amount", TIMEROTORHUD);
			return Plugin_Handled;
		}
		
		char sBuffer_cookie[32];
		IntToString(iTime, sBuffer_cookie, sizeof(sBuffer_cookie));
		SetClientCookie(iClient, g_hCookie_HudRotorTime, sBuffer_cookie);
		
		g_CSettings_Hud[iClient].RotorTime = iTime;
		CReplyToCommand(iClient, "%s%t %s%t %s%t", g_SchemeConfig.Color_Tag, "EW_Tag", g_SchemeConfig.Color_Warning, "Hud Rotation", g_SchemeConfig.Color_Enabled, "Saved")
	}
	
	return Plugin_Handled;
}