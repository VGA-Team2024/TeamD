using UnityEngine;

public class ResourceGenerator : MonoBehaviour
{
    private readonly PlayerManager _playerManager = PlayerManager.Instance;
    
    private void FixedUpdate()
    {
        AutoGenerate();
    }
    
    // TODO 一応自動用と手動用で分けているが、統一出来そうな気もする

    /// <summary>
    /// リソース自動生産メソッド
    /// </summary>
    public void AutoGenerate()
    {
        //_playerManager.AddResource(_playerManager.AutoGenerateCount);
        foreach (var item in _playerManager.AutoGeneratorDictionary.Values)
        {
            _playerManager.AddResource(item.BaseGeneratorValue * item.BaseScale);
        }
    }

    /// <summary>
    /// リソース手動生産メソッド
    /// </summary>
    public void ManualGenerate()
    {
        _playerManager.AddResource(_playerManager.ManualGenerateCount.BaseGeneratorValue * _playerManager.ManualGenerateCount.BaseScale);
    }
}