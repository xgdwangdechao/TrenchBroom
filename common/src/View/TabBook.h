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

#ifndef TrenchBroom_TabBook
#define TrenchBroom_TabBook

#include <wx/panel.h>

#include <vector>

class wxBookCtrlEvent;
class wxSimplebook;

namespace TrenchBroom {
    namespace View {
        class TabBar;

        class TabBookPage : public wxPanel {
        public:
            TabBookPage(wxWindow* parent);
            virtual ~TabBookPage();
            virtual wxWindow* createTabBarPage(wxWindow* parent);
        };
        
        class TabBook : public wxPanel {
        public:
            enum class Pinning {
                None, Vertical
            };
        private:
            /**
             * Includes pinned and unpinned tabs
             */
            std::vector<TabBookPage*> m_allPages;
            /**
             * Horizontal sizer for the pinned tabs, followed by the tab book
             * of unpinned tabs
             */
            wxBoxSizer* m_outerSizer;
            
            wxBoxSizer* m_tabBarAndBookSizer;
            TabBar* m_tabBar;
            wxSimplebook* m_tabBook;
            
            Pinning m_pinningBehaviour;
        public:
            TabBook(wxWindow* parent, Pinning pinningBehaviour = Pinning::None);
            
            void addPage(TabBookPage* page, const wxString& title);
            void switchToPage(size_t index);
            void pinTab(TabBookPage* page);
            void setTabBarHeight(int height);
            Pinning pinningBehaviour() const;
            
            void OnTabBookPageChanged(wxBookCtrlEvent& event);
        };
    }
}

#endif /* defined(TrenchBroom_TabBook) */
