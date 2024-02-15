using System;
using System.Collections.Generic;
using TeamD.Enum;
using UnityEngine;
/// <summary>
/// 統計管理クラス
/// </summary>
public class StatsManager : MonoBehaviour
{
    /// <summary>一定のクッキーを所持した状態で転生した際に増やす値</summary>
    public int HeavenlyChipStats { get; set; }
    /// <summary>実績</summary>
    public Achievement AchievementStats { get; set; }

    /// <summary>施設のアップグレード状況と数</summary>
    public Dictionary<FactoryKey, (UpgradeTier Tier, int Amount)> FactoryStats { get; set; } = new();
}