///////////////////////////////////////////////////////////////////////
//文件名：   GlobalErr.h
//文件说明：全局游戏中错误类型定义
///////////////////////////////////////////////////////////////////////
// Game Error Message

#define	G_OK						0x00000001			// 成功
#define G_ERROR						0x00000000			// 错误
#define G_ERROR_UNKNOWN				0x00000002			// 未知错误

#define	G_CREATEWINDOW_FAIL			0x00000100			// 创建窗口失败
#define G_CREATETEXUTRE_FAIL		0x00000101			// 创建纹理失败
#define G_LOCKRECT_FAIL				0x00000102			// 锁定纹理失败
#define G_CREATEFONT_FAIL			0x00000103			// 创建字体失败
#define G_DRAWTEXT_FAIL				0x00000104			// 显示文字失败
#define G_GETBACKBUFFER_FAIL		0x00000105			// 得到后表面失败
#define G_CREATEIMAGESURFACE_FAIL	0x00000106			// 得到内存表面失败
#define G_FILENOTEXIST				0x00000200			// 文件不存在
#define G_FILEWRITE_FAIL			0x00000201			// 写文件失败
#define G_TGA_TYPEUNKNOW			0x00000300			// 未知的TGA文件格式
#define G_TGA_NOCOLORPALETTE		0x00000301			// 我们不处理带调色板的TGA文件
#define G_TGA_SIZENOTMATCH			0x00000302			// 错误的TGA文件格式
#define G_DD_ERROR					0x00000303			// DirectDraw error

#define G_EMPTY_CG					0x00001000			// 没有定义g_pCG

#define ERROR_CODE_NONE				0x00000000
#define ERROR_CODE_OUTOFMEMORY		0x00001001
#define ERROR_CODE_FILE_OPEN		0x00001002
#define ERROR_CODE_FILE_READ		0x00001003
#define	ERROR_CODE_OUTOFVIDEOMEMORY	0x00001004
#define ERROR_CODE_TEXTURE			0x00001005
