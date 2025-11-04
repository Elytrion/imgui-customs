#include "demo_manager.h"
// demo modules
#include "helpModule.h"
#include "dockEnforcer/dockEnforcerDemo.h"
#include "spinner/spinnerDemo.h"
#include "localModal/localModalDemo.h"
#include "toggle/toggleDemo.h"
#include "multiToggle/multiToggleDemo.h"
#include "tween/tweenDemo.h"
#include "progressBar/progressBarDemo.h"
#include "animatedText/animTextDemo.h"

void DemoManager::InitModules()
{
	// Modules will be displayed in the order they are registered here
	RegisterDemoModule(std::make_shared<HelpModule>());
	RegisterDemoModule(std::make_shared<DockEnforcerDemo>());
	RegisterDemoModule(std::make_shared<SpinnerDemo>());
	RegisterDemoModule(std::make_shared<LocalModalDemo>());
	RegisterDemoModule(std::make_shared<ToggleDemo>());
	RegisterDemoModule(std::make_shared<MultiToggleDemo>());
	RegisterDemoModule(std::make_shared<TweenDemo>());
	RegisterDemoModule(std::make_shared<ProgressBarDemo>());
	RegisterDemoModule(std::make_shared<AnimatedTextDemo>());
}