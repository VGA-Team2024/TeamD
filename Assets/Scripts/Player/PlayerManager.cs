using System.Collections.Generic;

public class PlayerManager
{
    private static PlayerManager _instance;
    /// <summary>所持リソース数管理変数</summary>
    public double CookieCount { get; private set; }
    public SortedDictionary<string, FactoryInfo> AutoGeneratorDictionary { get; private set; } = new();
    public FactoryInfo ManualGenerateCount = new("manual", 1000, 0);
    
    private PlayerManager()
    {
    }

    public bool IsGoldenCookieMode { get; private set; }
    public static PlayerManager Instance
    {
        get { return _instance ??= new PlayerManager(); }
    }

    public void AddCookie(double value)
    {
        CookieCount += value;
    }

    public void SubtractCookie(double value)
    {
        CookieCount -= value;
    }

    /// <summary>
    /// 自動生成数を増やす。増やす予定の値をもとの値に足して新たにFactoryInfoを作成し、Dictionaryに登録する。
    /// </summary>
    /// <param name="name"></param>
    /// <param name="value"></param>
    public void AddAutoGenerateCount(string name, double value)
    {
        // ここで更新したFactoryInfoを生成し、Dictionaryに登録する。
                                         //どの施設か,        元の生成量に増やす予定の生成量を足したもの,                     生成量にかかっている倍率
        var newFactoryInfo = new FactoryInfo(name, AutoGeneratorDictionary[name].BaseGeneratorValue + value, AutoGeneratorDictionary[name].BasePower);
        AutoGeneratorDictionary[name] = newFactoryInfo;
    }
    
    /// <summary>
    /// 自動生成数を減らす。減らす予定の値をもとの値に足して新たにFactoryInfoを作成し、Dictionaryに登録する。
    /// </summary>
    public void SubtractGeneratorCount(string name, double value)
    {
        // ここで更新したFactoryInfoを生成し、Dictionaryに登録する。
        //どの施設か,        元の生成量に増やす予定の生成量を足したもの,                     生成量にかかっている倍率
        //AutoGeneratorDictionary[name] = new FactoryInfo(name, LargeNumber.Add(AutoGeneratorDictionary[name].BaseGeneratorValue, value), AutoGeneratorDictionary[name].BaseScale);
        AutoGeneratorDictionary[name] = new FactoryInfo(name, AutoGeneratorDictionary[name].BaseGeneratorValue - value, AutoGeneratorDictionary[name].BasePower);
    }

    /// <summary>
    /// ゴールデンクッキー状態を切り替える
    /// </summary>
    public void ChangeGoldenCookieMode(bool flag)
    {
        IsGoldenCookieMode = flag;
    }
}