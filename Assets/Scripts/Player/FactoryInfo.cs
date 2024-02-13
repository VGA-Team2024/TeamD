public struct FactoryInfo
{
    public int BaseId;
    public LargeNumber BaseGeneratorValue;
    public double BaseScale;

    public FactoryInfo(int id, LargeNumber value, double scale)
    {
        BaseId = id;
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