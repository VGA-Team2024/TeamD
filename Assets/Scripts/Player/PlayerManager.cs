using System.Collections.Generic;

public class PlayerManager
{
    /// <summary>所持リソース数管理変数</summary>
    public LargeNumber PlayerResources { get; private set; } = new(0, 0);

    private static PlayerManager _instance = null;
    
    public SortedDictionary<string, FactoryInfo> AutoGeneratorDictionary { get; private set; } = new();

    public FactoryInfo ManualGenerateCount = new("manual", new(1, 0), 1);

    private PlayerManager()
    {
    }

    public static PlayerManager Instance
    {
        get
        {
            if (_instance == null)
            {
                _instance = new PlayerManager();
            }

            return _instance;
        }
    }

    public void AddResource(LargeNumber value)
    {
        PlayerResources = LargeNumber.Add(PlayerResources, value);
    }

    public void SubtractResource(LargeNumber value)
    {
        PlayerResources = LargeNumber.Subtract(PlayerResources, value);
    }

    /// <summary>
    /// 自動生成数を増やす。増やす予定の値をもとの値に足して新たにFactoryInfoを作成し、Dictionaryに登録する。
    /// </summary>
    /// <param name="name"></param>
    /// <param name="value"></param>
    public void AddAutoGenerateCount(string name, LargeNumber value)
    {
        var newfactoryInfo = new FactoryInfo(name, LargeNumber.Add(AutoGeneratorDictionary[name].BaseGeneratorValue, value), AutoGeneratorDictionary[name].BaseScale);
        AutoGeneratorDictionary[name] = newfactoryInfo;
    }
}