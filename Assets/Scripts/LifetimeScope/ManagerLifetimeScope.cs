using System.Linq;
using UnityEngine;
using VContainer;
using VContainer.Unity;

public class ManagerLifetimeScope : LifetimeScope
{
    [SerializeField] EventManager _eventManager;
    [SerializeField] JsonManager _jsonManager;
    [SerializeField] CanvasManager _canvasManager;
    protected override void Configure(IContainerBuilder builder)
    {
        builder.RegisterComponent(_canvasManager);
        builder.RegisterComponent(_jsonManager);
        builder.RegisterComponent(_eventManager);
        foreach (var datum in _eventManager.EventTriggerData
                     .Where(datum=>datum.EventTrigger.EventClip is ReincarnationEvent))
        {
            builder.RegisterComponent((ReincarnationEvent)datum.EventTrigger.EventClip);
        }
        foreach (var datum in _eventManager.EventTriggerData
                     .Where(datum=>datum.EventTrigger.EventClip is UnlockAchievement))
        {
            builder.RegisterComponent((UnlockAchievement)datum.EventTrigger.EventClip);
        }
    }
}