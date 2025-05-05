/*
 * Copyright (c) 2025 Matvii Jarosh
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
*/
#include "mainwindow.h"
#include "contactswindow.h"
#include "aboutwindow.h"

#include <QMessageBox>
#include <QFileDialog>
#include <QMenuBar>
#include <QColorDialog>
#include <QFontDialog>
#include <QTextBlock>
#include <QTextDocumentFragment>
#include <QRegularExpression>
#include <QTextList>
#include <QPagedPaintDevice>
#include <QPrinter>
#include <QPrintDialog>

MainWindow::MainWindow(const QString &_filePath, QWidget *parent)
    : QMainWindow(parent), filePath(_filePath.isEmpty() ? "none" : _filePath)
{
    setMinimumSize(400, 300);
    resize(640, 480);
    setWindowIcon(QIcon(":/myappico.ico"));

    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    layout = new QVBoxLayout(centralWidget);
    layout->setContentsMargins(0,0,0,0);

    textEdit = new QTextEdit();
    textEdit->setAcceptRichText(true);
    connect(textEdit, &QTextEdit::textChanged, this, [this](){
        setWindowTitle(filePath + "*");
        isSaving = false;
    });

    layout->addWidget(textEdit);

    if (!filePath.isEmpty() && filePath != "none") {
        QFile file(filePath);
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            textEdit->setHtml(file.readAll());
            file.close();
        }
    }

    setupMenu();
}

MainWindow::~MainWindow() {}

void MainWindow::closeEvent(QCloseEvent *event) {
    if (!isSaving) {
        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(this, "Unsaved changes",
                                      "The file contains unsaved changes. Are you sure you want to exit?",
                                      QMessageBox::Yes | QMessageBox::No);
        if (reply == QMessageBox::No) {
            event->ignore();
            return;
        }
    }
    event->accept();
}

