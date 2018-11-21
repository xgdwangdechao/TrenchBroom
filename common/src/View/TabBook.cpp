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

#include "TabBook.h"

#include "Macros.h"
#include "View/TabBar.h"

#include <wx/simplebook.h>
#include <wx/sizer.h>

namespace TrenchBroom {
    namespace View {
        // TabBookPage
        
        TabBookPage::TabBookPage(wxWindow* parent) :
        wxPanel(parent) {}
        TabBookPage::~TabBookPage() {}
        
        wxWindow* TabBookPage::createTabBarPage(wxWindow* parent) {
            return new wxPanel(parent);
        }

        // TabBook
        
        TabBook::TabBook(wxWindow* parent, const Pinning pinning) :
        wxPanel(parent),
        m_tabBar(new TabBar(this)),
        m_tabBook(new wxSimplebook(this)),
        m_pinningBehaviour(pinning) {
            m_tabBook->Bind(wxEVT_COMMAND_BOOKCTRL_PAGE_CHANGED, &TabBook::OnTabBookPageChanged, this);

            m_tabBarAndBookSizer = new wxBoxSizer(wxVERTICAL);
            m_tabBarAndBookSizer->Add(m_tabBar, 0, wxEXPAND);
            m_tabBarAndBookSizer->Add(m_tabBook, 1, wxEXPAND);
            
            m_outerSizer = new wxBoxSizer(wxHORIZONTAL);
            m_outerSizer->Add(m_tabBarAndBookSizer, 1, wxEXPAND);
            SetSizer(m_outerSizer);
        }
        
        void TabBook::addPage(TabBookPage* page, const wxString& title) {
            ensure(page != nullptr, "page is null");
            assert(page->GetParent() == this);
            
            m_allPages.push_back(page);
            
            RemoveChild(page);
            page->Reparent(m_tabBook);
            m_tabBook->AddPage(page, title);
            m_tabBar->addTab(page, title);
        }

        void TabBook::switchToPage(const size_t index) {
            auto* page = m_allPages.at(index);
            
            auto bookIndex = m_tabBook->FindPage(page);
            if (bookIndex != wxNOT_FOUND) {
                // bookIndex will be wxNOT_FOUND if the tab is pinned
                m_tabBook->SetSelection(static_cast<size_t>(bookIndex));
            }
        }

        void TabBook::pinTab(TabBookPage* page) {
            wxLogDebug("pin tab %p", static_cast<void*>(page));
            
            const auto tabBookIndex = m_tabBook->FindPage(page);
            ensure(tabBookIndex != wxNOT_FOUND, "page should be in the tab book in order to pin it");
            
            m_tabBar->removeTab(page);
            
            // reparent it
            
            m_tabBook->RemovePage(static_cast<size_t>(tabBookIndex));
            
            page->Reparent(this);
            
            m_outerSizer->Prepend(page, 1, wxEXPAND);
            page->Show();
            Layout();
        }
        
        void TabBook::setTabBarHeight(const int height) {
            m_tabBarAndBookSizer->SetItemMinSize(m_tabBar, wxDefaultCoord, height);
            Layout();
        }

        TabBook::Pinning TabBook::pinningBehaviour() const {
            return m_pinningBehaviour;
        }
        
        void TabBook::OnTabBookPageChanged(wxBookCtrlEvent& event) {
            if (IsBeingDeleted()) return;

            ProcessEvent(event);
            event.Skip();
        }
    }
}
