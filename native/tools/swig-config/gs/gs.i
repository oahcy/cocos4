%module(target_namespace="jsb") gs

#pragma SWIG nowarn=503,302,401,317,402

%insert(header_file) %{
#pragma once
#include "bindings/jswrapper/SeApi.h"
#include "bindings/manual/jsb_conversions.h"
#include "vendor/gs/Framework/commons/GsCallback.h"
#include "vendor/gs/Framework/commons/GsTypes.h"
#include "vendor/gs/Framework/commons/GsServices.h"
#include "vendor/gs/Framework/Achievements.h"
#include "vendor/gs/Framework/Friends.h"
#include "vendor/gs/Framework/RemoteStorage.h"
#include "vendor/gs/Framework/Stats.h"
#include "vendor/gs/Framework/Utils.h"
%}

%{
#include "bindings/auto/jsb_gs_auto.h"
#include "vendor/gs/Framework/JsbConversions.h"
using namespace cc::Gs;
%}

%import "base/Macros.h"

%ignore cc::Gs::AsyncCallbackBase;
%ignore cc::Gs::EventDelegateBase;
%ignore cc::RefCounted;

// Native-side helper for combining FriendFlags; JS passes a plain bitmask number.
%ignore cc::Gs::hasFlag;

// Result structs are converted to plain JS objects by JsbConversions.h, so the
// SWIG-wrapped classes (and their member setters) are never used from JS.
%ignore cc::Gs::AchievementDefinition;
%ignore cc::Gs::AchievementState;
%ignore cc::Gs::AchievementIdsResult;
%ignore cc::Gs::AchievementDefinitionResult;
%ignore cc::Gs::AchievementStateResult;
%ignore cc::Gs::FriendInfo;
%ignore cc::Gs::FriendListResult;
%ignore cc::Gs::AvatarImage;
%ignore cc::Gs::FriendsGroupInfo;
%ignore cc::Gs::FriendsGroupListResult;
%ignore cc::Gs::FileInfo;
%ignore cc::Gs::QuotaInfo;
%ignore cc::Gs::FileList;
%ignore cc::Gs::StatIntResult;
%ignore cc::Gs::StatFloatResult;

// tick is driven internally by the engine Tick event (_tickListener in
// GsServicesCommon), so it must not be exposed to JS.
%ignore cc::Gs::IGsServices::tick;

%nodefaultctor cc::Gs::IGsServices;
%nodefaultdtor cc::Gs::IGsServices;
%nodefaultctor cc::Gs::IAchievements;
%nodefaultdtor cc::Gs::IAchievements;
%nodefaultctor cc::Gs::IFriends;
%nodefaultdtor cc::Gs::IFriends;
%nodefaultctor cc::Gs::IRemoteStorage;
%nodefaultdtor cc::Gs::IRemoteStorage;
%nodefaultctor cc::Gs::IStats;
%nodefaultdtor cc::Gs::IStats;
%nodefaultctor cc::Gs::IUtils;
%nodefaultdtor cc::Gs::IUtils;

%import "base/Ptr.h"
%import "base/RefCounted.h"

%include "vendor/gs/Framework/commons/GsCallback.h"
%include "vendor/gs/Framework/commons/GsTypes.h"
%include "vendor/gs/Framework/commons/GsServices.h"
%include "vendor/gs/Framework/Achievements.h"
%include "vendor/gs/Framework/Friends.h"
%include "vendor/gs/Framework/RemoteStorage.h"
%include "vendor/gs/Framework/Stats.h"
%include "vendor/gs/Framework/Utils.h"
