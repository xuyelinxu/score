#pragma once
#include <QCommandLineParser>
#include <QCommandLineOption>
#include <QFile>
#include <QApplication>
struct ApplicationSettings
{
        bool tryToRestore = true;
        bool gui = true;
        QStringList loadList;

        void parse()
        {
            QCommandLineParser parser;
            parser.setApplicationDescription(QObject::tr("i-score - An interactive sequencer for the intermedia arts."));
            parser.addHelpOption();
            parser.addVersionOption();
            parser.addPositionalArgument("load", QCoreApplication::translate("main", "Scenario to load."));

            QCommandLineOption noGUI("no-gui", QCoreApplication::translate("main", "Disable GUI"));
            parser.addOption(noGUI);
            QCommandLineOption noRestore("no-restore", QCoreApplication::translate("main", "Disable auto-restore"));
            parser.addOption(noRestore);

            parser.process(*qApp);

            const QStringList args = parser.positionalArguments();

            tryToRestore = !parser.isSet(noRestore);
            gui = !parser.isSet(noGUI);

            if(args.size() > 0 && QFile::exists(args[0]))
            {
                loadList.push_back(args[0]);
            }
        }
};