#pragma once
/*
 *      Copyright (c) 2018~now Palo Kisa <palo.kisa@gmail.com>
 *
 *      Copyright (C) 2014 Josef Rokos
 *      http://github.com/PepaRokos/xbmc-pvr-addons/
 *
 *      Copyright (C) 2011 Pulse-Eight
 *      http://www.pulse-eight.com/
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this addon; see the file COPYING.  If not, write to
 *  the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
 *  http://www.gnu.org/copyleft/gpl.html
 *
 */

#include <vector>
#include "client.h"
#include "p8-platform/threads/threads.h"
#include "apimanager.h"
#include <mutex>
#include <memory>
#include <condition_variable>

struct PVRIptvEpgEntry
{
  unsigned    iBroadcastId;
  int         iChannelId;
  int         iGenreType;
  int         iGenreSubType;
  time_t      startTime;
  time_t      endTime;
  std::string strTitle;
  std::string strPlotOutline;
  std::string strPlot;
  std::string strIconPath;
  std::string strGenreString;
  std::string strEventId;
  std::string strStreamURL;
  bool availableTimeshift;
};

typedef std::map<time_t, PVRIptvEpgEntry> epg_entry_container_t;
struct PVRIptvEpgChannel
{
  std::string                  strId;
  std::string                  strName;
  epg_entry_container_t epg;
};

struct PVRIptvChannel
{
  bool        bIsRadio;
  int         iUniqueId;
  int         iChannelNumber;
  int         iEncryptionSystem;
  int         iTvgShift;
  std::string strChannelName;
  std::string strIconPath;
  std::string strStreamURL;
  std::string strId;
  std::string strGroupId;
};

struct PVRIptvChannelGroup
{
  bool              bRadio;
  std::string       strGroupId;
  std::string       strGroupName;
  std::vector<int>  members;
};

struct PVRIptvRecording
{
  std::string		strRecordId;
  std::string		strTitle;
  std::string		strStreamUrl;
  std::string		strPlotOutline;
  std::string		strPlot;
  std::string		strChannelName;
  time_t		startTime;
  int			duration;
  std::string strDirectory;
  bool bRadio;
};

struct PVRIptvTimer
{
  unsigned int    iClientIndex;
  int             iClientChannelUid;
  time_t          startTime;
  time_t          endTime;
  PVR_TIMER_STATE state;                                     /*!< @brief (required) the state of this timer */
  std::string     strTitle;
  std::string     strSummary;
  int             iLifetime;
  bool            bIsRepeating;
  time_t          firstDay;
  int             iWeekdays;
  int             iEpgUid;
  unsigned int    iMarginStart;
  unsigned int    iMarginEnd;
  int             iGenreType;
  int             iGenreSubType;
};

typedef std::vector<PVRIptvChannelGroup> group_container_t;
typedef std::vector<PVRIptvChannel> channel_container_t;
typedef std::map<std::string, PVRIptvEpgChannel> epg_container_t;
typedef std::vector<PVRIptvRecording> recording_container_t;
typedef std::vector<PVRIptvTimer> timer_container_t;

class DataContainer
{
public:
};

class PVRIptvData : public P8PLATFORM::CThread
{
public:
  PVRIptvData(const std::string & userName
      , const std::string & password
      , bool hdEnabled
      , int iEpgMaxDays);
  virtual ~PVRIptvData(void);

  int GetChannelsAmount(void);
  PVR_ERROR GetChannels(ADDON_HANDLE handle, bool bRadio);
  PVR_ERROR GetChannelStreamUrl(const PVR_CHANNEL* channel, std::string & streamUrl) const;
  PVR_ERROR GetEPGForChannel(ADDON_HANDLE handle, const PVR_CHANNEL &channel, time_t iStart, time_t iEnd);
  PVR_ERROR IsEPGTagPlayable(const EPG_TAG* tag, bool* bIsPlayable) const;
  PVR_ERROR GetEPGStreamUrl(const EPG_TAG* tag, std::string & streamUrl) const;
  PVR_ERROR SetEPGTimeFrame(int iDays);
  int GetChannelGroupsAmount(void);
  PVR_ERROR GetChannelGroups(ADDON_HANDLE handle, bool bRadio);
  PVR_ERROR GetChannelGroupMembers(ADDON_HANDLE handle, const PVR_CHANNEL_GROUP &group);
  int GetRecordingsAmount();
  PVR_ERROR GetRecordings(ADDON_HANDLE handle);
  PVR_ERROR GetRecordingStreamUrl(const PVR_RECORDING* recording, std::string & streamUrl) const;
  void GetRecordingsUrls();
  int GetTimersAmount();
  PVR_ERROR GetTimers(ADDON_HANDLE handle);
  PVR_ERROR AddTimer(const PVR_TIMER &timer);
  PVR_ERROR DeleteRecord(const std::string &strRecordId);
  PVR_ERROR DeleteRecord(int iRecordId);
  PVR_ERROR GetDriveSpace(long long *iTotal, long long *iUsed);
  bool LoggedIn() const;

protected:
  static int ParseDateTime(std::string strDate);

protected:
  bool KeepAlive();
  void KeepAliveJob();
  bool LoadPlayList(void);
  bool LoadEPG(time_t iStart, bool bSmallStep);
  void ReleaseUnneededEPG();
  //! \return true if actual update was performed
  bool LoadEPGJob();
  bool LoadRecordings();
  void LoadRecordingsJob();
  void SetLoadRecordings();
  void LoginLoop();
  bool WaitForChannels() const;

protected:
  virtual void *Process(void) override;

private:
  bool                              m_bKeepAlive;
  bool                              m_bLoadRecordings;
  mutable std::mutex                m_mutex;
  bool                              m_bChannelsLoaded;
  mutable std::condition_variable   m_waitCond;

  // stored data from backend (used by multiple threads...)
  std::shared_ptr<const group_container_t> m_groups;
  std::shared_ptr<const channel_container_t> m_channels;
  std::shared_ptr<const epg_container_t> m_epg;
  std::shared_ptr<const recording_container_t> m_recordings;
  std::shared_ptr<const timer_container_t> m_timers;
  long long m_recordingAvailableDuration;
  long long m_recordingRecordedDuration;
  time_t m_epgMinTime;
  time_t m_epgMaxTime;
  int m_epgMaxDays;

  // data used only by "job" thread
  bool m_bEGPLoaded;
  time_t m_iLastStart;
  time_t m_iLastEnd;
  time_t m_epgLastFullRefresh;
  bool m_bHdEnabled;

  ApiManager                        m_manager;
};
