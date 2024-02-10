using System;

[Serializable]
public struct LargeNumber
{
    public long BaseValue;
    public int Multiplier;

    public LargeNumber(long baseValue, int multiplier)
    {
        this.BaseValue = baseValue;
        this.Multiplier = multiplier;
    }

    // 数値を文字列として表示するための簡単なメソッド
    public override string ToString()
    {
        return $"{BaseValue}e{Multiplier}";
    }

    // 二つのLargeNumberを加算する静的メソッド
    public static LargeNumber Add(LargeNumber a, LargeNumber b)
    {
        if (a.Multiplier > b.Multiplier)
        {
            long scaleDifference = a.Multiplier - b.Multiplier;
            long scaledBaseValue = b.BaseValue * (long)Math.Pow(10, -scaleDifference);
            return new LargeNumber(a.BaseValue + scaledBaseValue, a.Multiplier);
        }
        else if (a.Multiplier < b.Multiplier)
        {
            long scaleDifference = b.Multiplier - a.Multiplier;
            long scaledBaseValue = a.BaseValue * (long)Math.Pow(10, -scaleDifference);
            return new LargeNumber(b.BaseValue + scaledBaseValue, b.Multiplier);
        }
        else
        {
            return new LargeNumber(a.BaseValue + b.BaseValue, a.Multiplier);
        }
    }

    // LargeNumberの値をスケーリングするメソッド
    public void Scale(int scale)
    {
        this.Multiplier += scale;
    }
}
