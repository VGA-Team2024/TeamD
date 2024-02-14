using System;
using System.Threading;
using Cysharp.Threading.Tasks;
using UnityEngine;

public class CookieGenerator : MonoBehaviour
{
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
    public async UniTaskVoid AutoGenerate(CancellationToken ct)
    {
        while (true)
        {
            foreach (var item in _playerManager.AutoGeneratorDictionary.Values)
            {
                _playerManager.AddCookie(item.BaseGeneratorValue * item.BaseScale);
            }
            await UniTask.Delay(TimeSpan.FromSeconds(1), cancellationToken: ct);
        }
    }

    /// <summary>
    /// リソース手動生産メソッド
    /// </summary>
    public void ManualGenerate()
    {
        _playerManager.AddCookie(_playerManager.ManualGenerateCount.BaseGeneratorValue * _playerManager.ManualGenerateCount.BaseScale);
    }
}