void MainWindow::setupMenu() {
    QMenu* fileMenu = menuBar()->addMenu("&File");
    fileMenu->addAction(QIcon::fromTheme(QIcon::ThemeIcon::DocumentNew), "&New", this, &MainWindow::newFile)->setShortcut(QKeySequence::New);
    fileMenu->addAction(QIcon::fromTheme(QIcon::ThemeIcon::DocumentOpen), "&Open", this, &MainWindow::openFile)->setShortcut(QKeySequence::Open);
    fileMenu->addAction(QIcon::fromTheme(QIcon::ThemeIcon::DocumentSave), "&Save", this, &MainWindow::saveFile)->setShortcut(QKeySequence::Save);
    fileMenu->addAction(QIcon::fromTheme(QIcon::ThemeIcon::DocumentSaveAs), "&Save as", this, &MainWindow::saveAsFile)->setShortcut(QKeySequence::SaveAs);
    fileMenu->addSeparator();
    fileMenu->addAction("&Export as text", this, &MainWindow::exportAsPlainText);
    fileMenu->addSeparator();
    fileMenu->addAction(QIcon::fromTheme(QIcon::ThemeIcon::Printer), "&Print", this, &MainWindow::print)->setShortcut(QKeySequence::Print);
    fileMenu->addSeparator();
    fileMenu->addAction(QIcon::fromTheme(QIcon::ThemeIcon::ApplicationExit), "&Exit", this, &QMainWindow::close)->setShortcut(QKeySequence::Quit);

    QMenu* editMenu = menuBar()->addMenu("&Edit");
    QAction* undoAction = editMenu->addAction(QIcon::fromTheme(QIcon::ThemeIcon::EditUndo), "&Undo", this, [this](){ textEdit->undo(); });
    undoAction->setShortcut(QKeySequence::Undo);

    QAction* redoAction = editMenu->addAction(QIcon::fromTheme(QIcon::ThemeIcon::EditRedo), "&Redo", this, [this](){ textEdit->redo(); });
    redoAction->setShortcut(QKeySequence::Redo);

    editMenu->addSeparator();

    QAction* cutAction = editMenu->addAction(QIcon::fromTheme(QIcon::ThemeIcon::EditCut), "&Cut", this, [this](){ textEdit->cut(); });
    cutAction->setShortcut(QKeySequence::Cut);

    QAction* copyAction = editMenu->addAction(QIcon::fromTheme(QIcon::ThemeIcon::EditCopy), "&Copy", this, [this](){ textEdit->copy(); });
    copyAction->setShortcut(QKeySequence::Copy);

    QAction* pasteAction = editMenu->addAction(QIcon::fromTheme(QIcon::ThemeIcon::EditPaste), "&Paste", this, [this](){ textEdit->paste(); });
    pasteAction->setShortcut(QKeySequence::Paste);

    editMenu->addSeparator();

    QAction* selectAllAction = editMenu->addAction(QIcon::fromTheme(QIcon::ThemeIcon::EditSelectAll), "&Select all", this, [this](){ textEdit->selectAll(); });
    selectAllAction->setShortcut(QKeySequence::SelectAll);

    QMenu* formatMenu = menuBar()->addMenu("&Format");
    formatMenu->addAction(QIcon::fromTheme(QIcon::ThemeIcon::FormatTextBold), "&Bold", this, &MainWindow::bold)->setShortcut(QKeySequence::Bold);
    formatMenu->addAction(QIcon::fromTheme(QIcon::ThemeIcon::FormatTextItalic), "&Italic", this, &MainWindow::italic)->setShortcut(QKeySequence::Italic);
    formatMenu->addAction(QIcon::fromTheme(QIcon::ThemeIcon::FormatTextUnderline), "&Underline", this, &MainWindow::underline)->setShortcut(QKeySequence::Underline);
    formatMenu->addSeparator();
    formatMenu->addAction("&Change color", this, &MainWindow::color);
    formatMenu->addAction(QIcon::fromTheme("preferences-desktop-font"), "&Change font/size", this, &MainWindow::font);
    formatMenu->addSeparator();
    formatMenu->addAction(QIcon::fromTheme(QIcon::ThemeIcon::FormatJustifyLeft), "&Align left", this, [this](){setAlign(Qt::AlignLeft);});
    formatMenu->addAction(QIcon::fromTheme(QIcon::ThemeIcon::FormatJustifyCenter), "&Center", this, [this](){setAlign(Qt::AlignCenter);});
    formatMenu->addAction(QIcon::fromTheme(QIcon::ThemeIcon::FormatJustifyRight), "&Align right", this, [this](){setAlign(Qt::AlignRight);});
    formatMenu->addSeparator();
    formatMenu->addAction("&Disc list", this, [this](){createList(QTextListFormat::ListDisc);});
    formatMenu->addAction("&Namerical list", this, [this](){createList(QTextListFormat::ListDecimal);});
    formatMenu->addSeparator();
    formatMenu->addAction(QIcon::fromTheme(QIcon::ThemeIcon::ListAdd), "&Horizontal line", this, [this](){textEdit->insertHtml("<hr>");});
    formatMenu->addSeparator();
    formatMenu->addAction("&Make plain text", this, &MainWindow::makePlainText);

    QMenu* helpMenu = menuBar()->addMenu("&Help");
    helpMenu->addAction("&Contacts", this, &MainWindow::showContacts);
    helpMenu->addSeparator();
    helpMenu->addAction(QIcon::fromTheme(QIcon::ThemeIcon::HelpAbout), "&About", this, &MainWindow::showAbout);
}

void MainWindow::createList(QTextListFormat::Style style) {
    QTextCursor cursor = textEdit->textCursor();
    QTextListFormat listFormat;
    listFormat.setStyle(style);
    listFormat.setIndent(1);

    if (!cursor.hasSelection()) {
        cursor.insertList(listFormat);
    } else {
        int startPos = cursor.selectionStart();
        int endPos = cursor.selectionEnd();

        QTextBlock startBlock = textEdit->document()->findBlock(startPos);
        QTextBlock endBlock = textEdit->document()->findBlock(endPos);

        if (endPos == endBlock.position() && startBlock != endBlock) {
            endBlock = endBlock.previous();
        }

        QVector<QTextBlock> blocksToFormat;
        for (QTextBlock block = startBlock; block.isValid() && block.position() <= endBlock.position(); block = block.next()) {
            blocksToFormat.append(block);
        }

        foreach (QTextBlock block, blocksToFormat) {
            QTextCursor tempCursor(block);
            QTextList* existingList = tempCursor.currentList();
            if (existingList) {
                existingList->remove(block);
            }
        }

        if (!blocksToFormat.isEmpty()) {
            cursor.setPosition(blocksToFormat.first().position());
            QTextList* newList = cursor.createList(listFormat);

            for (int i = 1; i < blocksToFormat.size(); ++i) {
                cursor.setPosition(blocksToFormat[i].position());
                newList->add(cursor.block());
            }
        }
    }

    textEdit->setTextCursor(cursor);
}

