using System.Collections.Generic;
using TeamD.Enum;

public class PlayerManager
{
    private static PlayerManager _instance;
    /// <summary>所持リソース数管理変数</summary>
    public double CookieCount { get; set; }
    public int ManualGenerateCount = 1000;
    
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
    /// ゴールデンクッキー状態を切り替える
    /// </summary>
    public void ChangeGoldenCookieMode(bool flag)
    {
        IsGoldenCookieMode = flag;
    }
}