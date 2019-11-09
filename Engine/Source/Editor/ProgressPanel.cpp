#include "ProgressPanel.h"
#include <GUI/Manager.h>
#include <GUI/Window.h>

namespace Squirrel {
namespace Editor {

ProgressPanel::ProgressPanel()
{
	mPanel = new GUI::Window();
	mPanel->setName("ProgressPanel");
	mPanel->setText("Progress:");
	mPanel->setSizeSep(350, 40);
	GUI::Manager::Instance().getMainPanel().addPanel( mPanel );

	mTitle = NULL;

	mProgress = mPanel->add<GUI::Label>("progressStageLabel", 3, 2, 330, 18);

	//hide
	mPanel->setVisible(false);
}

ProgressPanel::~ProgressPanel()
{
	GUI::Manager::Instance().getMainPanel().delPanel( mPanel );
	DELETE_PTR(mPanel);
}

void ProgressPanel::show(const char_t * title)
{
	mPanel->setVisible(true);
	mPanel->setTopmost();
	mPanel->setText(title);

	tuple2i wndSize = Render::IRender::GetActive()->getWindow()->getSize();
	tuple2i pos = wndSize - mPanel->getSize();
	pos.x /= 2;
	pos.y /= 2;

	mPanel->setPos(pos);
}

void ProgressPanel::setText(const char_t * text)
{
	mPanel->setTopmost();
	mProgress->setText(text);
}

void ProgressPanel::close()
{
	mPanel->setVisible(false);
}

}//namespace Render { 
}//namespace Squirrel {

