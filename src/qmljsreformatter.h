/*
 * Copyright (C) 2017 - 2019 Orange
 *
 * This software is distributed under the terms and conditions of the GNU
 * General Public Licence version 3 as published by the Free Software
 * Foundation with exceptions as appearing in the file LICENSE.GPL3-EXCEPT
 * included in the packaging of this file. Please review the following
 * information to ensure the GNU General Public License requirements will
 * be met: https://www.gnu.org/licenses/gpl-3.0.html.
 */

/*
 * Orange version of the Qt Creator's qmljsreformatter
 *
 * Based on Qt Creator project. The modifications created by Orange in this
 * file provide a new signature for the 'reformat' function.
 *
 * Version:     1.0
 * Created:     2017-04-03 by Julien Déramond
 */

/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of Qt Creator.
**
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 as published by the Free Software
** Foundation with exceptions as appearing in the file LICENSE.GPL3-EXCEPT
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
****************************************************************************/

// MODIFIED BY ORANGE

#pragma once

#include "../qt-creator-simplified/src/libs/qmljs/qmljs_global.h"

#include "../qt-creator-simplified/src/libs/qmljs/qmljsdocument.h"

namespace QmlJS {
QMLJS_EXPORT QString reformat(const Document::Ptr &doc, bool splitLongLines = false);
QMLJS_EXPORT QString reformat(const Document::Ptr &doc, int indentSize, int tabSize, bool splitLongLines = false);
}
