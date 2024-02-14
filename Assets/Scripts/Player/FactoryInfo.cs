/// <summary>
/// 施設の生成情報の構造体
/// どの施設か、基本の生成数、生成量にかかっている倍率
/// を持つ
/// </summary>
public struct FactoryInfo
{
    public string Name;
    public LargeNumber BaseGeneratorValue;
    public double BaseScale;

    public FactoryInfo(string name, LargeNumber value, double scale)
    {
        Name = name;
        BaseGeneratorValue = value;
        BaseScale = scale;
    }

    public void AddValue(LargeNumber value)
    {
        BaseGeneratorValue = LargeNumber.Add(BaseGeneratorValue, value);
    }

    public void AddScale(double scale)
    {
        BaseScale += scale;
    }
}