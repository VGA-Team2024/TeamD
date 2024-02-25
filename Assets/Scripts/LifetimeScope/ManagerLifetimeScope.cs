using System.Linq;
using Story;
using UnityEngine;
using VContainer;
using VContainer.Unity;

public class ManagerLifetimeScope : LifetimeScope
{
    [SerializeField] EventManager _eventManager;
    [SerializeField] JsonManager _jsonManager;
    [SerializeField] CanvasManager _canvasManager;
    [SerializeField] StoryEventManager _storyEventManager;
    [SerializeField] Shop _shop;
    protected override void Configure(IContainerBuilder builder)
    {
        builder.RegisterComponent(_canvasManager);
        builder.RegisterComponent(_jsonManager);
        builder.RegisterComponent(_eventManager);
        builder.RegisterComponent(_storyEventManager);
        builder.RegisterInstance(_shop);
        //  コンテナビルド後、オブジェクトに注入を行う
        builder.RegisterBuildCallback(container =>
        {
            //  EventManager内にReincarnationEventがあれば注入する
            foreach (var datum in _eventManager.EventTriggerData
                         .Where(datum=>datum.EventTrigger.EventClip is ReincarnationEvent))
            {
                container.Inject((ReincarnationEvent)datum.EventTrigger.EventClip);
            }
            //  EventManager内にUnlockAchievementがあれば注入する
            foreach (var datum in _eventManager.EventTriggerData
                         .Where(datum=>datum.EventTrigger.EventClip is UnlockAchievement))
            {
                container.Inject((UnlockAchievement)datum.EventTrigger.EventClip);
            }
            //  StoryEventManager内にUnlockAchievementがあれば注入する
            foreach (var datum in _storyEventManager.EventTriggerInfos)
            {
                foreach (var storyOption in datum.EventTrigger.StoryOptions
                             .Where(storyOption=>storyOption.EventClip is UnlockAchievement))
                {
                    container.Inject((UnlockAchievement)storyOption.EventClip);
                }
            }
        });
    }
}