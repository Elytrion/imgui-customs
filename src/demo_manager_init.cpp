#include "demo_manager.h"
// demo modules
#include "dockEnforcer/dockEnforcerDemo.h"
#include "spinner/spinnerDemo.h"
#include "localModal/localModalDemo.h"
#include "toggle/toggleDemo.h"


void DemoManager::InitModules()
{
	RegisterDemoModule(std::make_shared<DockEnforcerDemo>());
	RegisterDemoModule(std::make_shared<SpinnerDemo>());
	RegisterDemoModule(std::make_shared<LocalModalDemo>());
	RegisterDemoModule(std::make_shared<ToggleDemo>());
}