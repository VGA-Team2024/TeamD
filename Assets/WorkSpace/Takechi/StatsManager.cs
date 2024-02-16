using System.Collections.Generic;
using TeamD.Enum;
/// <summary>
/// 統計管理クラス
/// </summary>
public static class StatsManager
{
    /// <summary>一定のクッキーを所持した状態で転生した際に増やす値</summary>
    public static int HeavenlyChipStats { get; set; }
    /// <summary>保有実績</summary>
    public static Achievement AchievementStats { get; set; }

    /// <summary>施設のアップグレード状況と数</summary>
    public static Dictionary<FactoryKey, (UpgradeTier Tier, int Amount)> FactoryStats { get; set; } = new();
}