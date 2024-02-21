using System;
using TeamD.Enum;
using UnityEngine;

[Serializable]
public class NothingCondition : ICondition
{
    public bool CheckCondition() => true;
}
/// <summary>
/// 施設の数で条件判定するクラス
/// </summary>
[Serializable]
public class FactoryAmountCondition : ICondition
{
    [Header("施設の数で条件判定")]
    [SerializeField, Tooltip("施設のEnum")] FactoryKey _factoryType;
    [SerializeField, Tooltip("比較に使う演算子")] CompareOptions _compareOptions;
    [SerializeField, Tooltip("右項")] int _rightNumber;
    public bool CheckCondition()
    {
        return CompareUtils.Compare(_compareOptions, StatsManager.CurrentFactories[_factoryType].Amount, _rightNumber);
    } 
}
/// <summary>
/// 保有実績で条件判定するクラス
/// </summary>
[Serializable]
public class AchievementCondition : ICondition
{
    [Header("保有実績で条件判定")]
    [SerializeField, Tooltip("要求実績")] Achievement _requiredAchievement;
    public bool CheckCondition()
    {
        return (StatsManager.Achievements & _requiredAchievement) == _requiredAchievement;
    }
}
/// <summary>
/// 施設のアップグレード状況で条件判定するクラス
/// </summary>
[Serializable]
public class FactoryUpgradeTierCondition : ICondition
{
    [Header("施設のアップグレード状況で条件判定")]
    [SerializeField, Tooltip("施設のEnum")] FactoryKey _factoryType;
    [SerializeField, Tooltip("比較に使う演算子")] CompareOptions _compareOptions;
    [SerializeField, Tooltip("右項")] UpgradeTier _rightUpgradeTier;
    public bool CheckCondition()
    {
        return CompareUtils.Compare(_compareOptions, (int)StatsManager.CurrentFactories[_factoryType].Tier, (int)_rightUpgradeTier);
    }
}
/// <summary>
/// 施設の売却数で条件判定するクラス
/// </summary>
[Serializable]
public class FactorySellCountCondition : ICondition
{
    [Header("施設の売却数で条件判定")]
    [SerializeField, Tooltip("施設のEnum")] FactoryKey _factoryType;
    [SerializeField, Tooltip("比較に使う演算子")] CompareOptions _compareOptions;
    [SerializeField, Tooltip("右項")] int _rightNumber;
    public bool CheckCondition()
    {
        return CompareUtils.Compare(_compareOptions, StatsManager.FactorySellCount[_factoryType], _rightNumber);
    }
}
/// <summary>
/// 転生回数で条件判定するクラス
/// </summary>
[Serializable]
public class ReincarnationCountCondition : ICondition
{
    [Header("転生回数で条件判定")]
    [SerializeField, Tooltip("比較に使う演算子")] CompareOptions _compareOptions;
    [SerializeField, Tooltip("右項")] int _rightNumber;
    public bool CheckCondition()
    {
        return CompareUtils.Compare(_compareOptions, StatsManager.ReincarnationCount, _rightNumber);
    }
}
/// <summary>
/// ゴールデンクッキー獲得回数で条件判定するクラス
/// </summary>
[Serializable]
public class GoldenCookieObtainCountCondition : ICondition
{
    [Header("ゴールデンクッキー獲得回数で条件判定")]
    [SerializeField, Tooltip("比較に使う演算子")] CompareOptions _compareOptions;
    [SerializeField, Tooltip("右項")] int _rightNumber;
    public bool CheckCondition()
    {
        return CompareUtils.Compare(_compareOptions, StatsManager.GoldenCookieObtainCount, _rightNumber);
    }
}
public static class CompareUtils
{
    public static bool Compare(CompareOptions compareOptions, int left, int right)
    {
        switch (compareOptions)
        {
            case CompareOptions.Equals :
                if (left != right) return false;
                break;
            case CompareOptions.NotEquals :
                if (left == right) return false;
                break;
            case CompareOptions.GreaterEqual :
                if (left < right) return false;
                break;
            case CompareOptions.GreaterThan :
                if (left <= right) return false;
                break;
            case CompareOptions.LessEqual :
                if (left > right) return false;
                break;
            case CompareOptions.LessThan :
                if (left >= right) return false;
                break;
        }
        return true;
    }
}