void MainWindow::setAlign(Qt::Alignment align) {
    QTextCursor cursor = textEdit->textCursor();
    QTextBlockFormat blockFormat;
    blockFormat.setAlignment(align);

    if (!cursor.hasSelection()) {
        cursor.mergeBlockFormat(blockFormat);
    } else {

        QTextBlock startBlock = textEdit->document()->findBlock(cursor.selectionStart());
        QTextBlock endBlock = textEdit->document()->findBlock(cursor.selectionEnd());

        for (QTextBlock block = startBlock; block.isValid() && block.position() <= endBlock.position(); block = block.next()) {
            QTextCursor blockCursor(block);
            blockCursor.mergeBlockFormat(blockFormat);
        }
    }

    textEdit->setTextCursor(cursor);
}

void MainWindow::newFile() {
    filePath = "none";
    textEdit->setHtml("");
}

void MainWindow::openFile() {
    QStringList filters = {
        "HTML (*.html)",
        "All Files (*)"
    };

    filePath = QFileDialog::getOpenFileName(
        this,
        "Open HTML File",
        QDir::homePath(),
        filters.join(";;")
        );

    if (!filePath.isEmpty()) {
        QFile file(filePath);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QMessageBox::critical(this, "Error", "Could not open file:");
            return;
        }
        QTextStream in(&file);
        QString text = in.readAll();
        textEdit->setHtml(text);
    }
}

void MainWindow::saveFile() {
    if (filePath.isEmpty() || filePath == "none") {
        QStringList filters = {
            "HTML (*.html)",
            "All Files (*)"
        };

        filePath = QFileDialog::getSaveFileName(
            this,
            "Save File",
            QDir::homePath(),
            filters.join(";;")
            );

        if (filePath.isEmpty()) {
            return;
        }
    }

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::critical(this, "Error", "Could not save file!");
        return;
    }

    QTextStream out(&file);
    out << textEdit->toHtml();
    file.close();
    isSaving = true;
    setWindowTitle(filePath);
}

void MainWindow::saveAsFile() {
    QStringList filters = {
        "HTML (*.html)",
        "All Files (*)"
    };

    QString oldPath = filePath;
    filePath = QFileDialog::getSaveFileName(
        this,
        "Save File",
        QDir::homePath(),
        filters.join(";;")
        );

    if (filePath.isEmpty()) {
        filePath = oldPath;
        return;
    }

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::critical(this, "Error", "Could not save file!");
        return;
    }

    QTextStream out(&file);
    out << textEdit->toHtml();
    file.close();
    isSaving = true;
    setWindowTitle(filePath);
}

void MainWindow::exportAsPlainText() {
    QStringList filters = {
        "Text files (*.txt)",
        "All Files (*)"
    };

    QString eFilePath = QFileDialog::getSaveFileName(
        this,
        "Save File",
        QDir::homePath(),
        filters.join(";;")
        );

    if (eFilePath.isEmpty()) {
        return;
    }

    QFile file(eFilePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::critical(this, "Error", "Could not save file!");
        return;
    }

    QTextStream out(&file);
    out << textEdit->toPlainText();
    file.close();
}

void MainWindow::print() {
    QString text = textEdit->toHtml();
    QPrinter printer;
    QPrintDialog printDialog(&printer, this);

    if (printDialog.exec() == QDialog::Accepted) {
        QTextDocument document;
        document.setHtml(text);
        document.print(&printer);
    }
}

