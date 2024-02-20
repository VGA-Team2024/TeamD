/// <summary>
/// 施設の生成情報の構造体
/// どの施設か、基本の生成数、生成量にかかっている倍率
/// を持つ
/// </summary>
public struct FactoryInfo
{
    public string Name;
    public double BaseGeneratorValue;
    /// <summary>2の累乗の指数の数</summary>
    public double BasePower;

    public FactoryInfo(string name, double value, double power)
    {
        Name = name;
        BaseGeneratorValue = value;
        BasePower = power;
    }

    public void AddValue(double value)
    {
        BaseGeneratorValue += value;
    }

    public void AddPower(double power)
    {
        BasePower += power;
    }
}