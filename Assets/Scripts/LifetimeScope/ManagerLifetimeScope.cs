using UnityEngine;
using VContainer;
using VContainer.Unity;

public class ManagerLifetimeScope : LifetimeScope
{
    [SerializeField] EventManager _eventManager;
    [SerializeField] JsonManager _jsonManager;
    
    protected override void Configure(IContainerBuilder builder)
    {
        builder.RegisterComponent(_jsonManager);
        builder.RegisterComponent(_eventManager);
    }
}