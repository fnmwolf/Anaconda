/*
Code by Jamie McLaughlin.
zlib license.
*/

#include "Common.h"

// C interface

void * create_object()
{
    RuntimeData * data = new RuntimeData;
    return (void*)data;
}

inline RuntimeData * get_data(void * data)
{
    return (RuntimeData*)data;
}

void destroy_object(void * handle)
{
    RuntimeData * data = get_data(handle);
    delete data;
}

void update_object()
{
    SteamAPI_RunCallbacks();
}

// conditions

int is_initialized(void * handle)
{
    return get_data(handle)->InitSuccessful;
}

// actions

void set_achievement(char * name)
{
    if(!SteamUserStats()||!SteamUser()||!SteamUser()->BLoggedOn())
        return;

    SteamUserStats()->SetAchievement(name);

    return;
}

void request_data()
{
    if(!SteamUserStats()||!SteamUser()||!SteamUser()->BLoggedOn())
        return;

    SteamUserStats()->RequestCurrentStats();

    return;
}

void set_int_statistic(char * name, int32 value)
{
    if(!SteamUserStats()||!SteamUser()||!SteamUser()->BLoggedOn())
        return;

    SteamUserStats()->SetStat(name, value);

    return;
}

void set_float_statistic(char * name, float value)
{
    if(!SteamUserStats()||!SteamUser()||!SteamUser()->BLoggedOn())
        return;

    SteamUserStats()->SetStat(name, value);

    return;
}

void update_avgrate(char * statistic, float value, float duration)
{
    if(!SteamUserStats()||!SteamUser()||!SteamUser()->BLoggedOn())
        return;

    SteamUserStats()->UpdateAvgRateStat(statistic, value, duration);

    return;
}

void store_data(void * handle) 
{
    if(!SteamUserStats()||!SteamUser()||!SteamUser()->BLoggedOn()||!
       get_data(handle)->InitSuccessful)
        return;
    SteamUserStats()->StoreStats();
    return;
}

// expressions

int get_app_id()
{
    if(!SteamUtils())
        return 0;

    return SteamUtils()->GetAppID();
}

char * get_player_name()
{
    if(!SteamFriends()) {
        return "";
    }
    return (char*)SteamFriends()->GetPersonaName();
}

char * get_achievement_api(void * handle)
{
    return (char*)get_data(handle)->AchievementStored_Name;
}

uint32 get_current_progress(void * handle)
{
    return get_data(handle)->AchievementStored_CurrentProgress;
}

uint32 get_max_progress(void * handle)
{
    return get_data(handle)->AchievementStored_MaxProgress;
}

char * get_achievement_display(char * achievement)
{
    if(!SteamUserStats()) {
        return "";
    }
    return (char*)SteamUserStats()->GetAchievementDisplayAttribute(achievement,
        "name");
}

char * get_achievement_description(char * achievement)
{
    if(!SteamUserStats()) {
        return "";
    }
    return (char*)SteamUserStats()->GetAchievementDisplayAttribute(achievement,
        "desc");
}

int get_statistics_int(char * statistic)
{
    if(!SteamUserStats()) {
        return 0;
    }
    int32 value;
    SteamUserStats()->GetStat(statistic, &value);
    return value;
}


float get_statistics_float(char * statistic)
{
    if(!SteamUserStats()) {
        return 0;
    }
    float value;
    SteamUserStats()->GetStat(statistic, &value);
    return value;
}