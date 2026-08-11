///////////////////////////////////////////////////////////////////////
//文件名：g_TaskData.h
//版本：V1.80
//文件说明：1.80版新增任务系统的客户端相关显示数据
///////////////////////////////////////////////////////////////////////

#include "taskdata.h"

CTaskData  g_TaskData;

CTaskData::CTaskData(void)
{
	m_wAbandonTaskID = 0;	//放弃任务ID
	m_iMaxTaskCount = 0;	//最大任务数
	m_iFinishTaskCount = 0;	//总任务数
	m_iLoopNum = 0;			//跑环任务数
	m_iAchievement = 0; 	//积分
	m_iRevNum = 0;			//已接任务数

	RankUpInfo ri;
	ri.iKey = 1;
	ri.vTask.push_back(0);
	ri.vTask.push_back(1);
	ri.vTask.push_back(2);
	ri.vTask.push_back(3);
	ri.vTask.push_back(4);
	ri.vTask.push_back(20);
	ri.vTask.push_back(67);
	ri.vTask.push_back(68);
	ri.vTask.push_back(69);
	ri.vTask.push_back(75);
	ri.vTask.push_back(76);
	m_vRandUpInfo.push_back(ri);

	ri.iKey = 2;
	ri.vTask.clear();
	ri.vTask.push_back(5);
	ri.vTask.push_back(6);
	m_vRandUpInfo.push_back(ri);

	ri.iKey = 3;
	ri.vTask.clear();
	ri.vTask.push_back(7);
	ri.vTask.push_back(8);
	m_vRandUpInfo.push_back(ri);

	ri.iKey = 4;
	ri.vTask.clear();
	ri.vTask.push_back(9);
	m_vRandUpInfo.push_back(ri);

	ri.iKey = 5;
	ri.vTask.clear();
	ri.vTask.push_back(10);
	m_vRandUpInfo.push_back(ri);

	ri.iKey = 6;
	ri.vTask.clear();
	ri.vTask.push_back(11);
	ri.vTask.push_back(12);
	ri.vTask.push_back(62);
	m_vRandUpInfo.push_back(ri);

	ri.iKey = 7;
	ri.vTask.clear();
	ri.vTask.push_back(13);
	ri.vTask.push_back(14);
	ri.vTask.push_back(63);
	ri.vTask.push_back(64);
	m_vRandUpInfo.push_back(ri);

	ri.iKey = 8;
	ri.vTask.clear();
	ri.vTask.push_back(65);
	ri.vTask.push_back(66);
	ri.vTask.push_back(70);
	ri.vTask.push_back(77);
	m_vRandUpInfo.push_back(ri);
	
	ri.iKey = 9;
	ri.vTask.clear();
	ri.vTask.push_back(15);
	m_vRandUpInfo.push_back(ri);

	ri.iKey = 10;
	ri.vTask.clear();
	ri.vTask.push_back(16);
	m_vRandUpInfo.push_back(ri);

	ri.iKey = 11;
	ri.vTask.clear();
	ri.vTask.push_back(17);
	ri.vTask.push_back(71);
	ri.vTask.push_back(78);
	m_vRandUpInfo.push_back(ri);

	ri.iKey = 14;
	ri.vTask.clear();
	ri.vTask.push_back(18);
	m_vRandUpInfo.push_back(ri);

	ri.iKey = 15;
	ri.vTask.clear();
	ri.vTask.push_back(19);
	ri.vTask.push_back(72);
	ri.vTask.push_back(73);
	m_vRandUpInfo.push_back(ri);

	ri.iKey = 20;
	ri.vTask.clear();
	ri.vTask.push_back(21);
	ri.vTask.push_back(74);
	m_vRandUpInfo.push_back(ri);

	ri.iKey = 24;
	ri.vTask.clear();
	ri.vTask.push_back(45);
	ri.vTask.push_back(46);
	ri.vTask.push_back(54);
	ri.vTask.push_back(55);
	m_vRandUpInfo.push_back(ri);

	ri.iKey = 25;
	ri.vTask.clear();
	ri.vTask.push_back(47);
	ri.vTask.push_back(56);
	ri.vTask.push_back(57);
	ri.vTask.push_back(58);
	m_vRandUpInfo.push_back(ri);

	ri.iKey = 26;
	ri.vTask.clear();
	ri.vTask.push_back(48);
	ri.vTask.push_back(49);
	ri.vTask.push_back(59);
	ri.vTask.push_back(60);
	m_vRandUpInfo.push_back(ri);

	ri.iKey = 27;
	ri.vTask.clear();
	ri.vTask.push_back(50);
	ri.vTask.push_back(51);
	ri.vTask.push_back(61);
	m_vRandUpInfo.push_back(ri);

	ri.iKey = 28;
	ri.vTask.clear();
	ri.vTask.push_back(52);
	ri.vTask.push_back(53);
	m_vRandUpInfo.push_back(ri);
}

