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
#include "aboutwindow.h"

AboutWindow::AboutWindow(QWidget *parent) : QWidget(parent) {
    setWindowTitle("About TexEdit");
    setWindowIcon(QIcon::fromTheme(QIcon::ThemeIcon::HelpAbout));
    setFixedSize(300, 200);

    QGridLayout *gridLayout = new QGridLayout(this);
    gridLayout->setContentsMargins(0, 0, 0, 0);
    setLayout(gridLayout);

    QLabel *label = new QLabel(this);
    label->setTextFormat(Qt::MarkdownText);
    label->setTextInteractionFlags(Qt::TextSelectableByMouse);
    label->setText(
        R"(# TexEdit
**author**: Matvii Jarosh \
**copyright**: (c) 2025 Matvii Jarosh \
**license**: MIT \
**description**: TexEdit is a relatively simple text editor.
It handles plain text or HTML, and can display any file as text.)");
    label->setWordWrap(true);

    QLabel* img = new QLabel(this);
    QPixmap originalPixmap(":/myappico.ico");
    QPixmap scaledPixmap = originalPixmap.scaled(75, 75, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    img->setPixmap(scaledPixmap);

    gridLayout->addWidget(img, 0, 1);
    gridLayout->addWidget(label, 0, 0);

    gridLayout->setRowMinimumHeight(0, 100);
    gridLayout->setColumnMinimumWidth(0, 100);
}

AboutWindow::~AboutWindow() {}
