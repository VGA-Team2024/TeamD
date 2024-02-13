using System.Collections.Generic;

public class PlayerManager
{
    /// <summary>所持リソース数管理変数</summary>
    public LargeNumber PlayerResources { get; private set; } = new(0, 0);

    private static PlayerManager _instance = null;
    
    public SortedDictionary<int, FactoryInfo> AutoGeneratorDictionary { get; private set; } = new();

    public FactoryInfo ManualGenerateCount = new();

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

    public void AddAutoGenerateCount(FactoryInfo info)
    {
        AutoGeneratorDictionary.Add(info.BaseId, info);
    }
}