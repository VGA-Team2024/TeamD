using System;
using System.Linq;
using TeamD.Enum;
using UniRx;


namespace Editor.EditorClicker.Data
{
    /// <summary>
    /// ユーザのデータ
    /// </summary>
    [Serializable]
    public class UserData
    {
        // 今持ってるクッキーの数
        PlayerManager playerManager = PlayerManager.Instance;
        // 解放している施設やアップグレードの数
        ReactiveDictionary<FactoryKey, (UpgradeTier Tier, int Amount)> CurrentFactories { get; }
            = new(Enum.GetValues(typeof(FactoryKey)).Cast<FactoryKey>().ToDictionary(e => e, _ => (UpgradeTier.NoUpgrade, 0)));
    }
}