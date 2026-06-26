import { BTNode, BTNodeType, ConditionType, ActionType } from './btTypes';

// 内置示例行为树原始数据
export const SAMPLE_TREES: { name: string; label: string; xml: string }[] = [
  {
    name: 'warrior',
    label: '战士战斗行为树',
    xml: `<?xml version="1.0" encoding="GBK"?>
<BehaviorTree name="战士战斗行为树 - Warrior v2.0">
    <Selector name="战士主决策">
        <Sequence name="安全区恢复行为">
            <ConditionInSafeArea name="检测是否在安全区" />
            <Parallel name="安全区内多任务处理">
                <Sequence name="安全区回血">
                    <ConditionLowHP name="HP不足70%" percent="70" />
                    <Probability name="吃药犹豫(30%)" chance="30">
                        <ActionUsePotion name="使用HP药水" hpType="1" />
                    </Probability>
                </Sequence>
                <Sequence name="安全区回蓝">
                    <ConditionLowMP name="MP不足40%" percent="40" />
                    <ActionUsePotion name="使用MP药水" hpType="0" />
                </Sequence>
                <Probability name="巡逻概率(40%)" chance="40">
                    <ActionPatrol name="安全区内巡逻" />
                </Probability>
                <Sequence name="安全区社交">
                    <Probability name="聊天概率(60%)" chance="60">
                        <ActionChat name="随机聊天" />
                    </Probability>
                </Sequence>
            </Parallel>
        </Sequence>
        <Sequence name="背包已满处理">
            <ConditionBagFull name="背包是否已满" />
            <ActionUseItem name="使用回城卷" itemName="回城卷" />
        </Sequence>
        <Sequence name="紧急逃生策略">
            <ConditionLowHP name="HP低于20%濒死" percent="20" />
            <Parallel name="逃生多动作">
                <ActionFlee name="向安全方向逃跑" />
                <Probability name="传送概率(20%)" chance="20">
                    <ActionUseItem name="随机传送卷逃脱" itemName="随机传送卷" />
                </Probability>
            </Parallel>
        </Sequence>
        <Sequence name="战斗策略">
            <ConditionHasTarget name="检查是否有目标" />
            <ActionChangeAttackMode name="切换全体攻击模式" attackMode="1" />
            <Parallel name="战斗并行行为">
                <Sequence name="战斗主循环">
                    <ActionMoveToTarget name="移向目标" />
                    <Selector name="攻击方式选择">
                        <Sequence name="技能攻击尝试">
                            <Probability name="技能犹豫(10%)" chance="10">
                                <ActionUseSkill name="使用战士技能攻击" magicId="0" />
                            </Probability>
                        </Sequence>
                        <ActionAttack name="普通攻击" />
                    </Selector>
                </Sequence>
                <Sequence name="战斗补给">
                    <Sequence name="战斗喝红">
                        <ConditionLowHP name="HP低于50%" percent="50" />
                        <Probability name="立刻喝药概率(80%)" chance="80">
                            <ActionUsePotion name="战斗中喝HP药水" hpType="1" />
                        </Probability>
                    </Sequence>
                    <Sequence name="战斗喝蓝">
                        <ConditionLowMP name="MP低于30%" percent="30" />
                        <ActionUsePotion name="战斗中喝MP药水" hpType="0" />
                    </Sequence>
                </Sequence>
            </Parallel>
        </Sequence>
        <Sequence name="自由漫游策略">
            <ActionPickupItem name="拾取地上物品" />
            <Random name="闲逛随机行为">
                <ActionPatrol name="随机巡逻走动" />
                <ActionRest name="原地休息" duration="5000" />
                <ActionChat name="随机聊天" />
            </Random>
        </Sequence>
    </Selector>
</BehaviorTree>`,
  },
  {
    name: 'mage',
    label: '法师战斗行为树',
    xml: `<?xml version="1.0" encoding="GBK"?>
<BehaviorTree name="法师战斗行为树 - Mage v2.0">
    <Selector name="法师主决策">
        <Sequence name="安全区行为">
            <ConditionInSafeArea name="在安全区" />
            <Parallel name="安全区并行行为">
                <Sequence name="安全区回血">
                    <ConditionLowHP name="HP小于60%" percent="60" />
                    <ActionUsePotion name="喝HP药水" hpType="1" />
                </Sequence>
                <Sequence name="安全区回蓝">
                    <ConditionLowMP name="MP小于50%" percent="50" />
                    <ActionUsePotion name="喝MP药水" hpType="0" />
                </Sequence>
                <Probability name="安全区巡逻(30%)" chance="30">
                    <ActionPatrol name="安全区巡逻" />
                </Probability>
                <Probability name="安全区聊天(50%)" chance="50">
                    <ActionChat name="安全区聊天" />
                </Probability>
            </Parallel>
        </Sequence>
        <Sequence name="背包满处理">
            <ConditionBagFull name="背包满" />
            <ActionUseItem name="使用回城卷" itemName="回城卷" />
        </Sequence>
        <Sequence name="法师紧急逃生">
            <ConditionLowHP name="HP小于25%濒死" percent="25" />
            <Parallel name="逃生多动作">
                <Probability name="随机传送(40%)" chance="40">
                    <ActionUseItem name="随机传送逃跑" itemName="随机传送卷" />
                </Probability>
                <ActionFlee name="向安全方向逃跑" />
            </Parallel>
        </Sequence>
        <Sequence name="战斗策略">
            <ConditionHasTarget name="有目标" />
            <Sequence name="法师战斗流程">
                <ActionChangeAttackMode name="切换全体攻击" attackMode="1" />
                <Selector name="法师攻击方式选择">
                    <Sequence name="远程技能攻击">
                        <ActionUseSkill name="远程技能攻击" magicId="0" />
                        <Probability name="攻击后停顿(20%)" chance="20">
                            <ActionRest name="短暂停顿" duration="800" />
                        </Probability>
                    </Sequence>
                    <Sequence name="普通攻击">
                        <ActionAttack name="普通攻击" />
                    </Sequence>
                </Selector>
                <ActionMoveToTarget name="调整与目标距离" />
                <Parallel name="战斗补给并行">
                    <Sequence name="战斗喝红">
                        <ConditionLowHP name="HP小于45%" percent="45" />
                        <ActionUsePotion name="喝HP药水" hpType="1" />
                    </Sequence>
                    <Sequence name="战斗喝蓝">
                        <ConditionLowMP name="MP小于40%" percent="40" />
                        <ActionUsePotion name="喝MP药水" hpType="0" />
                    </Sequence>
                </Parallel>
            </Sequence>
        </Sequence>
        <Sequence name="自由漫游">
            <ActionPickupItem name="拾取物品" />
            <Random name="漫游随机行为">
                <ActionPatrol name="巡逻" />
                <ActionRest name="休息" duration="3000" />
                <ActionChat name="聊天" />
                <Sequence name="发呆">
                    <Probability name="发呆概率" chance="10" />
                </Sequence>
            </Random>
        </Sequence>
    </Selector>
</BehaviorTree>`,
  },
  {
    name: 'taoist',
    label: '道士战斗行为树',
    xml: `<?xml version="1.0" encoding="GBK"?>
<BehaviorTree name="道士战斗行为树 - Taoist v2.0">
    <Selector name="道士主决策">
        <Sequence name="安全区自动恢复">
            <ConditionInSafeArea name="在安全区" />
            <Parallel name="安全区行为">
                <Sequence name="回血">
                    <ConditionLowHP name="HP小于65%" percent="65" />
                    <ActionUsePotion name="喝HP" hpType="1" />
                </Sequence>
                <Sequence name="回蓝">
                    <ConditionLowMP name="MP小于35%" percent="35" />
                    <ActionUsePotion name="喝MP" hpType="0" />
                </Sequence>
                <Probability name="巡逻(25%)" chance="25">
                    <ActionPatrol name="安全区巡逻" />
                </Probability>
                <Probability name="聊天(65%)" chance="65">
                    <ActionChat name="安全区聊天" />
                </Probability>
            </Parallel>
        </Sequence>
        <Sequence name="背包已满">
            <ConditionBagFull name="包满" />
            <ActionUseItem name="回城" itemName="回城卷" />
        </Sequence>
        <Sequence name="紧急自救">
            <ConditionLowHP name="HP小于30%" percent="30" />
            <ActionFlee name="逃跑" />
        </Sequence>
        <Sequence name="战斗主流程">
            <ConditionHasTarget name="有目标" />
            <ActionChangeAttackMode name="切善恶模式" attackMode="0" />
            <Sequence name="战前MP检查">
                <ConditionLowMP name="MP小于50%" percent="50" />
                <ActionUsePotion name="战前喝蓝" hpType="0" />
            </Sequence>
            <Parallel name="战斗并行策略">
                <Sequence name="自我治疗">
                    <ConditionLowHP name="HP小于50%加血" percent="50" />
                    <ActionUseSkill name="使用治愈术" magicId="0" />
                </Sequence>
                <Sequence name="攻击序列">
                    <ActionUseSkill name="道术攻击" magicId="0" />
                    <Probability name="追加攻击(20%)" chance="20">
                        <ActionUseSkill name="追加攻击" magicId="0" />
                    </Probability>
                    <ActionMoveToTarget name="逼近目标" />
                </Sequence>
                <Sequence name="战斗补给">
                    <Selector name="补给选择">
                        <Sequence name="喝HP">
                            <ConditionLowHP name="HP小于40%" percent="40" />
                            <ActionUsePotion name="喝HP" hpType="1" />
                        </Sequence>
                        <Sequence name="喝MP">
                            <ConditionLowMP name="MP小于25%" percent="25" />
                            <ActionUsePotion name="喝MP" hpType="0" />
                        </Sequence>
                    </Selector>
                </Sequence>
            </Parallel>
        </Sequence>
        <Sequence name="自由行为">
            <ActionPickupItem name="拾取物品" />
            <Random name="随机行为">
                <ActionPatrol name="巡逻走动" />
                <ActionRest name="原地休息" duration="5000" />
                <ActionChat name="聊天" />
                <Sequence name="反转示例">
                    <Inverter name="反转安全区判断">
                        <ConditionInSafeArea name="不在安全区" />
                    </Inverter>
                    <ActionPatrol name="反向巡逻" />
                </Sequence>
            </Random>
        </Sequence>
    </Selector>
</BehaviorTree>`,
  },
  {
    name: 'merchant',
    label: '商人行为树',
    xml: `<?xml version="1.0" encoding="GBK"?>
<BehaviorTree name="商人社交行为树 - Merchant v2.0">
    <Selector name="商人主决策">
        <Sequence name="安全区行为">
            <ConditionInSafeArea name="在安全区" />
            <Parallel name="商人行为并行">
                <Sequence name="恢复HP">
                    <ConditionLowHP name="HP小于70%" percent="70" />
                    <ActionUsePotion name="喝HP" hpType="1" />
                </Sequence>
                <Sequence name="恢复MP">
                    <ConditionLowMP name="MP小于50%" percent="50" />
                    <ActionUsePotion name="喝MP" hpType="0" />
                </Sequence>
                <Probability name="聊天(80%)" chance="80">
                    <ActionChat name="喊话交易" />
                </Probability>
                <Probability name="巡逻(35%)" chance="35">
                    <ActionPatrol name="安全区巡逻" />
                </Probability>
                <Probability name="摆摊休息(40%)" chance="40">
                    <ActionRest name="摆摊中" duration="8000" />
                </Probability>
            </Parallel>
        </Sequence>
        <Sequence name="出城拾取">
            <Inverter name="不在安全区才继续">
                <ConditionInSafeArea name="检测安全区" />
            </Inverter>
            <Selector name="野外行为选择">
                <ActionPickupItem name="拾取材料" />
                <Sequence name="包满回城">
                    <ConditionBagFull name="背包已满" />
                    <ActionUseItem name="回城卷" itemName="回城卷" />
                </Sequence>
                <ActionPatrol name="野外巡逻" />
            </Selector>
        </Sequence>
        <Sequence name="主城恢复">
            <Sequence name="反复喝药">
                <DecoratorRepeat name="重复回血直到满" count="0">
                    <Sequence name="检查并喝药">
                        <ConditionLowHP name="HP小于90%" percent="90" />
                        <ActionUsePotion name="喝HP药水" hpType="1" />
                    </Sequence>
                </DecoratorRepeat>
                <DecoratorRepeat name="重复回蓝直到满" count="0">
                    <Sequence name="检查并喝蓝">
                        <ConditionLowMP name="MP小于80%" percent="80" />
                        <ActionUsePotion name="喝MP药水" hpType="0" />
                    </Sequence>
                </DecoratorRepeat>
            </Sequence>
            <ActionChangeAttackMode name="切换和平模式" attackMode="2" />
        </Sequence>
        <Probability name="发呆(15%)" chance="15">
            <ActionRest name="发呆中" duration="4000" />
        </Probability>
        <Random name="漫游随机">
            <ActionPatrol name="巡逻" />
            <ActionChat name="聊天" />
            <ActionRest name="休息" duration="6000" />
            <ActionPickupItem name="拾取" />
        </Random>
    </Selector>
</BehaviorTree>`,
  },
  {
    name: 'patrolguard',
    label: '巡逻守卫行为树',
    xml: `<?xml version="1.0" encoding="GBK"?>
<BehaviorTree name="巡逻守卫行为树 - PatrolGuard v2.0">
    <Selector name="守卫主决策">
        <Sequence name="紧急撤退">
            <ConditionLowHP name="HP小于15%" percent="15" />
            <Parallel name="撤退并行">
                <ActionFlee name="向安全区逃跑" />
                <Probability name="传送(30%)" chance="30">
                    <ActionUseItem name="随机传送" itemName="随机传送卷" />
                </Probability>
            </Parallel>
        </Sequence>
        <Sequence name="守卫战斗">
            <ConditionHasTarget name="有目标" />
            <Sequence name="战斗流程">
                <ActionChangeAttackMode name="切换攻击模式" attackMode="1" />
                <Parallel name="战斗动作并行">
                    <Sequence name="主攻序列">
                        <ActionMoveToTarget name="移向目标" />
                        <ActionUseSkill name="使用技能" magicId="0" />
                        <ActionAttack name="普通攻击" />
                    </Sequence>
                    <Sequence name="战斗回复">
                        <Selector name="回复选择">
                            <Sequence name="喝红">
                                <ConditionLowHP name="HP小于45%" percent="45" />
                                <ActionUsePotion name="喝HP" hpType="1" />
                            </Sequence>
                            <Sequence name="喝蓝">
                                <ConditionLowMP name="MP小于30%" percent="30" />
                                <ActionUsePotion name="喝MP" hpType="0" />
                            </Sequence>
                        </Selector>
                    </Sequence>
                </Parallel>
                <Probability name="战斗后停顿(50%)" chance="50">
                    <ActionRest name="战斗后喘息" duration="1500" />
                </Probability>
            </Sequence>
        </Sequence>
        <Sequence name="持续巡逻">
            <ActionPickupItem name="沿途拾取" />
            <DecoratorRepeat name="重复巡逻3次" count="3">
                <ActionPatrol name="迈出巡逻一步" />
            </DecoratorRepeat>
            <Probability name="巡逻后休息(60%)" chance="60">
                <ActionRest name="巡逻间歇休息" duration="3000" />
            </Probability>
        </Sequence>
        <Sequence name="社交互动">
            <Probability name="主动聊天(50%)" chance="50">
                <ActionChat name="与玩家聊天" />
            </Probability>
            <Probability name="发呆望风(30%)" chance="30">
                <ActionRest name="望风中" duration="4000" />
            </Probability>
        </Sequence>
        <ActionPatrol name="默认巡逻" />
    </Selector>
</BehaviorTree>`,
  },
  {
    name: 'fullfeatured',
    label: '全功能演示行为树',
    xml: `<?xml version="1.0" encoding="GBK"?>
<BehaviorTree name="全功能综合行为树 v4.0">
    <Selector name="【Selector】主选择器">
        <Sequence name="【Sequence】安全区逻辑">
            <ConditionInSafeArea name="检测是否在安全区" />
            <Parallel name="【Parallel】安全区内多任务">
                <Sequence name="安全区回血">
                    <ConditionLowHP name="HP不足70%" percent="70" />
                    <Probability name="吃药犹豫" chance="30">
                        <ActionUsePotion name="使用HP药水" hpType="1" />
                    </Probability>
                </Sequence>
                <Sequence name="安全区回蓝">
                    <ConditionLowMP name="MP不足40%" percent="40" />
                    <ActionUsePotion name="使用MP药水" hpType="0" />
                </Sequence>
                <Probability name="巡逻概率" chance="40">
                    <ActionPatrol name="安全区内巡逻" />
                </Probability>
                <Probability name="聊天概率" chance="60">
                    <ActionChat name="随机聊天" />
                </Probability>
            </Parallel>
        </Sequence>
        <Sequence name="【Sequence】背包已满处理">
            <ConditionBagFull name="背包是否已满" />
            <ActionUseItem name="使用回城卷" itemName="回城卷" />
        </Sequence>
        <Sequence name="【Sequence】紧急逃生">
            <ConditionLowHP name="HP低于20%濒死" percent="20" />
            <Parallel name="逃生多动作">
                <ActionFlee name="向安全方向逃跑" />
                <Probability name="传送概率" chance="20">
                    <ActionUseItem name="随机传送卷逃脱" itemName="随机传送卷" />
                </Probability>
            </Parallel>
        </Sequence>
        <Sequence name="【Sequence】战斗策略">
            <ConditionHasTarget name="检查是否有目标" />
            <ActionChangeAttackMode name="切换全体攻击" attackMode="1" />
            <Parallel name="【Parallel】战斗并行">
                <Sequence name="战斗主循环">
                    <ActionMoveToTarget name="移向目标" />
                    <Selector name="【Selector】攻击方式选择">
                        <Sequence name="技能攻击尝试">
                            <ConditionSkillReady name="技能是否就绪" />
                            <Probability name="技能犹豫" chance="10">
                                <ActionUseSkill name="使用技能攻击" magicId="0" />
                            </Probability>
                        </Sequence>
                        <ActionAttack name="普通攻击" />
                    </Selector>
                </Sequence>
                <Sequence name="战斗补给">
                    <Sequence name="战斗喝红">
                        <ConditionLowHP name="HP低于50%" percent="50" />
                        <Probability name="立刻喝药概率" chance="80">
                            <ActionUsePotion name="战斗中喝HP药水" hpType="1" />
                        </Probability>
                    </Sequence>
                    <Sequence name="战斗喝蓝">
                        <ConditionLowMP name="MP低于30%" percent="30" />
                        <ActionUsePotion name="战斗中喝MP药水" hpType="0" />
                    </Sequence>
                </Sequence>
            </Parallel>
        </Sequence>
        <Sequence name="【Sequence】自由漫游">
            <ActionPickupItem name="拾取地上物品" />
            <Random name="【Random】闲逛随机行为">
                <ActionPatrol name="随机巡逻走动" />
                <ActionRest name="原地休息" duration="5000" />
                <ActionChat name="随机聊天" />
                <DecoratorRepeat name="重复两次" count="2">
                    <ActionPatrol name="重复巡逻" />
                </DecoratorRepeat>
            </Random>
        </Sequence>
    </Selector>
</BehaviorTree>`,
  },
];

// 根据名称获取示例
export function getSampleTree(name: string): string | undefined {
  return SAMPLE_TREES.find((s) => s.name === name)?.xml;
}