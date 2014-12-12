#pragma once
#include <interface/plugincontrol/PluginControlInterface.hpp>
#include <QAction>

class HelloWorldCommand : public iscore::PluginControlInterface
{
		Q_OBJECT
	public:
		HelloWorldCommand();
		virtual void populateMenus(iscore::MenubarManager*) override;
		virtual void populateToolbars() override;
		virtual void setPresenter(iscore::Presenter*) override;

		virtual iscore::SerializableCommand* instantiateUndoCommand(QString name, QByteArray data) override;

	signals:
		void incrementProcesses();
		void decrementProcesses();

	private slots:
		void on_actionTrigger();

	private:
		QAction* m_action_HelloWorldigate;
		iscore::Presenter* m_presenter{};
};
