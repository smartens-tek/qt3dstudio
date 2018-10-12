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

#include "Qt3DSCommonPrecompile.h"
#include "ui_StudioProjectSettingsPage.h"
#include "StudioProjectSettingsPage.h"
#include "StudioProjectSettings.h"
#include "StudioApp.h"
#include "Doc.h"
#include "Views.h"
#include "MainFrm.h"
#include "CommonConstants.h"
#include "StudioPreferences.h"
#include "Core.h"

CStudioProjectSettingsPage::CStudioProjectSettingsPage(QWidget *parent)
    : CStudioPreferencesPropPage(parent)
    , m_AspectRatio(0.0f)
    , m_ui(new Ui::StudioProjectSettingsPage)
{
    m_Font = QFont(CStudioPreferences::GetFontFaceName());
    m_Font.setPixelSize(CStudioPreferences::fontSize());

    // Create a bold font for the group box text
    m_BoldFont = m_Font;
    m_BoldFont.setBold(true);

    onInitDialog();
}


CStudioProjectSettingsPage::~CStudioProjectSettingsPage()
{
}

/**
 * OnInitDialog: Handle the WM_INITDIALOG message.
 *
 * Initialize the dialog by setting the various control values.
 *
 * @return Returns TRUE always.
 */
void CStudioProjectSettingsPage::onInitDialog()
{
    m_ui->setupUi(this);

    m_ui->m_PresentationId->setToolTip(tr("Presentation Id"));
    m_ui->m_ClientSizeWidth->setToolTip(tr("Presentation Width"));
    m_ui->m_ClientSizeHeight->setToolTip(tr("Presentation Height"));
    m_ui->m_checkConstrainProportions->setToolTip(tr("Check to maintain the aspect ratio when "
                                                     "changing presentation width or height"));
    m_ui->m_Author->setToolTip(tr("Enter an author name for this presentation"));
    m_ui->m_Company->setToolTip(tr("Enter a company name for this presentation"));

    // Set fonts for child windows.
    for (auto w : findChildren<QWidget *>())
        w->setFont(m_Font);

    // Make the group text bold
    for (auto w : findChildren<QGroupBox *>())
        w->setFont(m_BoldFont);

    // Set the ranges of the client width and height
    m_ui->m_ClientSizeWidth->setRange(1, 16384);
    m_ui->m_ClientSizeHeight->setRange(1, 16384);

    // Load the settings for the controls
    this->LoadSettings();

    auto valueChanged = static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged);
    connect(m_ui->m_PresentationId, &QLineEdit::textEdited,
            this, &CStudioProjectSettingsPage::onChangePresentationId);
    connect(m_ui->m_ClientSizeWidth, valueChanged,
            this, &CStudioProjectSettingsPage::onChangeEditPresWidth);
    connect(m_ui->m_ClientSizeHeight, valueChanged,
            this, &CStudioProjectSettingsPage::onChangeEditPresHeight);
    connect(m_ui->m_checkConstrainProportions, &QCheckBox::clicked,
            this, &CStudioProjectSettingsPage::onCheckMaintainRatio);
    connect(m_ui->m_Author, &QLineEdit::textEdited,
            this, &CStudioProjectSettingsPage::onChangeAuthor);
    connect(m_ui->m_Company, &QLineEdit::textEdited,
            this, &CStudioProjectSettingsPage::onChangeCompany);
}

// LoadSettings: Load the settings from the project settings and set the control values.
void CStudioProjectSettingsPage::LoadSettings()
{
    // Presentation Id
    m_ui->m_PresentationId->setText(g_StudioApp.GetCore()->GetDoc()->getPresentationId());

    // Get the Client size
    CStudioProjectSettings *theProjectSettings = g_StudioApp.GetCore()->GetStudioProjectSettings();
    CPt theClientSize = theProjectSettings->GetPresentationSize();

    // Set client width & height
    m_ui->m_ClientSizeWidth->setValue(theClientSize.x);
    m_ui->m_ClientSizeHeight->setValue(theClientSize.y);

    // Save the aspect ratio
    m_AspectRatio = (double)theClientSize.x / (double)theClientSize.y;

    // Maintain Aspect Ratio checkbox
    m_ui->m_checkConstrainProportions->setChecked(theProjectSettings->GetMaintainAspect());

    m_ui->m_checkPortraitFormat->setChecked(theProjectSettings->GetRotatePresentation());

    // Author
    m_ui->m_Author->setText(theProjectSettings->GetAuthor());

    // Company
    m_ui->m_Company->setText(theProjectSettings->GetCompany());
}

