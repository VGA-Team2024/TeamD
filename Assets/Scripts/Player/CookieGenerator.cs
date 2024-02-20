using System.Linq;
using System.Text;
using System.Threading;
using Cysharp.Threading.Tasks;
using TeamD.Enum;
using UniRx;
using UnityEngine;

public class CookieGenerator : MonoBehaviour
{
    [SerializeField, Tooltip("デバッグログ表示")] bool _activeDebugLog = true;
    private const int GoldenCookieAddValue = 7;
    private readonly PlayerManager _playerManager = PlayerManager.Instance;
    Factories _factories;
    FloatReactiveProperty _currentCpS = new();

    private void Start()
    {
        _factories = Resources.Load<Factories>("Excel/Factories");
        var ct = this.GetCancellationTokenOnDestroy();
        AutoGenerate(ct).Forget();
        _currentCpS.SkipLatestValueOnSubscribe().Subscribe(_=>CurrentCpSView()).AddTo(this);
    }

    private void CurrentCpSView()
    {
        if (!_activeDebugLog) return;
        StringBuilder sb = new StringBuilder().Append("現在のCpS: ").Append(_currentCpS.Value);
        print(sb.ToString());
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
                var cps = StatsManager.CpS * GoldenCookieAddValue;
                _currentCpS.Value = (float)cps;
                _playerManager.AddCookie(cps * Time.deltaTime);
            }
            else
            {
                var cps = StatsManager.CpS;
                _currentCpS.Value = (float)cps;
                _playerManager.AddCookie(cps * Time.deltaTime);
            }
            //  Updateメソッドと同じ周期
            await UniTask.Yield(cancellationToken: ct);
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