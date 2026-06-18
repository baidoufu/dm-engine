#pragma once
class CHumanPlayer;
class CGroupObject;
class CGroupObjectMgr : public xSingletonClass<CGroupObjectMgr>
{
public:
	CGroupObjectMgr(VOID);
	virtual ~CGroupObjectMgr(VOID);
	BOOL CreateGroup(CHumanPlayer* pLeader, CHumanPlayer* pFirstMember);
	VOID DestroyGroup(CGroupObject* pObject);
private:
	xObjectPool<CGroupObject> m_xGroupObject;
};