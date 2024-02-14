using System;

/// <summary>
/// 大きな数値を扱うための構造体
/// 例えば「1000」は「BaseValue=1, Multiplier=3」または「BaseValue=1000, Multiplier=0」として表現される
/// </summary>
[Serializable]
public struct LargeNumber : IComparable<LargeNumber>, IEquatable<LargeNumber>
{
    /// <summary>
    /// 値がいくつかを示す基底値
    /// </summary>
    public double BaseValue;

    /// <summary>
    /// 基底値の桁数を示す乗数。10の乗数を表す。
    /// </summary>
    public int Multiplier;
    public const double MaxBaseValue = double.MaxValue;
    public const double MinBaseValue = double.MinValue;

    public LargeNumber(double baseValue, int multiplier)
    {
        AdjustValues(ref baseValue, ref multiplier);

        this.BaseValue = baseValue;
        this.Multiplier = multiplier;
    }

    private static void AdjustValues(ref double baseValue, ref int multiplier)
    {
        while (baseValue > MaxBaseValue)
        {
            baseValue /= 10;
            multiplier++;
        }

        while (baseValue < MinBaseValue)
        {
            baseValue *= 10;
            multiplier--;
        }
    }

    // 数値を文字列として表示するための簡単なメソッド
    public override string ToString()
    {
        if (Multiplier == 0)
        {
            return BaseValue.ToString("F2");
        }
        return $"{BaseValue:0.##}e{Multiplier}";
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

        // 有効桁数が超える場合、Multiplierを調整
        while (Math.Abs(result) >= double.MaxValue)
        {
            result /= 10;
            resultMultiplier++;
        }

        while (Math.Abs(result) > 0 && Math.Abs(result) < 1)
        {
            result *= 10;
            resultMultiplier--;
        }

        return new LargeNumber(result, resultMultiplier);
    }

    public int CompareTo(LargeNumber other)
    {
        // 基底値が異なる場合、基底値を比較
        if (BaseValue != other.BaseValue)
        {
            return BaseValue.CompareTo(other.BaseValue);
        }
        // 基底値が同じ場合、乗数を比較
        return Multiplier.CompareTo(other.Multiplier);
    }

    public bool Equals(LargeNumber other)
    {
        return BaseValue == other.BaseValue && Multiplier == other.Multiplier;
    }

    public override bool Equals(object obj)
    {
        if (obj is LargeNumber)
        {
            return Equals((LargeNumber)obj);
        }
        return false;
    }

    public override int GetHashCode()
    {
        return HashCode.Combine(BaseValue, Multiplier);
    }

    public static bool operator <=(LargeNumber a, LargeNumber b)
    {
        return a.CompareTo(b) <= 0;
    }
    
    public static bool operator >=(LargeNumber a, LargeNumber b)
    {
        return a.CompareTo(b) >= 0;
    }

}
