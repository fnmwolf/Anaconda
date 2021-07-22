#define SteamCallback(C) CCallback<RuntimeData, C##_t, false> C##CallbackMember; \
						CCallResult<RuntimeData, C##_t> C##CallResult; \
						void Callback##C(C##_t * Parameter)

#define SteamRegisterCallback(C) C##CallbackMember(this, &RuntimeData::Callback##C)

#define generate_event __pyx_f_10extensions_10Steamworks_7wrapper_generate_event

static void generate_event(int);

struct RuntimeData
{
	bool InitSuccessful;
	const char * LastError;

	SteamCallback(UserStatsReceived)
	{
		if(!SteamUtils())
			return;

		if(Parameter->m_nGameID != SteamUtils()->GetAppID())
			return;

		if(Parameter->m_eResult == k_EResultOK)
		{
			generate_event(1);
		}
	}

	SteamCallback(UserStatsStored)
	{

		if(!SteamUtils())
			return;

		if(Parameter->m_nGameID != SteamUtils()->GetAppID())
			return;

		if(Parameter->m_eResult == k_EResultOK)
		{
			generate_event(2);
		}
	}

	bool AchievementStored_IsGroupAchievement;
	char AchievementStored_Name[k_cchStatNameMax];
	uint32 AchievementStored_CurrentProgress;
	uint32 AchievementStored_MaxProgress;

	SteamCallback(UserAchievementStored)
	{

		if(!SteamUtils())
			return;

		if(Parameter->m_nGameID != SteamUtils()->GetAppID())
			return;

		AchievementStored_IsGroupAchievement = Parameter->m_bGroupAchievement;
		strcpy(AchievementStored_Name, Parameter->m_rgchAchievementName);
		AchievementStored_CurrentProgress = Parameter->m_nCurProgress;
		AchievementStored_MaxProgress = Parameter->m_nMaxProgress;

		generate_event(3);
	}

	RuntimeData() :	SteamRegisterCallback(UserStatsReceived),
					SteamRegisterCallback(UserStatsStored),
					SteamRegisterCallback(UserAchievementStored)
	{
		AchievementStored_IsGroupAchievement = false;
		*AchievementStored_Name = 0;
		AchievementStored_CurrentProgress = 0;
		AchievementStored_MaxProgress = 0;

		InitSuccessful = SteamAPI_Init();
	}

	~RuntimeData()
	{
		SteamAPI_Shutdown();
	}

};