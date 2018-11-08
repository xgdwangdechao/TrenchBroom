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

#ifndef TrenchBroom_Preference
#define TrenchBroom_Preference

#include "Color.h"
#include "Exceptions.h"
#include "Macros.h"
#include "StringUtils.h"
#include "IO/Path.h"
#include "View/KeyboardShortcut.h"

#include <memory>

#include <wx/config.h>
#include <wx/confbase.h>
#include <wx/tokenzr.h>

namespace TrenchBroom {
    class PreferenceSerializerBase {
    public:
        static wxString pathToConfigKey(const IO::Path& path) {
            ensure(!path.isAbsolute(), "TrenchBroom config paths must be relative");

            // Prepend a / to make it an absolute path. wxConfig has a concept of relative paths and a
            // current path, which are bug-prone (#2438), so never use relative paths.
            wxString key;
            key << "/";
            key << path.asString("/");
            return key;
        }

        static bool readString(wxConfigBase* config, const IO::Path& path, wxString* valueOut) {
            return config->Read(pathToConfigKey(path), valueOut);
        }

        static bool writeString(wxConfigBase* config, const IO::Path& path, const wxString& value) {
            return config->Write(pathToConfigKey(path), value);
        }
    };

    template <typename T>
    class PreferenceSerializer : public PreferenceSerializerBase {
    public:
        bool read(wxConfigBase* config, const IO::Path& path, T& result) const { return false; }
        bool write(wxConfigBase* config, const IO::Path& path, const T& value) const { return false; }
    };
    
    template <>
    class PreferenceSerializer<bool> : public PreferenceSerializerBase {
    public:
        bool read(wxConfigBase* config, const IO::Path& path, bool& result) const {
            wxString string;
            if (readString(config, path, &string)) {
                long longValue = 0;
                if (string.ToLong(&longValue)) {
                    result = longValue != 0L;
                    return true;
                }
            }
            return false;
        }
        
        bool write(wxConfigBase* config, const IO::Path& path, const bool& value) const {
            wxString str;
            str << (value ? 1 : 0);
            return writeString(config, path, str);
        }
    };
    
    template <>
    class PreferenceSerializer<int> : public PreferenceSerializerBase {
    public:
        bool read(wxConfigBase* config, const IO::Path& path, int& result) const {
            wxString string;
            if (readString(config, path, &string)) {
                long longValue = 0;
                if (string.ToLong(&longValue) && longValue >= std::numeric_limits<int>::min() && longValue <= std::numeric_limits<int>::max()) {
                    result = static_cast<int>(longValue);
                    return true;
                }
            }
            return false;
        }
        
        bool write(wxConfigBase* config, const IO::Path& path, const int& value) const {
            wxString str;
            str << value;
            return writeString(config, path, str);
        }
    };
    
    template <>
    class PreferenceSerializer<float> : public PreferenceSerializerBase {
    public:
        bool read(wxConfigBase* config, const IO::Path& path, float& result) const {
            wxString string;
            if (readString(config, path, &string)) {
                double doubleValue = 0.0;
                if (string.ToDouble(&doubleValue) && doubleValue >= std::numeric_limits<float>::min() && doubleValue <= std::numeric_limits<float>::max()) {
                    result = static_cast<float>(doubleValue);
                    return true;
                }
            }
            return false;
        }
        
        bool write(wxConfigBase* config, const IO::Path& path, const float& value) const {
            wxString str;
            str << value;
            return writeString(config, path, str);
        }
    };
    
    template <>
    class PreferenceSerializer<double> : public PreferenceSerializerBase {
    public:
        bool read(wxConfigBase* config, const IO::Path& path, double& result) const {
            wxString string;
            if (readString(config, path, &string)) {
                double doubleValue = 0.0;
                if (string.ToDouble(&doubleValue)) {
                    result = doubleValue;
                    return true;
                }
            }
            return false;
        }
        
        bool write(wxConfigBase* config, const IO::Path& path, const double& value) const {
            wxString str;
            str << value;
            return writeString(config, path, str);
        }
    };
    
    template <>
    class PreferenceSerializer<String> : public PreferenceSerializerBase {
    public:
        bool read(wxConfigBase* config, const IO::Path& path, String& result) const {
            wxString string;
            if (readString(config, path, &string)) {
                result = string.ToStdString();
                return true;
            }
            return false;
        }
        
        bool write(wxConfigBase* config, const IO::Path& path, const String& value) const {
            wxString str;
            str << value;
            return writeString(config, path, str);
        }
    };
    
