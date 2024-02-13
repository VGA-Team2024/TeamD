using System;

[Serializable]
public struct LargeNumber
{
    public double BaseValue;
    public int Multiplier;

    public LargeNumber(double baseValue, int multiplier)
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
            int scaleDifference = a.Multiplier - b.Multiplier;
            double scaledBaseValue = b.BaseValue * Math.Pow(10, -scaleDifference);
            return new LargeNumber(a.BaseValue + scaledBaseValue, a.Multiplier);
        }
        else if (a.Multiplier < b.Multiplier)
        {
            int scaleDifference = b.Multiplier - a.Multiplier;
            double scaledBaseValue = a.BaseValue * Math.Pow(10, -scaleDifference);
            return new LargeNumber(b.BaseValue + scaledBaseValue, b.Multiplier);
        }
        else
        {
            return new LargeNumber(a.BaseValue + b.BaseValue, a.Multiplier);
        }
    }

    // 減算メソッド
    public static LargeNumber Subtract(LargeNumber a, LargeNumber b)
    {
        if (a.Multiplier > b.Multiplier)
        {
            int scaleDifference = a.Multiplier - b.Multiplier;
            double scaledBaseValue = b.BaseValue * Math.Pow(10, -scaleDifference);
            return new LargeNumber(a.BaseValue - scaledBaseValue, a.Multiplier);
        }
        else if (a.Multiplier < b.Multiplier)
        {
            int scaleDifference = b.Multiplier - a.Multiplier;
            double scaledBaseValue = a.BaseValue * Math.Pow(10, -scaleDifference);
            return new LargeNumber(b.BaseValue - scaledBaseValue, b.Multiplier);
        }
        else
        {
            return new LargeNumber(a.BaseValue - b.BaseValue, a.Multiplier);
        }
    }

    // LargeNumberの値をスケーリングするメソッド
    public void Scale(int scale)
    {
        this.Multiplier += scale;
    }

    // * 演算子のオーバーロード
    public static LargeNumber operator *(LargeNumber a, double b)
    {
        return a.Multiply(b);
    }

    // 乗算メソッド（以前の例と同じ）
    public LargeNumber Multiply(double multiplier)
    {
        double result = BaseValue * multiplier;
        int resultMultiplier = Multiplier;

        while (result >= 10)
        {
            result /= 10;
            resultMultiplier++;
        }

        while (result > 0 && result < 1)
        {
            result *= 10;
            resultMultiplier--;
        }

        return new LargeNumber(result, resultMultiplier);
    }
}