CTaskData::~CTaskData(void)
{
	m_mapMainTask.clear();
	m_mapBranchTask.clear();
}

void CTaskData::ClearTask()
{
	m_wAbandonTaskID = 0;	//放弃任务ID
	m_iMaxTaskCount = 0;	//最大任务数
	m_iFinishTaskCount = 0;	//总任务数
	m_iLoopNum = 0;			//跑环任务数
	m_iAchievement = 0; 	//积分
	m_iRevNum = 0;			//已接任务数

	m_mapMainTask.clear();
	m_mapBranchTask.clear();
}

_Task* CTaskData::FindTask(WORD wThemeID,WORD wTaskID,BOOL bMain)
{
	MTaskList* pTaskList = &m_mapMainTask;
	if (!bMain) pTaskList = &m_mapBranchTask;
	
	MTaskList::iterator it = pTaskList->find(wThemeID);

	if (it != pTaskList->end())
	{
		_ThemeTask& ThemeTask = it->second;
		VTask::iterator itT = ThemeTask.vTask.begin();
		while (itT != ThemeTask.vTask.end())
		{
			if(itT->wTaskID == wTaskID)
				return &(*itT);

			itT++;
		}
	}

	return NULL;
}

_Task* CTaskData::FindTask(WORD wTaskID,BOOL bMain)
{
	MTaskList* pTaskList = &m_mapMainTask;
	if (!bMain) pTaskList = &m_mapBranchTask;

	MTaskList::iterator it = pTaskList->begin();
	while (it != pTaskList->end())
	{
		if (it != pTaskList->end())
		{
			_ThemeTask& ThemeTask = it->second;
			VTask::iterator itT = ThemeTask.vTask.begin();
			while (itT != ThemeTask.vTask.end())
			{
				if(itT->wTaskID == wTaskID)
					return &(*itT);

				itT++;
			}
		}

		it++;
	}

	return FALSE;
}

_ThemeTask* CTaskData::FindThemeTask(WORD wThemeID,BOOL bMain)
{
	MTaskList* pTaskList = &m_mapMainTask;
	if (!bMain) pTaskList = &m_mapBranchTask;

	MTaskList::iterator it = pTaskList->find(wThemeID);
	if (it != pTaskList->end())
	{
		return &(it->second);
	}
	
	return NULL;
}
BOOL CTaskData::DeleteTask(WORD wTaskID,BOOL bMain)
{
	MTaskList* pTaskList = &m_mapMainTask;
	if (!bMain) pTaskList = &m_mapBranchTask;

	MTaskList::iterator it = pTaskList->begin();
	while (it != pTaskList->end())
	{
		if (it != pTaskList->end())
		{
			_ThemeTask& ThemeTask = it->second;
			VTask::iterator itT = ThemeTask.vTask.begin();
			while (itT != ThemeTask.vTask.end())
			{
				if(itT->wTaskID == wTaskID)
				{
					ThemeTask.vTask.erase(itT);

					if (ThemeTask.vTask.size() <= 0)
						pTaskList->erase(it);
					m_iRevNum--;
					if (m_iRevNum < 0 ) m_iRevNum = 0;

					return TRUE;
				}

				itT++;
			}
		}

		it++;
	}

	return FALSE;
}

