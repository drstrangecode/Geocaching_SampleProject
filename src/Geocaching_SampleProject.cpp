/**
 * Name        : Geocaching_SampleProject
 * Version     : 
 * Vendor      : 
 * Description : 
 */


#include "Geocaching_SampleProject.h"
#include "MainForm.h"

using namespace Osp::App;
using namespace Osp::Base;
using namespace Osp::System;
using namespace Osp::Ui;
using namespace Osp::Ui::Controls;

Geocaching_SampleProject::Geocaching_SampleProject()
{
}

Geocaching_SampleProject::~Geocaching_SampleProject()
{
}

Application*
Geocaching_SampleProject::CreateInstance(void)
{
	return new Geocaching_SampleProject();
}

bool
Geocaching_SampleProject::OnAppInitializing(AppRegistry& appRegistry)
{
    MainForm * mainForm = new MainForm();
    mainForm->Initialize();

    GetAppFrame()->GetFrame()->AddControl(*mainForm);
    GetAppFrame()->GetFrame()->SetCurrentForm(*mainForm);

    mainForm->Draw();
    mainForm->Show();

	return true;
}

bool
Geocaching_SampleProject::OnAppTerminating(AppRegistry& appRegistry, bool forcedTermination)
{
	return true;
}

void
Geocaching_SampleProject::OnForeground(void)
{
}

void
Geocaching_SampleProject::OnBackground(void)
{
}

void
Geocaching_SampleProject::OnLowMemory(void)
{

}

void
Geocaching_SampleProject::OnBatteryLevelChanged(BatteryLevel batteryLevel)
{

}

void
Geocaching_SampleProject::OnScreenOn (void)
{

}

void
Geocaching_SampleProject::OnScreenOff (void)
{

}
