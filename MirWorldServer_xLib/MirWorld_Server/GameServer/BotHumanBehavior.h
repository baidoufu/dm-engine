#pragma once
#include <random>
#include <vector>
#include <algorithm>

enum e_randomType //随机类型
{
	RANDOM_OI, // 开区间
	RANDOM_CI, // 闭区间
};

// ============================================================================
// 人类行为模拟器 - 提供各种模拟真人行为的工具函数
// 用于让机器人的操作模式更接近真实玩家，规避检测
// ============================================================================
class CBotHumanBehavior
{
private:
	// 线程安全的随机数生成器（每线程独立引擎，避免rand()的数据竞争）
	static std::mt19937& GetRng()
	{
		thread_local std::mt19937 s_rng(std::random_device{}());
		return s_rng;
	}

public:
	// 随机数范围生成 - 默认闭区间
	static int RandomRange(int nMax, e_randomType bType = RANDOM_CI)
	{
		if (nMax <= 0) return 0;  // 防止除零崩溃
		if (bType == RANDOM_OI)
			return (GetRng()() % (nMax));
		else if (bType == RANDOM_CI)
			return (GetRng()() % (nMax + 1));
		return 0;
	}

	// 随机数范围生成 - 默认闭区间
	static int RandomRange(int nMin, int nMax, e_randomType bType = RANDOM_CI)
	{
		if (nMin >= nMax) return nMin;
		if (bType == RANDOM_OI)
			return nMin + (GetRng()() % (nMax - nMin));
		else if (bType == RANDOM_CI)
			return nMin + (GetRng()() % (nMax - nMin + 1));
		return 0;
	}

	// 百分比概率判定
	static BOOL RandomPercent(DWORD dwPercent)
	{
		if (dwPercent >= 100) return TRUE;
		if (dwPercent == 0) return FALSE;
		return (GetRng()() % 100) < (int)dwPercent;
	}

	// 生成带正态分布倾向的随机延迟
	// 使用Box-Muller变换近似正态分布
	static DWORD GenerateReactionTime(DWORD dwBaseTime)
	{
		// 生成两个均匀分布随机数
		// 使用std::uniform_real_distribution生成[0,1)均匀分布，线程安全
		double u1 = std::uniform_real_distribution<double>(0.0, 1.0)(GetRng());
		double u2 = std::uniform_real_distribution<double>(0.0, 1.0)(GetRng());

		// Box-Muller变换
		double z = sqrt(-2.0 * log(u1)) * cos(2.0 * 3.14159265358979323846 * u2);

		// 基础时间 + 标准差 * 随机偏移
		DWORD dwVariance = dwBaseTime / 3;
		DWORD dwResult = (DWORD)(dwBaseTime + dwVariance * z);

		// 限制范围在50ms到2000ms之间
		if (dwResult < 50) dwResult = 50;
		if (dwResult > 2000) dwResult = 2000;
		return dwResult;
	}

	// 操作频率限制检查
	static BOOL CheckOperationFrequency(DWORD& dwCount, DWORD dwMaxPerSecond)
	{
		return dwCount < dwMaxPerSecond;
	}

	// 是否应该发呆
	static BOOL ShouldIdle(DWORD dwIdleChance)
	{
		return RandomPercent(dwIdleChance);
	}

	// 是否应该聊天
	static BOOL ShouldChat(DWORD dwChatChance)
	{
		return RandomPercent(dwChatChance);
	}

	// 随机聊天内容生成
	static const char* GenerateChatMessage()
	{
		static const char* messages[] = {
			"大家好",
			"有人在吗",
			"今天运气不错",
			"这个怪好难打",
			"有没有组队的",
			"求带",
			"刷装备去了",
			"有人卖装备吗",
			"这地图人好少",
			"升级好慢啊",
			"谁有多的药",
			"来个道士加血",
		};
		int nCount = sizeof(messages) / sizeof(messages[0]);
		return messages[RandomRange(nCount, RANDOM_OI)];
	}

	// 移动路径随机化 - 在路径点中添加随机偏差
	static void RandomizePath(std::vector<POINT>& path)
	{
		for (auto& pt : path)
		{
			if (RandomPercent(20))  // 20%概率添加偏差
			{
				pt.x += RandomRange(-1, 1);
				pt.y += RandomRange(-1, 1);
			}
		}
	}

	// 生成随机思考间隔（在基础值上添加±20%偏差）
	static DWORD RandomizeThinkInterval(DWORD dwBaseInterval)
	{
		DWORD dwVariance = dwBaseInterval * 20 / 100;
		DWORD dwResult = dwBaseInterval + RandomRange(-(int)dwVariance, (int)dwVariance);
		if (dwResult < 100) dwResult = 100;  // 最小100ms
		return dwResult;
	}

	// 生成攻击间隔（模拟人类攻击节奏）
	static DWORD GenerateAttackInterval()
	{
		// 基础800ms，随机偏差0-400ms
		return 800 + RandomRange(400);
	}

	// 是否犹豫（模拟人类操作犹豫）
	static BOOL ShouldHesitate(DWORD dwChance = 10)
	{
		return RandomPercent(dwChance);
	}

	// 生成随机巡逻方向（8方向）
	static int RandomDirection8()
	{
		return RandomRange(7);
	}

	// 生成随机巡逻步数
	static int RandomPatrolSteps()
	{
		return RandomRange(3, 8);
	}

	// 生成随机发呆持续时间
	static DWORD RandomIdleDuration()
	{
		// 1-5秒的随机发呆
		return RandomRange(1000, 5000);
	}

	// 生成随机休息持续时间
	static DWORD RandomRestDuration()
	{
		// 3-10秒的随机休息
		return RandomRange(3000, 10000);
	}

	// 计算两点之间的8方向
	static int GetDirection8(int iTX, int iTY, int iOX, int iOY)
	{
		int x1 = iTX - iOX;
		int y1 = -(iTY - iOY);

		if (x1 == 0)
		{
			if (y1 > 0) return 0;
			else return 4;
		}
		if (y1 == 0)
		{
			if (x1 > 0) return 2;
			else return 6;
		}

		int j = abs((int)((float)y1 / (float)x1 * 1000));
		if (x1 > 0)
		{
			if (y1 > 0)
			{
				if (j < 414) return 2;
				else if (j < 2414) return 1;
				else return 0;
			}
			else
			{
				if (j < 414) return 2;
				else if (j < 2414) return 3;
				else return 4;
			}
		}
		else
		{
			if (y1 > 0)
			{
				if (j < 414) return 6;
				else if (j < 2414) return 7;
				else return 0;
			}
			else
			{
				if (j < 414) return 6;
				else if (j < 2414) return 5;
				else return 4;
			}
		}
	}

	// 计算两点之间的曼哈顿距离
	static int Distance(int x1, int y1, int x2, int y2)
	{
		return abs(x1 - x2) + abs(y1 - y2);
	}

	// 计算两点之间的切比雪夫距离（8方向移动距离）
	static int Distance8(int x1, int y1, int x2, int y2)
	{
		int dx = abs(x1 - x2);
		int dy = abs(y1 - y2);
		return (dx > dy) ? dx : dy;
	}
};
