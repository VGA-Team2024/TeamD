using System;
using System.Linq;
using System.Threading;
using Cysharp.Threading.Tasks;
using TeamD.Enum;
using UnityEngine;

public class CookieGenerator : MonoBehaviour
{
    private const int GoldenCookieAddValue = 7;
    private readonly PlayerManager _playerManager = PlayerManager.Instance;
    Factories _factories;

    private void Start()
    {
        _factories = Resources.Load<Factories>("Excel/Factories");
        var ct = this.GetCancellationTokenOnDestroy();
        AutoGenerate(ct).Forget();
    }

    // TODO 一応自動用と手動用で分けているが、統一出来そうな気もする

    /// <summary>
    /// リソース自動生産メソッド
    /// </summary>
    private async UniTaskVoid AutoGenerate(CancellationToken ct)
    {
        while (true)
        {
            if (_playerManager.IsGoldenCookieMode)
            {
                _playerManager.AddCookie(StatsManager.FactoryStats
                    .Select(x => _factories.Entities
                        .Find(e => e.Key == x.Key).CpS * x.Value.Amount * (1 << (int)x.Value.Tier))
                    .Sum() * GoldenCookieAddValue);
            }
            else
            {
                _playerManager.AddCookie(StatsManager.FactoryStats
                    .Select(x => _factories.Entities
                        .Find(e => e.Key == x.Key).CpS * x.Value.Amount * (1 << (int)x.Value.Tier))
                    .Sum());
            }

            await UniTask.Delay(TimeSpan.FromSeconds(1), cancellationToken: ct);
        }
    }

    /// <summary>
    /// リソース手動生産メソッド
    /// </summary>
    public void ManualGenerate()
    {
        _playerManager.AddCookie(_playerManager.ManualGenerateCount * (1 << (int)StatsManager.FactoryStats[FactoryKey.Cursor].Tier));
    }
}