    template <>
    class PreferenceSerializer<Color> : public PreferenceSerializerBase {
    public:
        bool read(wxConfigBase* config, const IO::Path& path, Color& result) const {
            wxString string;
            if (readString(config, path, &string)) {
                result = Color::parse(string.ToStdString());
                return true;
            }
            return false;
        }
        
        bool write(wxConfigBase* config, const IO::Path& path, const Color& value) const {
            wxString str;
            str << StringUtils::toString(value);
            return writeString(config, path, str);
        }
    };
    
    template<>
    class PreferenceSerializer<View::KeyboardShortcut> : public PreferenceSerializerBase {
    public:
        bool read(wxConfigBase* config, const IO::Path& path, View::KeyboardShortcut& result) const {
            wxString string;
            if (readString(config, path, &string)) {
                result = View::KeyboardShortcut(string);
                return true;
            }
            return false;
        }
        
        bool write(wxConfigBase* config, const IO::Path& path, const View::KeyboardShortcut& value) const {
            wxString str;
            str << value.asString();
            return writeString(config, path, str);
        }
    };
    
    template<>
    class PreferenceSerializer<IO::Path> : public PreferenceSerializerBase {
    public:
        bool read(wxConfigBase* config, const IO::Path& path, IO::Path& result) const {
            wxString string;
            if (readString(config, path, &string)) {
                result = IO::Path(string.ToStdString());
                return true;
            }
            return false;
        }
        
        bool write(wxConfigBase* config, const IO::Path& path, const IO::Path& value) const {
            wxString str;
            str << value.asString();
            return writeString(config, path, str);
        }
    };

    class PreferenceBase {
    public:
        typedef std::set<const PreferenceBase*> Set;
        PreferenceBase() {}

        PreferenceBase(const PreferenceBase& other) {}
        virtual ~PreferenceBase() {}
        
        PreferenceBase& operator=(const PreferenceBase& other) { return *this; }
        
        virtual void load(wxConfigBase* config) const = 0;
        virtual void save(wxConfigBase* config) = 0;
        virtual void resetToPrevious() = 0;

        bool operator==(const PreferenceBase& other) const {
            return this == &other;
        }

        virtual const IO::Path& path() const = 0;
    };
    
    
    template <typename T>
    class Preference : public PreferenceBase {
    protected:
        friend class PreferenceManager;
        template<typename> friend class SetTemporaryPreference;
        
        PreferenceSerializer<T> m_serializer;
        IO::Path m_path;
        T m_defaultValue;
        mutable T m_value;
        mutable T m_previousValue;
        mutable bool m_initialized;
        bool m_modified;
        
        void setValue(const T& value) {
            if (!m_modified) {
                m_modified = true;
                m_previousValue = m_value;
            }
            m_value = value;
        }

        bool initialized() const {
            return m_initialized;
        }
        
        void load(wxConfigBase* config) const override {
            using std::swap;
            T temp;
            if (m_serializer.read(config, m_path, temp)) {
                std::swap(m_value, temp);
                m_previousValue = m_value;
            }
            m_initialized = true;
        }
        
        void save(wxConfigBase* config) override {
            if (m_modified) {
                assertResult(m_serializer.write(config, m_path, m_value));
                m_modified = false;
                m_previousValue = m_value;
            }
        }

        void resetToPrevious() override {
            if (m_modified) {
                m_value = m_previousValue;
                m_modified = false;
            }
        }
    public:
        Preference(const IO::Path& path, const T& defaultValue) :
        m_path(path),
        m_defaultValue(defaultValue),
        m_value(m_defaultValue),
        m_previousValue(m_value),
        m_initialized(false),
        m_modified(false) {
            m_modified = m_initialized;
        }
        
        Preference(const Preference& other) :
        PreferenceBase(other),
        m_path(other.m_path),
        m_defaultValue(other.m_defaultValue),
        m_value(other.m_value),
        m_previousValue(other.m_previousValue),
        m_initialized(other.m_initialized),
        m_modified(other.m_modified) {}
        
        Preference& operator=(Preference other) {
            using std::swap;
            swap(*this, other);
            return *this;
        }

        friend void swap(Preference& lhs, Preference& rhs) {
            using std::swap;
            swap(lhs.m_path, rhs.m_path);
            swap(lhs.m_defaultValue, rhs.m_defaultValue);
            swap(lhs.m_value, rhs.m_value);
            swap(lhs.m_previousValue, rhs.m_previousValue);
            swap(lhs.m_initialized, rhs.m_initialized);
            swap(lhs.m_modified, rhs.m_modified);
        }
        
        const IO::Path& path() const override {
            return m_path;
        }
        
        const T& defaultValue() const {
            return m_defaultValue;
        }
        
        const T& value() const {
            return m_value;
        }
    };
}

#endif /* defined(TrenchBroom_Preference) */