BOOL CTaskData::DeleteTask(WORD wTaskMain,WORD wTaskBranch,BOOL bMain)
{
	MTaskList* pTaskList = &m_mapMainTask;
	if (!bMain) pTaskList = &m_mapBranchTask;
	
	MTaskList::iterator it = pTaskList->find(wTaskMain);

	if (it != pTaskList->end())
	{
		_ThemeTask& ThemeTask = it->second;
		VTask::iterator itT = ThemeTask.vTask.begin();
		while (itT != ThemeTask.vTask.end())
		{
			if(itT->wTaskID == wTaskBranch)
			{
				ThemeTask.vTask.erase(itT);
				
				if (ThemeTask.vTask.size() <= 0)
					pTaskList->erase(it);
				m_iRevNum--;
				if (m_iRevNum < 0 ) m_iRevNum = 0;

				return TRUE;
			}

			itT++;
		}
	}

	return FALSE;
}

_Task* CTaskData::AppendTask(WORD wTaskMain,WORD wTaskBranch,_Task& task,BOOL bMain)
{
	DeleteTask(wTaskMain,wTaskBranch,bMain);

	MTaskList* pTaskList = &m_mapMainTask;
	if (!bMain) pTaskList = &m_mapBranchTask;

	MTaskList::iterator it = pTaskList->find(wTaskMain);

	if (it != pTaskList->end())
	{
		task.wTaskID = wTaskBranch;
		it->second.vTask.push_back(task);		
		return &(it->second.vTask[it->second.vTask.size() - 1]);
	}
	else
	{
		m_iRevNum++;
		if (m_iRevNum > m_iMaxTaskCount) 
			m_iRevNum = m_iMaxTaskCount;

		_ThemeTask themetask;
		task.wTaskID = wTaskBranch;
		themetask.wThemeID = wTaskMain;
		themetask.vTask.push_back(task);		
		(*pTaskList)[wTaskMain] = themetask;
		return &((*pTaskList)[wTaskMain].vTask[0]);
	}
}

bool CTaskData::GetMarkedString(string& str,string& strMark,char cEnd,string& strResult)
{
	strResult.clear();
	int iPos = str.find(strMark,0);
	if (iPos >= 0)
	{
		iPos += strMark.size();
		int iEndPos = str.find(cEnd,iPos);
		if(iEndPos == -1) iEndPos = str.size();
		strResult = str.substr(iPos,iEndPos - iPos);
	}

	if (strResult.length() > 0) return true;

	return false;
}

void CTaskData::RemoveGuilidTask(WORD wTaskID)
{
	if (wTaskID == 0)
		return;

	vector<_OldTask*>::iterator it = m_vecGuildTask.begin();

	for (;it!= m_vecGuildTask.end();it++)
	{
		_OldTask* pTask = *it;
		if (pTask && pTask->wTaskID == wTaskID)
		{
			delete pTask;
			m_vecGuildTask.erase(it);
			break;
		}
	}
}

void CTaskData::ClearAllForGuild()
{
	for (int i = 0;i<(int)m_vecGuildTask.size();i++)
	{
		delete(m_vecGuildTask[i]);
	}

	m_vecGuildTask.clear();
}

void CTaskData::SetTracked(WORD wTaskID,bool b)
{
	_Task* pTask = FindTask(wTaskID,true);
	if (!pTask) pTask = FindTask(wTaskID,false);

	if (!pTask) return;
	
	pTask->bTrack = b;
}

CTaskData::RankUpInfo* CTaskData::GetRankUpInfo(int iLevel)
{
	for (int i = 0;i<m_vRandUpInfo.size();i++)
	{
		RankUpInfo& ri = m_vRandUpInfo.at(i);

		if (ri.iKey == iLevel)
		{
			return &ri;
		}
	}

	return NULL;
}