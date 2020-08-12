#pragma once

#include "baseitems/graphicsview.h"
#include "plugin.h"

namespace aadlinterface {

class InterfaceDocument;

class IVEditorPlugin : public shared::Plugin
{
public:
    explicit IVEditorPlugin(QObject *parent = 0);

    aadlinterface::InterfaceDocument *document() const;

    void setPluginActive(bool active) override;

    GraphicsView *chartView() override;
    void addToolBars(QMainWindow *window) override;
    void addMenuFileActions(QMenu *menu, QMainWindow *window) override;
    void addMenuEditActions(QMenu *menu, QMainWindow *window) override;
    void addMenuViewActions(QMenu *menu, QMainWindow *window) override;
    void addMenuHelpActions(QMenu *menu, QMainWindow *window) override;

    QToolBar *docToolBar() { return m_docToolBar; }

    void populateCommandLineArguments(shared::CommandLineParser *parser) const override;

    QAction *actionSaveSceneRender() { return m_actionSaveSceneRender; }
    QAction *actionShowAsnDialog() { return m_actionShowAsnDialog; }

private:
    aadlinterface::InterfaceDocument *m_document { nullptr };

    GraphicsView *m_graphicsView;
    QToolBar *m_docToolBar;

    QAction *m_actionSaveSceneRender { nullptr };
    QAction *m_actionShowAsnDialog { nullptr };
};

}
