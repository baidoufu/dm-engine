// ==================== HumanPlayer 内联函数实现 ====================
// 从 HumanPlayer.h 中提取的大型内联函数体
// 被 HumanPlayer.h 底部 #include

inline VOID CHumanPlayer::AddMp(int nAdd)
{
	if (nAdd < 0)
	{
		nAdd = -nAdd;
		DecPropValue(PI_CURMP, nAdd);
	}
	else
		AddPropValue(PI_CURMP, nAdd);
	SendHpMpChanged();
}

inline VOID CHumanPlayer::AddHp(int nAdd)
{
	if (nAdd < 0)
	{
		nAdd = -nAdd;
		DecPropValue(PI_CURHP, nAdd);
	}
	else
		AddPropValue(PI_CURHP, nAdd);
	SendHpMpChanged(-nAdd);
}

inline VOID CHumanPlayer::GiveCredit(WORD wValue)
{
	WORD wCur = m_Humandesc.dbinfo.dwFlag[0] & 0xffff;
	if (wValue > ((WORD)0xffff) - wCur)
		wCur = 0xffff;
	else
		wCur += wValue;
	m_Humandesc.dbinfo.dwFlag[0] = (m_Humandesc.dbinfo.dwFlag[0] & 0xffff0000) + wCur;
	UpdateSubProp();
}

inline BOOL CHumanPlayer::TakeCredit(WORD wValue)
{
	WORD wCur = m_Humandesc.dbinfo.dwFlag[0] & 0xffff;
	if (wCur < wValue)return FALSE;
	wCur -= wValue;
	m_Humandesc.dbinfo.dwFlag[0] = (m_Humandesc.dbinfo.dwFlag[0] & 0xffff0000) + wCur;
	UpdateSubProp();
	return TRUE;
}

inline int CHumanPlayer::getskillpower(WORD id)
{
	//计算SKILL文件中Value1到Value2之间的随机值
	if (id == 64 || id == 65 || id == 30 || id == 8)
		return 0;
	Magic magicskill = CMagicManager::GetInstance()->GetMagic(id);
	USERMAGIC* pMagic = GetMagic(id);
	int pow = magicskill.skills[pMagic->magic.btLevel].value1;
	int power = magicskill.skills[pMagic->magic.btLevel].value2;
	if (GetPropValue(PI_LUCKY) >= 9)//幸运大于9
		return power;
	else if (GetPropValue(PI_LUCKY) >= 7)//幸运大于7
	{
		if ((power - pow) > 0)
		{
			int temp = ((power - pow) / 3) * 2;
			int adddamage = temp + rand() % ((power - pow) - temp + 1);
			return pow + adddamage;
		}
		else
			return pow;
	}
	else { //幸运小于7
		if ((power - pow) > 0)
			return pow + rand() % (power - pow + 1);
		else
			return pow;
	}
}

inline VOID CHumanPlayer::SendHpMpChanged(int damage, BYTE wEffect)
{
	int wHp = GetPropValue(PI_CURHP); // 当前生命值
	int wMaxHp = GetPropValue(PI_MAXHP); // 最大生命值
	int wMp = GetPropValue(PI_CURMP);
	int wMaxMp = GetPropValue(PI_MAXMP);
	WORD heathHP = 100; // 血条长度按百分之显示
	WORD WbiliHP = (wMaxHp > 0) ? (wHp * 100 / wMaxHp) : 0;// 百分比取整
	HealthStatus healthStatus;
	healthStatus.iAddHp = -damage;
	healthStatus.btCritFlag = wEffect;
	SendAroundMsg(GetId(), SM_HPMPCHANGED, WbiliHP, wMp, heathHP, (LPVOID)&healthStatus, sizeof(HealthStatus));
	if (CanRecvMsg())
		SendMsg(GetId(), SM_HPMPCHANGED, wHp, wMp, wMaxHp, (LPVOID)&healthStatus, sizeof(HealthStatus));
}
