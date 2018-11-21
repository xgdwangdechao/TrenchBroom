/*
 Copyright (C) 2010-2017 Kristian Duske
 
 This file is part of TrenchBroom.
 
 TrenchBroom is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 
 TrenchBroom is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 
 You should have received a copy of the GNU General Public License
 along with TrenchBroom. If not, see <http://www.gnu.org/licenses/>.
 */

#include "TabBar.h"

#include "Macros.h"
#include "CollectionUtils.h"
#include "View/TabBook.h"
#include "View/ViewConstants.h"

#include <wx/button.h>
#include <wx/settings.h>
#include <wx/simplebook.h>
#include <wx/sizer.h>
#include <wx/statline.h>

#include <cassert>
#include <iostream>

namespace TrenchBroom {
    namespace View {
        TabBarButton::TabBarButton(wxWindow* parent, const wxString& label) :
        wxStaticText(parent, wxID_ANY, label),
        m_pressed(false) {
            SetFont(GetFont().Bold());
            Bind(wxEVT_LEFT_DOWN, &TabBarButton::OnClick, this);
        }
        
        void TabBarButton::setPressed(const bool pressed) {
            m_pressed = pressed;
            updateLabel();
        }

        void TabBarButton::OnClick(wxMouseEvent& event) {
            if (IsBeingDeleted()) return;

            wxCommandEvent commandEvent(wxEVT_BUTTON, GetId());
            commandEvent.SetEventObject(this);
            ProcessEvent(commandEvent);
        }

        void TabBarButton::updateLabel() {
            if (m_pressed)
                SetForegroundColour(Colors::highlightText());
            else
                SetForegroundColour(Colors::defaultText());
            Refresh();
        }

        TabBar::TabBar(TabBook* tabBook) :
        ContainerBar(tabBook, wxBOTTOM),
        m_tabBook(tabBook),
        m_barBook(new wxSimplebook(this)),
        m_controlSizer(new wxBoxSizer(wxHORIZONTAL)) {
            ensure(m_tabBook != nullptr, "tabBook is null");
            m_tabBook->Bind(wxEVT_COMMAND_BOOKCTRL_PAGE_CHANGED, &TabBar::OnTabBookPageChanged, this);

            m_controlSizer->AddSpacer(LayoutConstants::TabBarBarLeftMargin);
            m_controlSizer->AddStretchSpacer();
            m_controlSizer->Add(m_barBook, 0, wxALIGN_CENTER_VERTICAL);
            m_controlSizer->AddSpacer(LayoutConstants::NarrowHMargin);
            
            wxSizer* outerSizer = new wxBoxSizer(wxVERTICAL);
            outerSizer->AddSpacer(LayoutConstants::NarrowHMargin);
            outerSizer->Add(m_controlSizer, 1, wxEXPAND);
            outerSizer->AddSpacer(LayoutConstants::NarrowHMargin);
            
            SetSizer(outerSizer);
        }
        
        void TabBar::addTab(TabBookPage* bookPage, const wxString& title) {
            ensure(bookPage != nullptr, "bookPage is null");
            m_bookPages.push_back(bookPage);
            
            TabBarButton* button = new TabBarButton(this, title);
            button->Bind(wxEVT_BUTTON, &TabBar::OnButtonClicked, this);
            if (m_tabBook->pinningBehaviour() != TabBook::Pinning::None) {
                button->Bind(wxEVT_CONTEXT_MENU, &TabBar::OnButtonContextMenu, this);
            }
            button->setPressed(m_buttons.empty());
            m_buttons.push_back(button);
            
            const size_t sizerIndex = 2 * (m_buttons.size() - 1) + 1;
            m_controlSizer->Insert(sizerIndex, button, 0, wxALIGN_CENTER_VERTICAL);
            m_controlSizer->InsertSpacer(sizerIndex + 1, LayoutConstants::WideHMargin);
            
            wxWindow* barPage = bookPage->createTabBarPage(m_barBook);
            m_barPages.push_back(barPage);
            m_barBook->AddPage(barPage, title);
            
            Layout();
        }
        
        void TabBar::removeTab(TabBookPage* bookPage) {
            ensure(bookPage != nullptr, "bookPage is null");
            
            const auto index = VectorUtils::indexOf(m_bookPages, bookPage);
            
            auto *button = m_buttons.at(index);
            
            VectorUtils::erase(m_bookPages, index);
            VectorUtils::erase(m_buttons, index);
            VectorUtils::erase(m_barPages, index);
            
            // remove button
            button->Destroy();
            
            // remove bar page from m_barBook
            m_barBook->DeletePage(index);
        
            Layout();
        }
        
        void TabBar::OnButtonClicked(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            wxWindow* button = static_cast<wxWindow*>(event.GetEventObject());
            const size_t index = findButtonIndex(button);
            ensure(index < m_buttons.size(), "index out of range");
            m_tabBook->switchToPage(index);
        }

        void TabBar::OnButtonContextMenu(wxContextMenuEvent& event) {
            if (IsBeingDeleted()) return;
            
            wxWindow* button = static_cast<wxWindow*>(event.GetEventObject());
            const size_t index = findButtonIndex(button);
            ensure(index < m_buttons.size(), "index out of range");
            
            wxMenu popupMenu;
            const auto pinTabId = PinTabBaseId + static_cast<int>(index);
            popupMenu.Append(pinTabId, "Pin");
            popupMenu.Bind(wxEVT_MENU, &TabBar::OnPinTab, this, pinTabId);
            PopupMenu(&popupMenu);
        }
        
        void TabBar::OnPinTab(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;
            
            const auto index = static_cast<size_t>(event.GetId() - PinTabBaseId);
            auto* bookPage = m_bookPages.at(index);
            
            m_tabBook->pinTab(bookPage);
        }
        
        void TabBar::OnTabBookPageChanged(wxBookCtrlEvent& event) {
            if (IsBeingDeleted()) return;

            const int oldIndex = event.GetOldSelection();
            const int newIndex = event.GetSelection();
            
            if (oldIndex != wxNOT_FOUND && static_cast<size_t>(oldIndex) < m_buttons.size()) {
                setButtonInactive(oldIndex);
            }
            if (newIndex != wxNOT_FOUND && static_cast<size_t>(newIndex) < m_buttons.size()) {
                setButtonActive(newIndex);
                m_barBook->SetSelection(static_cast<size_t>(newIndex));
            }
        }

        size_t TabBar::findButtonIndex(wxWindow* button) const {
            for (size_t i = 0; i < m_buttons.size(); ++i) {
                if (m_buttons[i] == button)
                    return i;
            }
            return m_buttons.size();
        }

        void TabBar::setButtonActive(const int index) {
            m_buttons.at(static_cast<size_t>(index))->setPressed(true);
        }
        
        void TabBar::setButtonInactive(const int index) {
            m_buttons.at(static_cast<size_t>(index))->setPressed(false);
        }
    }
}
