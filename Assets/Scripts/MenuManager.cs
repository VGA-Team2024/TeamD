using UnityEngine;
/// <summary>
/// メニュー管理。UIをInstantiateするための親オブジェクトを持っている。
/// </summary>
public class MenuManager : SingletonBase<MenuManager>
{
    [SerializeField] Transform _menuParent;
    public Transform MenuParent => _menuParent;
    void Awake()
    {
        SetInstance();
    }
}
