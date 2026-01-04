This plugin for Unreal helps you to reduce your game's power consumption by adjusting several settings if no input gets detected over some time.

# What does the plugin do?
Every frame, the plugin checks how long it has been since the player made any input. After certain thresholds, it throttles the frame rate and the screen percentage to save energy, assuming that the player isn't actively playing the game anymore. If enough time has passed, the plugin can even disable the game's rendering completely, resulting in a black window.
As soon as any input is noticed, the game returns to the previous settings, so it doesn't affect the actual gameplay experience.

# Why is that a good thing?
Especially on handheld devices, reducing the power consumption can improve the player experience as the battery lasts longer. But it also helps to avoid overheating and reduces both the player's energy bill and the game's carbon footprint.

# Why does this plugin exist?
Game development can be quite stressful and most games aren't shipped on time. Features that are not strictly necessary to ship are usually the first ones to get cut as the deadline gets closer. The idea behind this plugin is to make it easier for developers to make their games energy-efficient without having to find extra time for it in the production schedule. If you notice that the plugin is not doing that for you and you have any idea how to change that, please reach out to me.

# How to use the plugin
After enabling the plugin, you can configure it using several cvars:
**EnergySaver.TimeThresholdForEnergySaving** is the time after which the frame rate and screen percentage get throttled.
**EnergySaver.TimeThresholdForDisabledRendering** is the time after which rendering gets disabled completely.

Both settings exist in two versions: **PluggedIn** and **OnBattery**, allowing you to use more aggressive settings in situations where power consumption really matters to the user.
**EnergySaver.MaxFps** and **EnergySaver.MaxScreenPercentage** are the settings used while the energy saving mode is enabled.

**EnergySaver.WhenWindowInactive** controls whether the energy saving mode should also be enabled while the window is out of focus (only relevant for PC).
I recommend exposing these cvars as settings to the player, and have sensible defaults that make sense for your game (Strategy games for example usually have longer periods of players not doing anything compared to racing games etc.)
## When and how to disable the plugin
In typical gameplay situations, it's usually safe to assume after some time without input that the player isn't paying attention anymore. That is not the case during cutscenes, though. To avoid the energy saving mode kicking in during cutscenes, you can use the **bIsEnergySavingAllowed** property of the **EnergySaver** subsystem. Just set **bIsEnergySavingAllowed** to **false** when starting a cutscene and set it back to **true** afterwards.
This property should only be used for temporary overrides. When giving the player the option to disable the energy saving mode, set the time thresholds to 0 instead.

# Notes on platforms
When reducing the screen percentage, the plugin tries to use Unreal's dynamic resolution feature, which is only available on consoles. On PCs, the screen percentage is set directly, which can result in small hitches, but that shouldn't be an issue as the setting is not adjusted every frame, but only once, and usually in a situation when the player isn't playing the game anyway.
On PS5, make sure to set Slate.Input.MotionFiresUserInteractionEvents to false, otherwise the plugin won't work. 

# Sources and further reading
The implementation of the energy saving mode leans heavily on [this paper](https://cdn2.unrealengine.com/reducing-fortnites-power-consumption-layout-v03-ffedbeb1adeb.pdf) covering Fortnite's energy saving features, and I recommend checking it out.
To learn more about the energy saving techniques used in this plugin and other possible improvements, check out [my blog post about the topic](https://haukethiessen.com/developing-games-for-devices-with-batteries/). 

# Requirements
The plugin requires at least Unreal 5.3 to work on consoles, as that version introduced the cvar **r.DynamicRes.ThrottlingMaxScreenPercentage**. For older engine versions, you can still make the plugin work by replacing it with **r.DynamicRes.MaxScreenPercentage**. On PS5, the plugin won't work with older engine versions because there's no way to disable the motion input not affecting the last interaction time. For more details on this, check out Epic's paper linked above.

# Further development
This plugin is still in a very early state, so be aware of that when using it in an actual production context. As of now, I only managed to test the game on PC. Any feedback and bug reports are appreciated.
## Planned features:
- When disabling the rendering, ideally, the plugin would take a screenshot to show instead the black screen in the following frames
- Option to dim the brightness for OLED screens
- Options to also change other settings based on player inactivity, like animation or VFX budgets.
## Known issues:
- Devices that are plugged in are currently considered as running on battery if the battery isn't charged 100%

# License
This plugin is free to use in commercial and non-commercial products.
This software is provided 'as-is', without any express or implied warranty. In no event will the authors be held liable for any damages arising from the use of this software.
Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely.
Attribution is appreciated, but not required.
