#pragma once
#include <interface/plugins/PanelFactoryInterface_QtInterface.hpp>
#include <interface/plugins/Autoconnect_QtInterface.hpp>

class InspectorPlugin :
	public QObject,
	public iscore::Autoconnect_QtInterface,
	public iscore::PanelFactoryInterface_QtInterface
{
		Q_OBJECT
		Q_PLUGIN_METADATA (IID Autoconnect_QtInterface_iid)
		Q_INTERFACES (iscore::Autoconnect_QtInterface
					  iscore::PanelFactoryInterface_QtInterface)

	public:
		InspectorPlugin();
		virtual ~InspectorPlugin() = default;

		// Autoconnect interface
		virtual QList<iscore::Autoconnect> autoconnect_list() const override;

		// Panel interface
		virtual QStringList panel_list() const override;
		virtual iscore::PanelFactoryInterface* panel_make (QString name) override;

};
