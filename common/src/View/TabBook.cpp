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
#include "CollectionUtils.h"
#include "View/TabBar.h"
#include "SplitterWindow2.h"

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
            switchToPage(page);
        }
        
        void TabBook::switchToPage(TabBookPage* page) {
            ensure(VectorUtils::contains(m_allPages, page), "must contain the requested page");
            
            auto bookIndex = m_tabBook->FindPage(page);
            if (bookIndex != wxNOT_FOUND) {
                // bookIndex will be wxNOT_FOUND if the tab is pinned
                m_tabBook->SetSelection(static_cast<size_t>(bookIndex));
            }
        }

        void TabBook::pinTab(TabBookPage* page) {
            ensure(m_pinningBehaviour != Pinning::None, "pinning should be enabled");
            wxLogDebug("pin tab %p", static_cast<void*>(page));
            
            const auto tabBookIndex = m_tabBook->FindPage(page);
            ensure(tabBookIndex != wxNOT_FOUND, "page should be in the tab book in order to pin it");
            
            m_tabBar->removeTab(page);
            
            // reparent it
            
            m_tabBook->RemovePage(static_cast<size_t>(tabBookIndex));
            
            page->Reparent(this);
            page->Show();
            
            // add it to the layout
            
            TabBar* pinnedPageTabBar = new TabBar(this);
            pinnedPageTabBar->addTab(page, "Some pinned tab!");
            
            wxBoxSizer* pinnedPageSizer = new wxBoxSizer(wxVERTICAL);
            pinnedPageSizer->Add(pinnedPageTabBar, 0, wxEXPAND);
            pinnedPageSizer->Add(page, 1, wxEXPAND);
            
            m_outerSizer->Prepend(pinnedPageSizer, 1, wxEXPAND);
            
            //Layout();

            // Search for a parent wxWidget implementing the ChildSizeRequestHandler protocol and ask it to resize us 

            wxSize wantedSize = GetSize();
            wantedSize.Scale(2.0, 1.0);

            for (wxWindow* parent = GetParent(); parent != nullptr; parent = parent->GetParent()) {
                if (ChildSizeRequestHandler* childSizeRequestHandler = dynamic_cast<ChildSizeRequestHandler*>(parent); childSizeRequestHandler) {
                    childSizeRequestHandler->childSizeRequest(this, wantedSize);
                    break;
                }
            }
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