// SaveSettings: Save the settings from the controls to the project settings.
void CStudioProjectSettingsPage::SaveSettings()
{
    CPt theClientSize;
    CStudioProjectSettings *theProjectSettings = g_StudioApp.GetCore()->GetStudioProjectSettings();

    // Presentation Id
    g_StudioApp.GetCore()->getProjectFile().writePresentationId(m_ui->m_PresentationId->text());

    // Presentation width & height
    theClientSize.x = m_ui->m_ClientSizeWidth->value();
    theClientSize.y = m_ui->m_ClientSizeHeight->value();
    theProjectSettings->SetPresentationSize(theClientSize);

    // Author
    QString theAuthor = m_ui->m_Author->text();
    theProjectSettings->SetAuthor(theAuthor);

    // Company
    QString theCompany = m_ui->m_Company->text();
    theProjectSettings->SetCompany(theCompany);

    g_StudioApp.GetViews()->recheckMainframeSizingMode();

    // Maintain Aspect Ratio checkbox
    theProjectSettings->SetMaintainAspect(m_ui->m_checkConstrainProportions->isChecked());

    theProjectSettings->SetRotatePresentation(m_ui->m_checkPortraitFormat->isChecked());
}

// Generic function when settings are modified.
void CStudioProjectSettingsPage::onSettingsModified()
{
    this->setModified(TRUE);
}

// OnApply: Handler for the Apply button
bool CStudioProjectSettingsPage::onApply()
{
    // make sure the presentation Id is unique and not empty
    if (m_ui->m_PresentationId->text().isEmpty()) {
        g_StudioApp.showPresentationIdEmptyWarning();
        return false;
    }
    if (!g_StudioApp.GetCore()->getProjectFile()
            .isUniquePresentationId(m_ui->m_PresentationId->text())) {
        g_StudioApp.showPresentationIdUniqueWarning();
        return false;
    }

    // Apply was clicked - save settings and disabled the Apply button
    this->SaveSettings();

    this->setModified(FALSE);

    return CStudioPreferencesPropPage::onApply();
}

// OnChangeEditPresWidth: EN_CHANGE handler for the IDC_EDIT_PRESWIDTH field
void CStudioProjectSettingsPage::onChangeEditPresWidth()
{
    this->setModified(TRUE);

    // Should the aspect ratio be maintained?
    if (m_ui->m_checkConstrainProportions->isChecked()) {
        long thePresWidth, thePresHeight;

        thePresWidth = m_ui->m_ClientSizeWidth->value();

        // Change the height
        thePresHeight = ((double)thePresWidth / m_AspectRatio);

        QSignalBlocker sb(m_ui->m_ClientSizeHeight);
        m_ui->m_ClientSizeHeight->setValue(thePresHeight);
    }
}

// OnChangeEditPresHeight: EN_CHANGE handler for the IDC_EDIT_PRESHEIGHT field
void CStudioProjectSettingsPage::onChangeEditPresHeight()
{
    this->setModified(TRUE);

    // Should the aspect ratio be maintained?
    if (m_ui->m_checkConstrainProportions->isChecked()) {
        long thePresWidth, thePresHeight;

        thePresHeight = m_ui->m_ClientSizeHeight->value();

        // Change the width
        thePresWidth = ((double)thePresHeight * m_AspectRatio);

        QSignalBlocker sb(m_ui->m_ClientSizeWidth);
        m_ui->m_ClientSizeWidth->setValue(thePresWidth);
    }
}

// OnCheckMaintainRatio: The aspect ratio checkbox has changed.
void CStudioProjectSettingsPage::onCheckMaintainRatio()
{
    this->setModified(TRUE);

    long thePresWidth, thePresHeight;

    // Get the width and height
    thePresWidth = m_ui->m_ClientSizeWidth->value();
    thePresHeight = m_ui->m_ClientSizeHeight->value();

    // Save the Aspect Ratio
    m_AspectRatio = (double)thePresWidth / (double)thePresHeight;
}

void CStudioProjectSettingsPage::onChangePresentationId()
{
    this->setModified(TRUE);
}

// OnChangeAuthor: EN_CHANGE handler for the IDC_AUTHOR field.
void CStudioProjectSettingsPage::onChangeAuthor()
{
    this->setModified(TRUE);
}

// OnChangeCompany: EN_CHANGE handler for the IDC_COMPANY field.
void CStudioProjectSettingsPage::onChangeCompany()
{
    this->setModified(TRUE);
}

// OnChangeSet1: EN_CHANGE handler for the IDC_SET1 field.
void CStudioProjectSettingsPage::onChangeSet1()
{
    this->setModified(TRUE);
}

// OnChangeSet2: EN_CHANGE handler for the IDC_SET2 field.
void CStudioProjectSettingsPage::onChangeSet2()
{
    this->setModified(TRUE);
}

// OnChangeSet3: EN_CHANGE handler for the IDC_SET3 field.
void CStudioProjectSettingsPage::onChangeSet3()
{
    this->setModified(TRUE);
}

// OnChangeSet5: EN_CHANGE handler for the IDC_SET5 field.
void CStudioProjectSettingsPage::onChangeSet5()
{
    this->setModified(TRUE);
}
