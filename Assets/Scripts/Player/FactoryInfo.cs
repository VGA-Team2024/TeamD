/// <summary>
/// 施設の生成情報の構造体
/// どの施設か、基本の生成数、生成量にかかっている倍率
/// を持つ
/// </summary>
public struct FactoryInfo
{
    public string Name;
    public LargeNumber BaseGeneratorValue;
    /// <summary>2の累乗の指数の数</summary>
    public double BasePower;

    public FactoryInfo(string name, LargeNumber value, double power)
    {
        Name = name;
        BaseGeneratorValue = value;
        BasePower = power;
    }

    public void AddValue(LargeNumber value)
    {
        BaseGeneratorValue = LargeNumber.Add(BaseGeneratorValue, value);
    }

    public void AddPower(double power)
    {
        BasePower += power;
    }
}