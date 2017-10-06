#include <stdexcept>
#include <iostream>
#include <sstream>
#include <iomanip>
#include "PropertyNode.h"

using std::cout;
using std::endl;
using std::string;
using std::stringstream;


/*
 * ___________________________________________________________________________
 */
PropertyNode::PropertyNode() {
}


/*
 * ___________________________________________________________________________
 */
PropertyNode::PropertyNode(const PropertyNode& propertyNode) {

    this->propImportLocal(propertyNode);
}


/*
 * ___________________________________________________________________________
 */
size_t PropertyNode::findUnique(const std::string& prefix,
            size_t offset, size_t max) const {

    size_t guess = 1;
    size_t from = 0;

    while ((max == 0 || (offset + guess) < max)) {

        // Check whether key "<prefix><offset + guess>" already exists
        stringstream key(prefix);
        key << offset + guess;
        if (properties_.count(key.str()) == 0) {
            // >>> We found a unique name >>>
            break;
        }

        from = guess;
        // We use a form of binary search here
        guess <<= 1;
    }

    if (guess - from > 1) {
        guess = findUnique(prefix, offset + from, offset + guess);
    } else {
        guess += offset;
    }

    return guess;
}


/*
 *    A redirect hook might redirect the node and change the name requested
 * ___________________________________________________________________________
 */
bool PropertyNode::propRedirectHook_(string& name, PropertyNode*& node) const {

    return false;
}


/*
 *    A read hook might change the value returned by getProperty...
 * ___________________________________________________________________________
 */
void PropertyNode::propReadHook_(const string& name,
        DynamicValue& value) const {
}


/*
 * ___________________________________________________________________________
 */
void PropertyNode::propWriteHook_(const string& name, DynamicValue& value) {
}


/*
 * ___________________________________________________________________________
 */
template<typename T>
bool PropertyNode::propSet(const string& name, const T& value) {

    /* apply the redirect hook */
    string redirectedName = name;
    PropertyNode* redirectedNode = (PropertyNode*)0;
    if (!this->propRedirectHook_(redirectedName, redirectedNode)) {

        // Reuse string instance "redirectedName"
        redirectedName = name;

        /* create a DynamicValue instance representing the value to be set */
        DynamicValue dynVal;
        dynVal.setValue<T>(value);

        // The key might use the auto-ID feature => check for it
        if (redirectedName.find_first_of("#") == 0) {
            // Resolve potential auto-ID
            redirectedName.erase(0, 1);
            stringstream suffix;
            suffix << this->findUnique(redirectedName);
            redirectedName.append(suffix.str());
        }

        /* ... pass it to the write hook */
        this->propWriteHook_(redirectedName, dynVal);

        /* now: only store the new value if it
         * hasn't been captured by the write hook */
        if (dynVal.isDef()) {
            properties_[redirectedName] = dynVal;
        }

        return true;
    } else if (redirectedNode != 0) {
        /* redirection hook has redirected the
         * request to another property node */
        return redirectedNode->propSet<T>(redirectedName, value);
    }

    // Redirection for property failed
    return false;
}

template bool PropertyNode::propSet<int>(const string&, const int&);

template bool PropertyNode::propSet<bool>(const string&, const bool&);

template bool PropertyNode::propSet<double>(const string&, const double&);

template bool PropertyNode::propSet<string>(const string&, const string&);


/*
 * ___________________________________________________________________________
 */
bool PropertyNode::propGetDynamicValue(const std::string& name,
        DynamicValue& value) const {

    bool found = false;

    /* apply the redirect hook */
    string redirectedName = name;
    PropertyNode* redirectedNode = (PropertyNode*)0;
    if (!this->propRedirectHook_(redirectedName, redirectedNode)) {

        /* retrieve a copy of the value ... */
        DynamicValue dynVal;
        if (properties_.count(name) > 0) {
            dynVal = properties_.find(name)->second;
        }

        /* ... pass it to the read hook
         * TODO: capture cases where the read hook hides local values */
        this->propReadHook_(name, dynVal);

        if (dynVal.isDef()) {
            value = dynVal;
            found = true;
        }
    } else if (redirectedNode != 0) {
        /* redirection hook has redirected the
         * request to another property node */
        found = redirectedNode->propGetDynamicValue(redirectedName, value);
    }

    return found;
}


