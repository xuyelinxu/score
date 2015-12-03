#include <boost/optional/optional.hpp>
#include <core/document/DocumentBackups.hpp>
#include <core/document/DocumentModel.hpp>
#include <core/presenter/Presenter.hpp>
#include <core/view/View.hpp>
#include <iscore/plugins/application/GUIApplicationContextPlugin.hpp>
#include <iscore/plugins/panel/PanelPresenter.hpp>
#include <iscore/tools/SettableIdentifierGeneration.hpp>
#include <QByteArray>
#include <QFile>
#include <QFileDialog>
#include <QFlags>
#include <QIODevice>
#include <QJsonDocument>
#include <QMessageBox>

#include <QSaveFile>
#include <QSettings>
#include <QStringList>
#include <QVariant>
#include <utility>

#include "DocumentManager.hpp"
#include "QRecentFilesMenu.h"
#include <iscore/application/ApplicationComponents.hpp>
#include <core/command/CommandStack.hpp>
#include <core/command/CommandStackSerialization.hpp>
#include <core/document/Document.hpp>
#include <iscore/tools/SettableIdentifier.hpp>
#include <iscore/tools/std/StdlibWrapper.hpp>
#include <iscore/tools/std/Algorithms.hpp>

namespace iscore
{
DocumentManager::DocumentManager(
        iscore::View& view,
        QObject* parentPresenter):
    m_view{view},
    m_builder{parentPresenter, &view}
{

}

void DocumentManager::init(const ApplicationContext& ctx)
{
    con(m_view, &View::activeDocumentChanged,
        this, [&] (const Id<DocumentModel>& doc) {
        prepareNewDocument(ctx);
        auto it = find_if(m_documents, [&] (auto other) { return other->model().id() == doc; });
        setCurrentDocument(ctx, it != m_documents.end() ? *it : nullptr);
    });


    con(m_view, &View::closeRequested,
        this, [&] (const Id<DocumentModel>& doc) {
        auto it = find_if(m_documents, [&] (auto other) { return other->model().id() == doc; });
        ISCORE_ASSERT(it != m_documents.end());
        closeDocument(ctx, **it);
    });

    m_recentFiles = new QRecentFilesMenu{tr("Recent files"), nullptr};

    QSettings settings("OSSIA", "i-score");
    m_recentFiles->restoreState(settings.value("RecentFiles").toByteArray());

    connect(m_recentFiles, &QRecentFilesMenu::recentFileTriggered,
            this, [&] (const QString& f) { loadFile(ctx, f); });

}

DocumentManager::~DocumentManager()
{
    QSettings settings("OSSIA", "i-score");
    settings.setValue("RecentFiles", m_recentFiles->saveState());

    // The documents have to be deleted before the application context plug-ins.
    // This is because the Local device has to be deleted last in OSSIAApplicationPlugin.
    for(auto document : m_documents)
    {
        delete document;
    }

    m_documents.clear();
    m_currentDocument = nullptr;
}

Document* DocumentManager::setupDocument(
        const iscore::ApplicationContext& ctx,
        Document* doc)
{
    if(doc)
    {
        for(auto& panel : ctx.components.panelPresenters())
        {
            doc->setupNewPanel(panel.second);
        }

        m_documents.push_back(doc);
        m_view.addDocumentView(&doc->view());
        setCurrentDocument(ctx, doc);
        connect(&doc->model(), &DocumentModel::fileNameChanged,
                this, [=] (const QString& s)
        {
            m_view.on_fileNameChanged(&doc->view(), s);
        });
    }
    else
    {
        setCurrentDocument(
                    ctx,
                    m_documents.empty() ? nullptr : m_documents.front());
    }

    return doc;
}

Document *DocumentManager::currentDocument() const
{
    return m_currentDocument;
}

void DocumentManager::setCurrentDocument(
        const iscore::ApplicationContext& ctx,
        Document* doc)
{
    auto old = m_currentDocument;
    m_currentDocument = doc;

    for(auto& pair : ctx.components.panelPresenters())
    {
        if(doc)
            m_currentDocument->bindPanelPresenter(pair.first);
        else
            pair.first->setModel(nullptr);
    }

    for(auto& ctrl : ctx.components.applicationPlugins())
    {
        ctrl->on_documentChanged(old, m_currentDocument);
    }
}

bool DocumentManager::closeDocument(
        const iscore::ApplicationContext& ctx,
        Document& doc)
{
    // Warn the user if he might loose data
    if(!doc.commandStack().isAtSavedIndex())
    {
        QMessageBox msgBox;
        msgBox.setText(tr("The document has been modified."));
        msgBox.setInformativeText(tr("Do you want to save your changes?"));
        msgBox.setStandardButtons(QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
        msgBox.setDefaultButton(QMessageBox::Save);
        int ret = msgBox.exec();
        switch (ret)
        {
            case QMessageBox::Save:
                if(saveDocument(doc))
                    break;
                else
                    return false;
            case QMessageBox::Discard:
                // Do nothing
                break;
            case QMessageBox::Cancel:
                return false;
                break;
            default:
                break;
        }
    }

    // Close operation
    m_view.closeDocument(&doc.view());
    remove_one(m_documents, &doc);
    setCurrentDocument(ctx,
                       m_documents.size() > 0 ? m_documents.back() : nullptr);

    delete &doc;
    return true;
}

bool DocumentManager::saveDocument(Document& doc)
{
    auto savename = doc.docFileName();

    if(savename == tr("Untitled"))
    {
        saveDocumentAs(doc);
    }
    else if (savename.size() != 0)
    {
        QSaveFile f{savename};
        f.open(QIODevice::WriteOnly);
        if(savename.indexOf(".scorebin") != -1)
            f.write(doc.saveAsByteArray());
        else
        {
            QJsonDocument json_doc;
            json_doc.setObject(doc.saveAsJson());

            f.write(json_doc.toJson());
        }
        f.commit();
    }

    return true;
}

bool DocumentManager::saveDocumentAs(Document& doc)
{
    QFileDialog d{&m_view, tr("Save Document As")};
    QString binFilter{tr("Binary (*.scorebin)")};
    QString jsonFilter{tr("JSON (*.scorejson)")};
    QStringList filters;
    filters << jsonFilter
            << binFilter;

    d.setNameFilters(filters);
    d.setConfirmOverwrite(true);
    d.setFileMode(QFileDialog::AnyFile);
    d.setAcceptMode(QFileDialog::AcceptSave);

    if(d.exec())
    {
        QString savename = d.selectedFiles().first();
        auto suf = d.selectedNameFilter();

        if(!savename.isEmpty())
        {
            if(suf == binFilter)
            {
                if(!savename.contains(".scorebin"))
                    savename += ".scorebin";
            }
            else
            {
                if(!savename.contains(".scorejson"))
                    savename += ".scorejson";
            }

            QSaveFile f{savename};
            f.open(QIODevice::WriteOnly);
            doc.setDocFileName(savename);
            if(savename.indexOf(".scorebin") != -1)
                f.write(doc.saveAsByteArray());
            else
            {
                QJsonDocument json_doc;
                json_doc.setObject(doc.saveAsJson());

                f.write(json_doc.toJson());
            }
            f.commit();
        }
        return true;
    }
    return false;
}

bool DocumentManager::saveStack()
{
    QFileDialog d{&m_view, tr("Save Stack As")};
    d.setNameFilters({".stack"});
    d.setConfirmOverwrite(true);
    d.setFileMode(QFileDialog::AnyFile);
    d.setAcceptMode(QFileDialog::AcceptSave);

    if(d.exec())
    {
        QString savename = d.selectedFiles().first();
        if(!savename.isEmpty())
        {
            if(!savename.contains(".stack"))
                savename += ".stack";

            QSaveFile f{savename};
            f.open(QIODevice::WriteOnly);

            f.reset();
            Serializer<DataStream> ser(&f);
            ser.readFrom(currentDocument()->id());
            ser.readFrom(currentDocument()->commandStack());
            f.commit();
        }
        return true;
    }
    return false;
}

bool DocumentManager::loadStack(
        const iscore::ApplicationContext& ctx)
{
    QString loadname = QFileDialog::getOpenFileName(&m_view, tr("Open Stack"), QString(), "*.stack");
    if(!loadname.isEmpty()
        && (loadname.indexOf(".stack") != -1) )
    {
        QFile cmdF{loadname};
        cmdF.open(QIODevice::ReadOnly);
        QByteArray cmdArr {cmdF.readAll()};
        cmdF.close();

        Deserializer<DataStream> writer(cmdArr);

        Id<DocumentModel> id; //getStrongId(documents())
        writer.writeTo(id);

        newDocument(ctx,
                    id,
                    ctx.components.availableDocuments().front());

        loadCommandStack(
                    ctx.components,
                    writer,
                    currentDocument()->commandStack(),
                    [] (auto cmd) { cmd->redo(); }
        );

        return true;
    }

    return false;
}

Document* DocumentManager::loadFile(
        const iscore::ApplicationContext& ctx)
{
    QString loadname = QFileDialog::getOpenFileName(&m_view, tr("Open"), QString(), "*.scorebin *.scorejson");
    return loadFile(ctx, loadname);
}

Document* DocumentManager::loadFile(
        const iscore::ApplicationContext& ctx,
        const QString& fileName)
{
    Document* doc{};
    if(!fileName.isEmpty()
    && (fileName.indexOf(".scorebin") != -1
     || fileName.indexOf(".scorejson") != -1 ))
    {
        QFile f {fileName};
        if(f.open(QIODevice::ReadOnly))
        {
            if (fileName.indexOf(".scorebin") != -1)
            {
                doc = loadDocument(ctx, f.readAll(), ctx.components.availableDocuments().front());
            }
            else if (fileName.indexOf(".scorejson") != -1)
            {
                auto json = QJsonDocument::fromJson(f.readAll());
                doc = loadDocument(ctx, json.object(), ctx.components.availableDocuments().front());
            }

            m_currentDocument->setDocFileName(fileName);
            m_recentFiles->addRecentFile(fileName);
        }
    }

    return doc;
}

void DocumentManager::prepareNewDocument(
        const iscore::ApplicationContext& ctx)
{
    for(GUIApplicationContextPlugin* appPlugin : ctx.components.applicationPlugins())
    {
        appPlugin->prepareNewDocument();
    }
}

bool DocumentManager::closeAllDocuments(
        const iscore::ApplicationContext& ctx)
{
    while(!m_documents.empty())
    {
        bool b = closeDocument(ctx, *m_documents.back());
        if(!b)
            return false;
    }

    return true;
}



void DocumentManager::restoreDocuments(
        const iscore::ApplicationContext& ctx)
{
    for(const auto& backup : DocumentBackups::restorableDocuments())
    {
        restoreDocument(ctx, backup.first, backup.second, ctx.components.availableDocuments().front());
    }
}

}

Id<iscore::DocumentModel> getStrongId(const std::vector<iscore::Document*>& v)
{
    using namespace std;
    vector<int32_t> ids(v.size());   // Map reduce

    transform(v.begin(),
              v.end(),
              ids.begin(),
              [](const auto elt)
    {
        return * (elt->id().val());
    });

    return Id<iscore::DocumentModel>{iscore::id_generator::getNextId(ids)};
}
