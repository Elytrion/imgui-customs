#include "demo_manager.h"
// demo modules
#include "dockEnforcer/dockEnforcerDemo.h"

void DemoManager::InitModules()
{
	RegisterDemoModule(std::make_shared<DockEnforcerDemo>());
}