/*
 * ___________________________________________________________________________
 */
template<typename T>
T PropertyNode::propGet(const string& name) const {

    DynamicValue dynVal;

    bool found = this->propGetDynamicValue(name, dynVal);
    if (!found) {
        throw std::runtime_error("PropertyNode::propGet<T>(...): "
                "Property not found: " + name);
    }

    return dynVal.getValue<T>();
}

template int PropertyNode::propGet<int>(const string&) const;

template bool PropertyNode::propGet<bool>(const string&) const;

template double PropertyNode::propGet<double>(const string&) const;

template string PropertyNode::propGet<string>(const string&) const;


/*
 * ___________________________________________________________________________
 */
template<typename T>
T PropertyNode::propGetDefault(const std::string& name, const T& defValue) const {

    T value = defValue;
    DynamicValue dynVal;

    bool found = this->propGetDynamicValue(name, dynVal);
    if (found) {
        value = dynVal.getValue<T>();
    }

    return value;
}

template int PropertyNode::propGetDefault<int>(
        const string&, const int& defValue) const;

template bool PropertyNode::propGetDefault<bool>(
        const string&, const bool& defValue) const;

template double PropertyNode::propGetDefault<double>(
        const string&, const double& defValue) const;

template string PropertyNode::propGetDefault<string>(
        const string&, const string& defValue) const;


/*
 * ___________________________________________________________________________
 */
template<typename T>
bool PropertyNode::propGet(const string& name, T& value) const {

    DynamicValue dynVal;

    bool found = this->propGetDynamicValue(name, dynVal);
    if (found) {
        value = dynVal.getValue<T>();
    }

    return found;
}

template bool PropertyNode::propGet<int>(const string&, int& value) const;

template bool PropertyNode::propGet<bool>(const string&, bool& value) const;

template bool PropertyNode::propGet<double>(const string&, double& value) const;

template bool PropertyNode::propGet<string>(const string&, string& value) const;


/*
 * ___________________________________________________________________________
 */
size_t PropertyNode::propImportLocal(const PropertyNode& properties,
        const std::string& prefix, bool override) {

    const map_type_t& props = properties.propGetLocal();

    size_t n = 0;
    for (map_type_t::const_iterator it = props.begin();
            it != props.end(); ++it) {

        /* prepend name prefix */
        string name = prefix + it->first;

        /* set property */
        if (override || properties_.count(name) == 0) {
            properties_[name] = it->second;
            ++n;
        }
    }

    return n;
}


/*
 * ___________________________________________________________________________
 */
void PropertyNode::propPrintLocal() const {

    std::ostream& stream = cout;

    // In a first step: determine the column widths we need
    size_t keyColWidth = 10;
    size_t valColWidth = 10;
    for (map_type_t::const_iterator it = properties_.begin();
            it != properties_.end(); ++it) {
        size_t len = it->first.length();
        if (len > keyColWidth) {
            keyColWidth = len;
        }
        len = it->second.getValue<string>().length();
        if (len > valColWidth) {
            valColWidth = len;
        }
    }
    keyColWidth += 2;
    valColWidth += 2;

    // Assemble headline
    stream << std::left << std::setw(keyColWidth) << "Key";
    stream << std::left << std::setw(valColWidth) << "Value" << endl;
    string line;
    line.append(keyColWidth + valColWidth, '=');
    stream << line << "\n";

    // Assemble body
    for (map_type_t::const_iterator it = properties_.begin();
            it != properties_.end(); ++it) {
        stream << std::left << std::setw(keyColWidth) << it->first;
        stream << std::left << std::setw(valColWidth)
                << it->second.getValue<string>() << endl;
    }
}


/*
 * ___________________________________________________________________________
 */
string PropertyNode::propGetAsText() const {

    string text;

    for (map_type_t::const_iterator it = properties_.begin();
            it != properties_.end(); ++it) {
        stringstream ss;
        ss << "<";
        ss << it->first;
        ss << " = ";
        ss << it->second.getValue<string>();
        ss << ">\n";
        text.append(ss.str());
    }

    return text;
}


/*
 * ___________________________________________________________________________
 */
PropertyNode::~PropertyNode() {
}

