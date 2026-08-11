#pragma once
// 包裹格子
#include "Control.h"
#include "GameData/Good.h"
#include "XmlUser.h"
#include "GameData/ItemCfgMgr.h"

class CGoodGrid : public CControl
{
	DTI_DECLARE(CGoodGrid, CControl)
public:
    CGoodGrid();
	CGoodGrid(int iWCells, int iHCells, int iCellWidth, int iCellHeight, int iCellDisW, int iCellDisH);
	~CGoodGrid(void);
    virtual bool Create(CControl * pParent, int iX, int iY);
	virtual bool CreateXML(CControl* pParent,CXmlGoodGrid* grid);
    virtual void Draw(void); 
    virtual bool OnLeftButtonDClick(int iX,int iY);
    virtual bool OnLeftButtonDown(int iX,int iY);
	virtual bool OnLeftButtonUp(int iX,int iY);
    virtual bool OnRightButtonUp(int iX,int iY);
    virtual bool OnMouseMove(int iX,int iY);
	virtual bool Msg(DWORD dwMsg,DWORD dwData,CControl * pControl);
    bool IsInGrid(int iX,int iY);
    void DrawGridGood(int x,int y,CGood& good);
    void DrawEquipClothes(CGood& good);                        //绘制装备栏衣服
    void DrawEquipWeapon(CGood& good);                         //绘制装备栏武器
    void DrawSpecialWeapon(CGood& good);                       //特殊装备栏武器上绘制特效
    void DrawMedalNecklace(int x,int y,CGood& good);           //绘制装备栏勋章和项链
    void DrawOtherEquip(int x,int y,CGood& good);              //绘制装备栏中的左右手镯 左右戒指 鞋子 腰带 宝石 杂物
	void DrawShield(int x,int y,CGood& good);				   //绘制装备栏中的盾牌
	void DrawWenPeiEquip(int x,int y,CGood& good);			   //绘制装备栏中的纹佩

    static void DrawOneGood(int x,int y,CGood& good,LPPOS pScale = NULL,int iType = 1);
    static void DrawSelectedGood(int iX,int iY);
    static stGoodFrom& GetDropGoodFrom();
    static void SetDropGoodFrom(stGoodFrom& stFrom);
    static void ClearGoodFrom();
	static bool ReleaseDrop();
	static void DrawCloth(int x,int y,bool bTianYi,bool bOnBigLeopard,int iWearGold,int iArmId,int iCloseId,WORD iBody,int iSex,int iMaskLevel,DWORD bodyColor,BYTE byHairType,BYTE byHairColor,CGood& Cloth,CGood& Shield,CGood& Helmet,bool alpha = false,int iWingLevel = 0,int iStrongLevel = 0);
	static void DrawWenPei(int x,int y,CGood& good);
	void SetGridType(int iGridType);
	static LPTexture GetBasicEquipTex(CGood& good); 

	static int ConvertType2ItemPack(int iType);
	static int ConvertType2StateItemPack(int iType);
	void SetStartPos(int start){ m_iStartPos = start;}
	int GetStartPos(){ return m_iStartPos;}
private:
    //CGood*  m_pGoods;
    int m_iGridType;    //0-12代表装备栏的0-12格，13表示物品栏

    int		m_iCellWidth,m_iCellHeight;		//单元格的宽度和高度
    int		m_iWCells,m_iHCells;			//水平和垂直方向的格数
    int		m_iCellDisW,m_iCellDisH;		//单元格水平和垂直的间隔
    bool	m_bDClicked;					//是否是双击左键 

	DWORD	m_dwRightClickGoodID;			//右键物品
    static stGoodFrom sm_stGoodFrom;

	int		m_iStartPos;				//起始位置

	CGood * GetGood(int index);
	static void DrawGoodAddEffect(GoodAddEffect &goodEffect,int x,int y,LPPOS pScale = NULL);//绘制物品叠加特效
	static void SDrawGoodEffect(const char *strName,int iX,int iY,CGood &good,int dwClientLooks,LPPOS pScale = NULL,int iType = 1);
	int GetCellIndex(int iX,int iY);//根据坐标获得物品index
	void ClickAvatarGood(int x,int y);//单击了子娃娃界面的物品
	void ClickPackageGood(int pos,bool bClickBelt = false);//点击包裹及腰带上的物品
	void DBClickPackageGood(int pos,bool bDClickBelt = false);//双击包裹及腰带上的物品
	void ClickPetPackageGood(int pos);//点击宠物包裹的物品
	void DBClickPetPackageGood(int pos);//双击宠物包裹的物品
	void DrawBackColor(int x,int y);//画子娃娃界面物品格子的背景表示该物品能否在该位置装备
	void DrawGoodEffect(const char *strName,int iX,int iY,CGood &good,int dwClientLooks = 0,LPPOS pScale = NULL);//绘制物品特效
// 	void ClickTrusteeShipPackageGood(int index ,int iPkgIdx);//单击了托管物品栏

	
};


