#include "demo_manager.h"
// demo modules
#include "dockEnforcer/dockEnforcerDemo.h"
#include "spinner/spinnerDemo.h"


void DemoManager::InitModules()
{
	RegisterDemoModule(std::make_shared<DockEnforcerDemo>());
	RegisterDemoModule(std::make_shared<SpinnerDemo>());
}