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