///////////////////////////////////////////////////////////////////////
//文件名：g_TaskData.h
//版本：V1.80
//文件说明：1.80版新增任务系统的客户端相关显示数据
///////////////////////////////////////////////////////////////////////

#pragma once

#include "Global/Global.h"
#include "TagText.h"

struct _WuSeTaskStruct
{
	std::string strName;
	int color;
	std::string type;
	std::string strInfo;
	std::string strCommand1;
	std::string strCommand2;
	int fresh;

	_WuSeTaskStruct()
	{
		strName = "";
		color = 0;
		type = "";
		strCommand1 = "";
		strCommand2 = "";
		strInfo = "";
		fresh = 0;
	}
};

struct _Task
{
	_Task()
	{
		bMain = FALSE;
		wTaskID = 0;
		dwTaskPic = 0;
		bTrack = false;
		strTaskName.clear();
		strTaskDesc.clear();
		strTaskAim.clear();
	}
	BOOL	bMain;			//主线任务
	BOOL	bTrack;			//追踪
	WORD	wTaskID;		//任务ID
	string	strTaskName;	//任务名称
	DWORD	dwTaskPic;		//任务图片
	string	strTaskDesc;    //任务描述
	string	strTaskAim;		//任务目的
};

typedef std::vector<_Task> VTask;
struct _ThemeTask
{
	WORD	wThemeID;		//主题ID
	string	strTaskTile;	//任务主题
	VTask	vTask;
};

typedef std::map<WORD,_ThemeTask> MTaskList;

class CTaskData
{
public:
	struct RankUpInfo 
	{
		int iKey;
		vector<int> vTask;
	};
	typedef vector<RankUpInfo> VRankUpInfo;

	struct _OldTask
	{
		_OldTask()
		{
			wTaskID = 0;
		}
		WORD wTotalNum;//任务总数
		WORD wRevNum;//已接数目
		WORD wTaskID;      // 任务ID
		string strTaskName;   // 任务名称
		CTagText strDesc;       // 任务描述
	};
	typedef vector<_OldTask*> VOldTaskList;
public:
	CTaskData(void);
	~CTaskData(void);
	void ClearTask();//清除所有任务
	_Task* FindTask(WORD wTaskID,BOOL bMain);
	_Task* FindTask(WORD wThemeID,WORD wTaskID,BOOL bMain);//根据主线和支线的ID找到相应的任务
	_ThemeTask* FindThemeTask(WORD wThemeID,BOOL bMain);
	BOOL DeleteTask(WORD wTaskID,BOOL bMain);	
	BOOL DeleteTask(WORD wTaskMain,WORD wTaskBranch,BOOL bMain);//删除任务
	_Task* AppendTask(WORD wTaskMain,WORD wTaskBranch,_Task& task,BOOL bMain);
	MTaskList& GetMainTaskList(){return m_mapMainTask;}
	MTaskList& GetBranchTaskList(){return m_mapBranchTask;}	
	
	void SetTracked(WORD wTaskID,bool b);

	void SetAbandonTaskID(WORD wID){m_wAbandonTaskID = wID;}
	WORD GetAbandonTaskID(){return m_wAbandonTaskID;}

	void SetMaxTaskNum(int i){m_iMaxTaskCount = i;}
	int	 GetMaxTaskNum(){return m_iMaxTaskCount;}

	void SetFinishNum(int i){m_iFinishTaskCount = i;}
	int  GetFinishNum(){return m_iFinishTaskCount;}

	void SetLoopNum(int i){m_iLoopNum = i;}
	int  GetLoopNum(){return m_iLoopNum;}

	void SetAchievement(int i){m_iAchievement = i;}
	int  GetAchievement(){return m_iAchievement;}

	int GetRevTaskNum(){return m_iRevNum;}

	bool GetMarkedString(string& str,string& strMark,char cEnd,string& strResult);


	const char* GetGuildTaskName(int i) { return m_vecGuildTask[i]->strTaskName.c_str();}
	inline WORD GetGuildTaskID(int i) { return m_vecGuildTask[i]->wTaskID;}
	VOldTaskList& GetGuildTaskList() { return m_vecGuildTask;}
	inline int GetGuildTaskCount() { return (int)m_vecGuildTask.size();}
	void RemoveGuilidTask(WORD wTaskID);
	void ClearAllForGuild();
	RankUpInfo* GetRankUpInfo(int iLevel);

private:
	MTaskList	m_mapMainTask;		//主线任务
	MTaskList	m_mapBranchTask;	//支线任务
	WORD		m_wAbandonTaskID;	//放弃任务ID
	int			m_iMaxTaskCount;	//最大任务数
	int			m_iFinishTaskCount;	//总任务数
	int			m_iLoopNum;			//跑环任务数
	int			m_iAchievement;		//积分
	int			m_iRevNum;			//已接任务数

	VOldTaskList	m_vecGuildTask;	//行会任务
	VRankUpInfo		m_vRandUpInfo;	//等级提升
};

extern CTaskData  g_TaskData;
