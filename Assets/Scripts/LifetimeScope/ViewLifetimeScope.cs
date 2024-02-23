using UnityEngine;
using VContainer;
using VContainer.Unity;

public class ViewLifetimeScope : LifetimeScope
{
    [SerializeField] Menu _menu;
    protected override void Configure(IContainerBuilder builder)
    {
        //  MenuにはInjectしないのでRegisterInstanceで登録
        builder.RegisterInstance(_menu);
    }
}
