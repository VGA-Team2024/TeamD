public class PlayerManager
{
    /// <summary>所持リソース数管理変数</summary>
    public LargeNumber PlayerResources { get; private set; }

    /// <summary>自動生成量格納変数</summary>
    public LargeNumber AutoGenerateCount { get; private set; }

    /// <summary>手動生成量格納変数</summary>
    public LargeNumber ManualGenerateCount { get; private set; }

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
    
    public void AddResource(LargeNumber value)
    {
        PlayerResources = LargeNumber.Add(PlayerResources, value);
    }

    public void SubtractResource(LargeNumber value)
    {
        PlayerResources = LargeNumber.Subtract(PlayerResources, value);
    }

    public void AddAutoGenerateCount(LargeNumber value)
    {
        AutoGenerateCount = LargeNumber.Add(AutoGenerateCount, value);
    }

    public void AddManualGenerateCount(LargeNumber value)
    {
        ManualGenerateCount = LargeNumber.Add(ManualGenerateCount, value);
    }
}