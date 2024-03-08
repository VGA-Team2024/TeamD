using UnityEngine;
/// <summary>
/// MonoBehaviourSingletonの抽象クラス
/// </summary>
public abstract class SingletonBase<T> : MonoBehaviour where T : Component
{
    static T _instance;
    public static T Instance
    {
        get
        {
            if (_instance)
            {
                return _instance;
            }

            FindObjectOfType<SingletonBase<T>>().SetInstance();
            if (_instance)
            {
                return _instance;
            }

            Debug.LogError (nameof(T) + " is nothing");
            return _instance;
        }
    }
    protected void SetInstance()
    {
        if (_instance)
        {
            if(_instance != this) Destroy(gameObject);
        }
        else
        {
            _instance = this as T;
            //  何かの子オブジェクトになっている場合はDontDestroyOnLoadできないので実行時にはParentを解除する
            transform.SetParent(null);  
            DontDestroyOnLoad(gameObject);
        }
    }
}
