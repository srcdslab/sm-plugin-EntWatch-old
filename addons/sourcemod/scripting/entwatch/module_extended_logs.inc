#define EW_MODULE_ELOGS

ConVar	g_hCvar_ELog_Type;
ConVar	g_hCvar_ELog_Webhook;

stock void EWM_ELogs_OnPluginStart()
{
	g_hCvar_ELog_Type		= CreateConVar("entwatch_elog_type", "0", "How should elogs be notified (0 = Server, 1 = Discord)", _, true, 0.0, true, 1.0);
	g_hCvar_ELog_Webhook	= CreateConVar("entwatch_elog_webhook", "", "The webhook URL of your Discord channel.", FCVAR_PROTECTED);
}

stock void EWM_ELogs_OnConfigsExecuted()
{
	
}

stock void EWM_ELogs_Log(int client, int target, const char[] message)
{
	if (g_hCvar_ELog_Type.IntValue == 1)
	{
		char sWebhook[64];
		Format(sWebhook, sizeof(sWebhook), "entwatch");

		char sMessage[4096];
		char sTime[64];
		int iTime = GetTime();
		FormatTime(sTime, sizeof(sTime), "%m/%d/%Y @ %H:%M:%S", iTime);

		char currentMap[PLATFORM_MAX_PATH];
		GetCurrentMap(currentMap, sizeof(currentMap));

		Format(sMessage, sizeof(sMessage), "*%s - %s* ```%s```", currentMap, sTime, message);

		if(StrContains(sMessage, "\"") != -1)
			ReplaceString(sMessage, sizeof(sMessage), "\"", "");

		char szWebhookURL[1000];
		g_hCvar_ELog_Webhook.GetString(szWebhookURL, sizeof szWebhookURL);

		Webhook webhook = new Webhook(sMessage);

		webhook.Execute(szWebhookURL, OnWebHookExecuted);
		delete webhook;
	}
	else
	{
		LogAction(client, target, message);
	}
}

public void OnWebHookExecuted(HTTPResponse response, DataPack pack)
{
	if (response.Status != HTTPStatus_OK)
	{
		LogError("Failed to send entwatch webhook");
	}
}

stock void EWM_ELogs_Use(class_ItemList ItemTest, int iClient, int iAbility)
{
	char sMessage[4096];

	if(iAbility == 0) Format(sMessage, sizeof(sMessage), "\"%L\" used item \"%s\"", iClient, ItemTest.Name);
	else if(iAbility == 1) Format(sMessage, sizeof(sMessage), "\"%L\" used First Ability of item \"%s\"", iClient, ItemTest.Name);
	else Format(sMessage, sizeof(sMessage), "\"%L\" used Second Ability of item \"%s\"", iClient, ItemTest.Name);

	EWM_ELogs_Log(iClient, -1, sMessage);
}

stock void EWM_ELogs_PickUp(class_ItemList ItemTest, int iClient)
{
	char sMessage[4096];
	Format(sMessage, sizeof(sMessage), "\"%L\" has picked up item \"%s\"", iClient, ItemTest.Name);
	EWM_ELogs_Log(iClient, -1, sMessage);
}

stock void EWM_ELogs_Drop(class_ItemList ItemTest, int iClient)
{
	char sMessage[4096];
	Format(sMessage, sizeof(sMessage), "\"%L\" has dropped item \"%s\"", iClient, ItemTest.Name);
	EWM_ELogs_Log(iClient, -1, sMessage);
}

stock void EWM_ELogs_Disconnect(class_ItemList ItemTest, int iClient)
{
	char sMessage[4096];
	Format(sMessage, sizeof(sMessage), "\"%L\" disconnected with item \"%s\"", iClient, ItemTest.Name);
	EWM_ELogs_Log(iClient, -1, sMessage);
}

stock void EWM_ELogs_PlayerDeath(class_ItemList ItemTest, int iClient)
{
	char sMessage[4096];
	Format(sMessage, sizeof(sMessage), "\"%L\" has died with item \"%s\"", iClient, ItemTest.Name);
	EWM_ELogs_Log(iClient, -1, sMessage);
}
