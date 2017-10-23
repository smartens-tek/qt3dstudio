/****************************************************************************
**
** Copyright (C) 2002 NVIDIA Corporation.
** Copyright (C) 2017 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of Qt 3D Studio.
**
** $QT_BEGIN_LICENSE:GPL-EXCEPT$
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
** $QT_END_LICENSE$
**
****************************************************************************/

//==============================================================================
// Prefix
//==============================================================================
#include "stdafx.h"

//==============================================================================
// Includes
//==============================================================================
#include "CoreUtils.h"
#include "StudioPreferences.h"
#include "StudioClipboard.h"
#include "Pt.h"

#include <QtWidgets/qapplication.h>
#include <QtWidgets/qdesktopwidget.h>
#include <QtGui/qdesktopservices.h>
#include <QtGui/qscreen.h>
#include <QtGui/qwindow.h>
#include <QtCore/qurl.h>

//==============================================================================
/**
 *	FormatTimeString: Format a time string.
 *	@param	inTimeMS	Time in milliseconds.
 *	@return The formatted time string in MM:SS:MS format.
 */
Q3DStudio::CString FormatTimeString(long inTimeMS)
{
    BOOL theNegativeFlag = (inTimeMS < 0);
    long theTimeMS = abs(inTimeMS);
    Q3DStudio::CString theTimeString;
    long theMM, theSS;

    // Format the time in MM:SS:MS format

    // Get the MM value
    theMM = theTimeMS / 60000;
    theTimeMS -= (theMM * 60000);

    // Get the SS value
    theSS = theTimeMS / 1000;
    theTimeMS -= (theSS * 1000);

    // Remainder is MS value

    // Format the string
    theTimeString.Format(_LSTR("%d:%0.2d.%0.2d"), theMM, theSS, theTimeMS / 10);

    // If the original time was negative, append the "-" to the front of the time string.
    if (theNegativeFlag) {
        theTimeString.Insert(0, "-");
    }

    return theTimeString;
}

//==============================================================================
/**
 *	Checks a string to determine if it is numeric.
 *	@param	inString	String to check for all numeric characters.
 *	@return	TRUE if the string is numeric.
 */
bool IsNumericString(const Q3DStudio::CString &inString)
{
    Q3DStudio::CString theNumbers = "0123456789";
    long theLoop;
    bool theNumericFlag = true;

    // Iterate through the entire string
    for (theLoop = 0; theLoop < inString.Length() && theNumericFlag; theLoop++) {
        // Check each character for being numeric
        if (theNumbers.Find(inString.Extract(theLoop, 1)) == Q3DStudio::CString::ENDOFSTRING)
            theNumericFlag = false;
    }

    return theNumericFlag;
}

//=============================================================================
/**
 * @return The available resolution in pixels of the display index "screen"
 * (minus the Dock/Taskbar, etc.). Default is current primary display.
 */
QSize GetAvailableDisplaySize(int screen)
{
    return QApplication::desktop()->availableGeometry(screen).size();
}

//=============================================================================
/**
* @return The total resolution in pixels of the  display index "screen".
* Default is the current primary display.
 */
QSize GetDisplaySize(int screen)
{
    return QApplication::desktop()->screenGeometry(screen).size();

}

//=============================================================================
/**
* @return The index of the screen containing "widget".
 */
int getWidgetScreen(QWidget *widget)
{
    return QApplication::desktop()->screenNumber(widget);
}

//=============================================================================
/**
 * Helper function to adjust the point for the color popup dialog so that it
 * ends up in the right place.  Adjust the given point if it is near the left
 * or bottom part of the screen.
 * @param ioPoint upper-left corner of the color popup dialog; will be adjust if necessary on return
 */
void TranslatePoint(QPoint &ioPoint, const QPoint &inSize)
{
    long theBuffer = 10; // Just because the taskbar seems to overlap the dialog by a little bit
    QPoint theDlgSize(150, 260); // Size of color popup dialog - note that it's hard-coded
    QSize theScreenSize = GetAvailableDisplaySize(-1);
    long theVertOffset = theDlgSize.y() - inSize.y();
    long theHorizOffset = theDlgSize.x();

    // If the point is too close to both the left side and the bottom of the screen, adjust both the
    // x and y values
    if ((ioPoint.y() > theScreenSize.height() - theVertOffset - theBuffer)
        && (ioPoint.x() >= theScreenSize.width() - theHorizOffset)) {

        ioPoint.setX(ioPoint.x() - (theHorizOffset + inSize.x()));
        ioPoint.setY(ioPoint.y() - theVertOffset);
    }
    // If the point is just too close to the bottom of the screen, adjust the y value
    else if (ioPoint.y() > theScreenSize.height()  - theVertOffset - theBuffer)
        ioPoint.setY(ioPoint.y() - theVertOffset);
    // If the point is just too close to the left side of the screen, adjust the x value
    else if (ioPoint.x() >= theScreenSize.width() - theHorizOffset)
        ioPoint.setX(ioPoint.x() - (theHorizOffset + inSize.x()));
}

long TimeToPos(long inTime, double inTimeRatio)
{
    return ::dtol(inTime * inTimeRatio) + CStudioPreferences::GUTTER_SIZE;
}

long TimeToPos(double inTime, double inTimeRatio)
{
    return ::dtol(inTime * inTimeRatio) + CStudioPreferences::GUTTER_SIZE;
}

long PosToTime(long inPos, double inTimeRatio)
{
    return ::dtol((inPos - CStudioPreferences::GUTTER_SIZE) / inTimeRatio);
}

//=============================================================================
/**
 *	opens the url in the web browser
 *
 *	@param	inURL
 *			the URL to open
 *
 *	@return	void
 */
void ShowURLInBrowser(const Q3DStudio::CString &inURL)
{
    QDesktopServices::openUrl(QUrl(inURL.toQString()));
}

QString resourcePath()
{
    return QStringLiteral(":/res");
}

QString resourceImagePath()
{
    return QStringLiteral(":/images/");
}

QString resourceImageUrl()
{
    return QStringLiteral("qrc:/images/");
}

// Returns the qml import path required for binary installations
QString qmlImportPath()
{
    QString extraImportPath(QStringLiteral("%1/qml"));
    return extraImportPath.arg(QApplication::applicationDirPath());
}

qreal devicePixelRatio()
{
    static qreal pixelRatio = 0.0;
    if (Q_UNLIKELY(pixelRatio == 0.0)) {
        const QWindowList list = QGuiApplication::topLevelWindows();
        if (list.size() > 0)
            pixelRatio = list[0]->devicePixelRatio();
    }
    return pixelRatio;
}
