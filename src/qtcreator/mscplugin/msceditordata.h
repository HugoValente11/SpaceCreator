/*
   Copyright (C) 2018 - 2019 European Space Agency - <maxime.perrotin@esa.int>

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

#include <QCoreApplication>
#include <QToolBar>
#include <QUndoGroup>
#include <coreplugin/editortoolbar.h>
#include <coreplugin/icontext.h>

using namespace Core;

namespace MscPlugin {
namespace Internal {

class MscTextEditorFactory;
class MscEditorWidget;
class MscEditorStack;
class MscContext;

class MscEditorData : public QObject
{
    Q_OBJECT
public:
    MscEditorData(QObject *parent = nullptr);
    ~MscEditorData() override;

    void fullInit();
    IEditor *createEditor();

private:
    void updateToolBar();
    QWidget *createModeWidget();
    EditorToolBar *createMainToolBar();

    MscContext *m_context = nullptr;
    Context m_contexts;
    QWidget *m_modeWidget = nullptr;
    MscEditorStack *m_widgetStack = nullptr;
    QToolBar *m_widgetToolBar = nullptr;
    EditorToolBar *m_mainToolBar = nullptr;
    QUndoGroup *m_undoGroup = nullptr;
    QAction *m_undoAction = nullptr;
    QAction *m_redoAction = nullptr;

    MscTextEditorFactory *m_editorFactory = nullptr;
};

} // namespace Internal
} // namespace MscPlugin