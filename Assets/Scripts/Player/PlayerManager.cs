public class PlayerManager
{
    /// <summary>所持リソース数管理変数</summary>
    public decimal PlayerResources { get; private set; }

    /// <summary>自動生成量格納変数</summary>
    public decimal AutoGenerateCount { get; private set; } = 1;

    /// <summary>手動生成量格納変数</summary>
    public decimal ManualGenerateCount { get; private set; } = 10000;

    private static PlayerManager _instance = null;

    private PlayerManager() { }
    
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

    public void AddResource(decimal resource)
    {
        PlayerResources += resource;
    }

    public void AddAutoGenerateCount(decimal value)
    {
        AutoGenerateCount += value;
    }

    public void AddManualGenerateCount(decimal value)
    {
        ManualGenerateCount += value;
    }
}