void MainWindow::bold() {
    QTextCursor cursor = textEdit->textCursor();

    if (!cursor.hasSelection()) {
        QTextCharFormat format = textEdit->currentCharFormat();
        format.setFontWeight(format.fontWeight() == QFont::Bold
                                 ? QFont::Normal
                                 : QFont::Bold);
        textEdit->setCurrentCharFormat(format);
    } else {
        QTextCharFormat format = cursor.charFormat();
        format.setFontWeight(format.fontWeight() == QFont::Bold
                                 ? QFont::Normal
                                 : QFont::Bold);
        cursor.mergeCharFormat(format);

        int start = cursor.selectionStart();
        int end = cursor.selectionEnd();
        cursor.clearSelection();
        cursor.setPosition(start);
        cursor.setPosition(end, QTextCursor::KeepAnchor);
        textEdit->setTextCursor(cursor);
    }
}

void MainWindow::italic() {
    QTextCursor cursor = textEdit->textCursor();

    if (!cursor.hasSelection()) {
        QTextCharFormat format = textEdit->currentCharFormat();
        format.setFontItalic(!format.fontItalic());
        textEdit->setCurrentCharFormat(format);
    } else {
        QTextCharFormat format = cursor.charFormat();
        format.setFontItalic(!format.fontItalic());
        cursor.mergeCharFormat(format);

        int start = cursor.selectionStart();
        int end = cursor.selectionEnd();
        cursor.clearSelection();
        cursor.setPosition(start);
        cursor.setPosition(end, QTextCursor::KeepAnchor);
        textEdit->setTextCursor(cursor);
    }
}

void MainWindow::underline() {
    QTextCursor cursor = textEdit->textCursor();

    if (!cursor.hasSelection()) {
        QTextCharFormat format = textEdit->currentCharFormat();
        format.setFontUnderline(!format.fontUnderline());
        textEdit->setCurrentCharFormat(format);
    } else {
        QTextCharFormat format = cursor.charFormat();
        format.setFontUnderline(!format.fontUnderline());
        cursor.mergeCharFormat(format);

        int start = cursor.selectionStart();
        int end = cursor.selectionEnd();
        cursor.clearSelection();
        cursor.setPosition(start);
        cursor.setPosition(end, QTextCursor::KeepAnchor);
        textEdit->setTextCursor(cursor);
    }
}

void MainWindow::color() {
    QColor color = QColorDialog::getColor(Qt::white, this, "Choose color");
    if (!color.isValid()) {
        return;
    }

    QBrush newBrush;
    newBrush = QBrush(color);

    QTextCursor cursor = textEdit->textCursor();
    QTextCharFormat format;
    format.setForeground(newBrush);

    if (!cursor.hasSelection()) {
        textEdit->mergeCurrentCharFormat(format);
    } else {
        cursor.mergeCharFormat(format);
        textEdit->setTextCursor(cursor);
    }
}

void MainWindow::font() {
    bool ok;
    QFont font = QFontDialog::getFont(&ok);

    QTextCursor cursor = textEdit->textCursor();
    QTextCharFormat format;
    format.setFont(font);

    if (!cursor.hasSelection()) {
        textEdit->mergeCurrentCharFormat(format);
    } else {
        cursor.mergeCharFormat(format);
        textEdit->setTextCursor(cursor);
    }
}

void MainWindow::makePlainText() {
    QTextCursor cursor = textEdit->textCursor();

    if (!cursor.hasSelection()) {
        textEdit->setCurrentCharFormat(QTextCharFormat());
    } else {
        QString plainText = cursor.selectedText();
        QTextCharFormat plainFormat = cursor.charFormat();
        plainFormat.setFontWeight(QFont::Normal);
        plainFormat.setFontItalic(false);
        plainFormat.setFontUnderline(false);
        plainFormat.setFontStrikeOut(false);
        plainFormat.setForeground(QBrush(Qt::white));
        plainFormat.setBackground(QBrush(Qt::transparent));
        plainFormat.setFont(textEdit->font());
        cursor.mergeCharFormat(plainFormat);
        cursor.insertText(plainText);
    }
}

void MainWindow::showContacts() {
    ContactsWindow* ct = new ContactsWindow();
    ct->show();
}


void MainWindow::showAbout() {
    AboutWindow* ab = new AboutWindow();
    ab->show();
}
