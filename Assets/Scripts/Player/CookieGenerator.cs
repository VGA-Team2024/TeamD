using System;
using System.Threading;
using Cysharp.Threading.Tasks;
using UnityEngine;

public class CookieGenerator : MonoBehaviour
{
    private const int GoldenCookieAddValue = 7;
    private readonly PlayerManager _playerManager = PlayerManager.Instance;

    private void Start()
    {
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
                foreach (var item in _playerManager.AutoGeneratorDictionary.Values)
                {
                    _playerManager.AddCookie((item.BaseGeneratorValue * (1 << (int)item.BasePower)) * GoldenCookieAddValue);
                }
            }
            else
            {
                foreach (var item in _playerManager.AutoGeneratorDictionary.Values)
                {
                    _playerManager.AddCookie(item.BaseGeneratorValue * (1 << (int)item.BasePower));
                }
            }

            await UniTask.Delay(TimeSpan.FromSeconds(1), cancellationToken: ct);
        }
    }

    /// <summary>
    /// リソース手動生産メソッド
    /// </summary>
    public void ManualGenerate()
    {
        _playerManager.AddCookie(_playerManager.ManualGenerateCount.BaseGeneratorValue * (1 << (int)_playerManager.ManualGenerateCount.BasePower));
    }
}