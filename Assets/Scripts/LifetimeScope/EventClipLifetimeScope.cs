using System.Collections.Generic;
using UnityEngine;
using VContainer;
using VContainer.Unity;

public class EventClipLifetimeScope : LifetimeScope
{
    [SerializeField] List<EventTrigger> _hasInjectionEventTriggers;
    protected override void Configure(IContainerBuilder builder)
    {
        foreach (var eventTrigger in _hasInjectionEventTriggers)
        {
            builder.RegisterComponent(eventTrigger.EventClip);
        }
    }
}
