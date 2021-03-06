/*
   Copyright (C) 2021 European Space Agency - <maxime.perrotin@esa.int>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this program. If not, see <https://www.gnu.org/licenses/lgpl-2.1.html>.
*/

#pragma once

#include "editorcore.h"

class QUndoStack;
class QToolBar;

namespace shared {
class CommandLineParser;
namespace cmd {
class CommandsStackBase;
}
namespace ui {
class GraphicsViewBase;
}
}

namespace dve {
class DVAppModel;

class DVEditorCore : public shared::EditorCore
{
    Q_OBJECT
public:
    explicit DVEditorCore(QObject *parent = nullptr);
    ~DVEditorCore() override;

    dve::DVAppModel *appModel() const;

    void addToolBars(QMainWindow *window) override;

    shared::ui::GraphicsViewBase *chartView() override;
    QToolBar *toolBar();
    QWidget *mainwidget();

    void registerBasicActions();

    QUndoStack *undoStack() const override;
    shared::cmd::CommandsStackBase *commandsStack() const;

    void populateCommandLineArguments(shared::CommandLineParser *parser) const override;
    bool renameAsnFile(const QString &oldName, const QString &newName) override;

    QString filePath() const override;
    bool save() override;

private:
    struct DeploymentInterfacePrivate;
    std::unique_ptr<DeploymentInterfacePrivate> d;
};

} // namespace dve
