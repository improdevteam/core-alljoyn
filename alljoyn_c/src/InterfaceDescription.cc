/**
 * @file
 *
 * This file implements the InterfaceDescription class
 */

/******************************************************************************
 *    Copyright (c) Open Connectivity Foundation (OCF), AllJoyn Open Source
 *    Project (AJOSP) Contributors and others.
 *
 *    SPDX-License-Identifier: Apache-2.0
 *
 *    All rights reserved. This program and the accompanying materials are
 *    made available under the terms of the Apache License, Version 2.0
 *    which accompanies this distribution, and is available at
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Copyright (c) Open Connectivity Foundation and Contributors to AllSeen
 *    Alliance. All rights reserved.
 *
 *    Permission to use, copy, modify, and/or distribute this software for
 *    any purpose with or without fee is hereby granted, provided that the
 *    above copyright notice and this permission notice appear in all
 *    copies.
 *
 *    THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL
 *    WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED
 *    WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE
 *    AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
 *    DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR
 *    PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 *    TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 *    PERFORMANCE OF THIS SOFTWARE.
 ******************************************************************************/

#include <alljoyn/InterfaceDescription.h>
#include <alljoyn/Translator.h>
#include <alljoyn_c/InterfaceDescription.h>
#include <alljoyn_c/Status.h>
#include <qcc/Debug.h>
#include <qcc/Mutex.h>
#include <vector>

#define QCC_MODULE "ALLJOYN_C"

namespace ajn {

class TranslatorC : public Translator {
  public:
    TranslatorC() : translation_callback_ptr(nullptr)
    { }

    void SetTranslationCallback(alljoyn_interfacedescription_translation_callback_ptr ptr)
    {
        translation_callback_ptr = ptr;
    }

    alljoyn_interfacedescription_translation_callback_ptr GetTranslationCallback()
    {
        return translation_callback_ptr;
    }

    virtual QStatus AddTargetLanguage(const char* language, bool* added = NULL) {
        targetLanguagesLock.Lock(MUTEX_CONTEXT);
        bool addStatus = targetLanguages.insert(language).second;
        targetLanguagesLock.Unlock(MUTEX_CONTEXT);
        if (added != NULL) {
            *added = addStatus;
        }
        return ER_OK;
    }

    size_t CopyPointersToArray(const char** array, size_t size) {
        if (array == nullptr) {
            return 0;
        }

        size_t count = 0u;
        targetLanguagesLock.Lock(MUTEX_CONTEXT);
        for (std::set<qcc::String>::const_iterator itL = targetLanguages.begin();
             (itL != targetLanguages.end()) && (count < size);
             itL++) {
            array[count] = itL->c_str();
            count++;
        }
        targetLanguagesLock.Unlock(MUTEX_CONTEXT);
        return count;
    }

  private:
    virtual size_t NumTargetLanguages()
    {
        return targetLanguages.size();
    }

    virtual void GetTargetLanguage(size_t index, qcc::String& ret)
    {
        targetLanguagesLock.Lock(MUTEX_CONTEXT);
        if (index < targetLanguages.size()) {
            std::set<qcc::String>::const_iterator itL = targetLanguages.begin();
            std::advance(itL, index);
            ret = *itL;
        } else {
            ret = "";
        }
        targetLanguagesLock.Unlock(MUTEX_CONTEXT);
    }

    virtual const char* Translate(const char* sourceLanguage, const char* targetLanguage, const char* source)
    {
        if (translation_callback_ptr == nullptr) {
            return Translator::Translate(sourceLanguage, targetLanguage, source);
        }
        return translation_callback_ptr(sourceLanguage, targetLanguage, source);
    }

  private:
    // Std::set used to provide uniqueness and sorting of language tags.
    std::set<qcc::String> targetLanguages;
    qcc::Mutex targetLanguagesLock;
    alljoyn_interfacedescription_translation_callback_ptr translation_callback_ptr;
};

/* static instances of class used for translation callback */
static TranslatorC translatorC;
}

struct _alljoyn_interfacedescription_handle {
    /* Empty by design, this is just to allow the type restrictions to save coders from themselves */
};

size_t AJ_CALL alljoyn_interfacedescription_member_getannotationscount(alljoyn_interfacedescription_member member) {
    QCC_DbgTrace(("%s", __FUNCTION__));
    return ((ajn::InterfaceDescription::Member*)member.internal_member)->GetAnnotations(NULL, NULL, 0);
}

void AJ_CALL alljoyn_interfacedescription_member_getannotationatindex(alljoyn_interfacedescription_member member,
                                                                      size_t index,
                                                                      char* name, size_t* name_size,
                                                                      char* value, size_t* value_size)
{
    QCC_DbgTrace(("%s", __FUNCTION__));
    size_t annotation_size = ((ajn::InterfaceDescription::Member*)member.internal_member)->GetAnnotations(NULL, NULL, 0);
    qcc::String* inner_names = new qcc::String[annotation_size];
    qcc::String* inner_values = new qcc::String[annotation_size];

    ((ajn::InterfaceDescription::Member*)member.internal_member)->GetAnnotations(inner_names, inner_values, annotation_size);

    if ((name != nullptr) && (value != nullptr)) {
        if (*name_size >= 1) {
            strncpy(name, inner_names[index].c_str(), *name_size);
            name[*name_size - 1] = '\0';
        }

        if (*value_size >= 1) {
            strncpy(value, inner_values[index].c_str(), *value_size);
            value[*value_size - 1] = '\0';
        }
    }
    //size of the string plus the nul character
    *name_size = inner_names[index].size() + 1;
    //size of the string plus the nul character
    *value_size = inner_values[index].size() + 1;
    delete[] inner_names;
    delete[] inner_values;
}

QCC_BOOL AJ_CALL alljoyn_interfacedescription_member_getannotation(alljoyn_interfacedescription_member member, const char* name, char* value, size_t* value_size)
{
    QCC_DbgTrace(("%s", __FUNCTION__));
    QCC_BOOL return_value = QCC_FALSE;
    qcc::String out_val;
    bool b = ((ajn::InterfaceDescription::Member*)member.internal_member)->GetAnnotation(name, out_val);

    if ((value != nullptr) && (*value_size >= 1)) {
        if (b) {
            strncpy(value, out_val.c_str(), *value_size);
            value[*value_size - 1] = '\0';
            return_value = QCC_TRUE;
        } else {
            //return empty string
            *value = '\0';
        }
    }
    *value_size = out_val.size() + 1;
    return return_value;
}

size_t AJ_CALL alljoyn_interfacedescription_member_getargannotationscount(alljoyn_interfacedescription_member member, const char* argName) {
    QCC_DbgTrace(("%s", __FUNCTION__));
    return ((ajn::InterfaceDescription::Member*)member.internal_member)->GetArgAnnotations(argName, NULL, NULL, 0);
}

void AJ_CALL alljoyn_interfacedescription_member_getargannotationatindex(alljoyn_interfacedescription_member member, const char* argName,
                                                                         size_t index,
                                                                         char* name, size_t* name_size,
                                                                         char* value, size_t* value_size)
{
    QCC_DbgTrace(("%s", __FUNCTION__));
    size_t annotation_size = ((ajn::InterfaceDescription::Member*)member.internal_member)->GetArgAnnotations(argName, NULL, NULL, 0);
    qcc::String* inner_names = new qcc::String[annotation_size];
    qcc::String* inner_values = new qcc::String[annotation_size];

    ((ajn::InterfaceDescription::Member*)member.internal_member)->GetArgAnnotations(argName, inner_names, inner_values, annotation_size);

    if ((name != nullptr) && (value != nullptr)) {
        if (*name_size >= 1) {
            strncpy(name, inner_names[index].c_str(), *name_size);
            name[*name_size - 1] = '\0';
        }

        if (*value_size >= 1) {
            strncpy(value, inner_values[index].c_str(), *value_size);
            value[*value_size - 1] = '\0';
        }
    }
    //size of the string plus the nul character
    *name_size = inner_names[index].size() + 1;
    //size of the string plus the nul character
    *value_size = inner_values[index].size() + 1;
    delete[] inner_names;
    delete[] inner_values;
}

QCC_BOOL AJ_CALL alljoyn_interfacedescription_member_getargannotation(alljoyn_interfacedescription_member member, const char* argName, const char* name, char* value, size_t* value_size)
{
    QCC_DbgTrace(("%s", __FUNCTION__));
    QCC_BOOL return_value = QCC_FALSE;
    qcc::String out_val;
    bool b = ((ajn::InterfaceDescription::Member*)member.internal_member)->GetArgAnnotation(argName, name, out_val);

    if ((value != nullptr) && (*value_size >= 1)) {
        if (b) {
            strncpy(value, out_val.c_str(), *value_size);
            value[*value_size - 1] = '\0';
            return_value = QCC_TRUE;
        } else {
            //return empty string
            *value = '\0';
        }
    }
    *value_size = out_val.size() + 1;
    return return_value;
}

size_t AJ_CALL alljoyn_interfacedescription_property_getannotationscount(alljoyn_interfacedescription_property property)
{
    QCC_DbgTrace(("%s", __FUNCTION__));
    return ((ajn::InterfaceDescription::Property*)property.internal_property)->GetAnnotations(NULL, NULL, 0);
}

void AJ_CALL alljoyn_interfacedescription_property_getannotationatindex(alljoyn_interfacedescription_property property,
                                                                        size_t index,
                                                                        char* name, size_t* name_size,
                                                                        char* value, size_t* value_size)
{
    QCC_DbgTrace(("%s", __FUNCTION__));
    size_t annotation_size = ((ajn::InterfaceDescription::Property*)property.internal_property)->GetAnnotations(NULL, NULL, 0);
    qcc::String* inner_names = new qcc::String[annotation_size];
    qcc::String* inner_values = new qcc::String[annotation_size];

    ((ajn::InterfaceDescription::Property*)property.internal_property)->GetAnnotations(inner_names, inner_values, annotation_size);

    if ((name != nullptr) && (value != nullptr)) {
        if (*name_size >= 1) {
            strncpy(name, inner_names[index].c_str(), *name_size);
            name[*name_size - 1] = '\0';
        }

        if (*value_size >= 1) {
            strncpy(value, inner_values[index].c_str(), *value_size);
            value[*value_size - 1] = '\0';
        }
    }
    //size of the string plus the nul character
    *name_size = inner_names[index].size() + 1;
    //size of the string plus the nul character
    *value_size = inner_values[index].size() + 1;
    delete[] inner_names;
    delete[] inner_values;
}

QCC_BOOL AJ_CALL alljoyn_interfacedescription_property_getannotation(alljoyn_interfacedescription_property property,
                                                                     const char* name,
                                                                     char* value,
                                                                     size_t* value_size)
{
    QCC_DbgTrace(("%s", __FUNCTION__));
    QCC_BOOL return_value = QCC_FALSE;
    qcc::String out_val;
    bool b = ((ajn::InterfaceDescription::Property*)property.internal_property)->GetAnnotation(name, out_val);

    if ((value != nullptr) && (*value_size >= 1)) {
        if (b) {
            strncpy(value, out_val.c_str(), *value_size);
            value[*value_size - 1] = '\0';
            return_value = QCC_TRUE;
        } else {
            //return empty string
            *value = '\0';
        }
    }
    *value_size = out_val.size() + 1;
    return return_value;
}

void AJ_CALL alljoyn_interfacedescription_activate(alljoyn_interfacedescription iface)
{
    QCC_DbgTrace(("%s", __FUNCTION__));
    ((ajn::InterfaceDescription*)iface)->Activate();
}

QCC_BOOL AJ_CALL alljoyn_interfacedescription_getmember(const alljoyn_interfacedescription iface, const char* name,
                                                        alljoyn_interfacedescription_member* member)
{
    QCC_DbgTrace(("%s", __FUNCTION__));
    const ajn::InterfaceDescription::Member* found_member = ((const ajn::InterfaceDescription*)iface)->GetMember(name);
    if (found_member != NULL) {
        member->iface = (alljoyn_interfacedescription)found_member->iface;
        member->memberType = (alljoyn_messagetype)found_member->memberType;
        member->name = found_member->name.c_str();
        member->signature = found_member->signature.c_str();
        member->returnSignature = found_member->returnSignature.c_str();
        member->argNames = found_member->argNames.c_str();

        member->internal_member = found_member;
    }
    return (found_member == NULL ? QCC_FALSE : QCC_TRUE);
}

QStatus AJ_CALL alljoyn_interfacedescription_addannotation(alljoyn_interfacedescription iface, const char* name, const char* value)
{
    QCC_DbgTrace(("%s", __FUNCTION__));
    return ((ajn::InterfaceDescription*)iface)->AddAnnotation(name, value);
}

QCC_BOOL AJ_CALL alljoyn_interfacedescription_getannotation(alljoyn_interfacedescription iface, const char* name, char* value, size_t* value_size)
{
    QCC_DbgTrace(("%s", __FUNCTION__));
    QCC_BOOL return_value = QCC_FALSE;
    qcc::String out_val;
    const bool b = ((ajn::InterfaceDescription*)iface)->GetAnnotation(name, out_val);

    if ((value != nullptr) && (*value_size >= 1)) {
        if (b) {
            strncpy(value, out_val.c_str(), *value_size);
            value[*value_size - 1] = '\0';
            return_value = QCC_TRUE;
        } else {
            //return empty string
            *value = '\0';
        }
    }
    *value_size = out_val.size() + 1;
    return return_value;
}

size_t AJ_CALL alljoyn_interfacedescription_getannotationscount(alljoyn_interfacedescription iface)
{
    QCC_DbgTrace(("%s", __FUNCTION__));
    return ((ajn::InterfaceDescription*)iface)->GetAnnotations(NULL, NULL, 0);
}

void AJ_CALL alljoyn_interfacedescription_getannotationatindex(alljoyn_interfacedescription iface,
                                                               size_t index,
                                                               char* name, size_t* name_size,
                                                               char* value, size_t* value_size)
{
    QCC_DbgTrace(("%s", __FUNCTION__));
    size_t annotation_size = ((ajn::InterfaceDescription*)iface)->GetAnnotations(NULL, NULL, 0);
    qcc::String* inner_names = new qcc::String[annotation_size];
    qcc::String* inner_values = new qcc::String[annotation_size];

    ((ajn::InterfaceDescription*)iface)->GetAnnotations(inner_names, inner_values, annotation_size);

    if ((name != nullptr) && (value != nullptr)) {
        if (*name_size >= 1) {
            strncpy(name, inner_names[index].c_str(), *name_size);
            name[*name_size - 1] = '\0';
        }

        if (*value_size >= 1) {
            strncpy(value, inner_values[index].c_str(), *value_size);
            value[*value_size - 1] = '\0';
        }
    }
    //size of the string plus the nul character
    *name_size = inner_names[index].size() + 1;
    //size of the string plus the nul character
    *value_size = inner_values[index].size() + 1;
    delete[] inner_names;
    delete[] inner_values;
}

QStatus AJ_CALL alljoyn_interfacedescription_addmember(alljoyn_interfacedescription iface, alljoyn_messagetype type,
                                                       const char* name, const char* inputSig, const char* outSig,
                                                       const char* argNames, uint8_t annotation)
{
    QCC_DbgTrace(("%s", __FUNCTION__));
    return ((ajn::InterfaceDescription*)iface)->AddMember((ajn::AllJoynMessageType)type, name, inputSig, outSig,
                                                          argNames, annotation);
}

QStatus AJ_CALL alljoyn_interfacedescription_addmemberannotation(alljoyn_interfacedescription iface,
                                                                 const char* member,
                                                                 const char* name,
                                                                 const char* value)
{
    QCC_DbgTrace(("%s", __FUNCTION__));
    return ((ajn::InterfaceDescription*)iface)->AddMemberAnnotation(member, name, value);
}

QCC_BOOL AJ_CALL alljoyn_interfacedescription_getmemberannotation(alljoyn_interfacedescription iface,
                                                                  const char* member,
                                                                  const char* name,
                                                                  char* value,
                                                                  size_t* value_size)
{
    QCC_DbgTrace(("%s", __FUNCTION__));
    QCC_BOOL return_value = QCC_FALSE;
    qcc::String out_val;
    const bool b = ((ajn::InterfaceDescription*)iface)->GetMemberAnnotation(member, name, out_val);

    if ((value != nullptr) && (*value_size >= 1)) {
        if (b) {
            strncpy(value, out_val.c_str(), *value_size);
            value[*value_size - 1] = '\0';
            return_value = QCC_TRUE;
        } else {
            //return empty string
            *value = '\0';
        }
    }
    *value_size = out_val.size() + 1;
    return return_value;
}

size_t AJ_CALL alljoyn_interfacedescription_getmembers(const alljoyn_interfacedescription iface,
                                                       alljoyn_interfacedescription_member* members,
                                                       size_t numMembers)
{
    QCC_DbgTrace(("%s", __FUNCTION__));
    size_t ret = 0;
    const ajn::InterfaceDescription::Member** tempMembers = NULL;
    if (members != NULL) {
        tempMembers = new const ajn::InterfaceDescription::Member*[numMembers];
        ret = ((const ajn::InterfaceDescription*)iface)->GetMembers(tempMembers, numMembers);
        for (size_t i = 0; i < ret; i++) {
            members[i].iface = (alljoyn_interfacedescription)tempMembers[i]->iface;
            members[i].memberType = (alljoyn_messagetype)tempMembers[i]->memberType;
            members[i].name = tempMembers[i]->name.c_str();
            members[i].signature = tempMembers[i]->signature.c_str();
            members[i].returnSignature = tempMembers[i]->returnSignature.c_str();
            members[i].argNames = tempMembers[i]->argNames.c_str();

            members[i].internal_member = tempMembers[i];
        }
    } else {
        ret = ((const ajn::InterfaceDescription*)iface)->GetMembers();
    }


    if (tempMembers != NULL) {
        delete [] tempMembers;
    }

    return ret;
}

QCC_BOOL AJ_CALL alljoyn_interfacedescription_hasmember(alljoyn_interfacedescription iface,
                                                        const char* name, const char* inSig, const char* outSig)
{
    QCC_DbgTrace(("%s", __FUNCTION__));
    return (((ajn::InterfaceDescription*)iface)->HasMember(name, inSig, outSig) == true ? QCC_TRUE : QCC_FALSE);
}

QStatus AJ_CALL alljoyn_interfacedescription_addmethod(alljoyn_interfacedescription iface,
                                                       const char* name,
                                                       const char* inputSig,
                                                       const char* outSig,
                                                       const char* argNames,
                                                       uint8_t annotation,
                                                       const char* accessPerms)
{
    QCC_DbgTrace(("%s", __FUNCTION__));
    return ((ajn::InterfaceDescription*)iface)->AddMember(ajn::MESSAGE_METHOD_CALL, name, inputSig, outSig, argNames, annotation, accessPerms);
}

QCC_BOOL AJ_CALL alljoyn_interfacedescription_getmethod(alljoyn_interfacedescription iface, const char* name, alljoyn_interfacedescription_member* member)
{
    QCC_DbgTrace(("%s", __FUNCTION__));
    const ajn::InterfaceDescription::Member* found_member = ((const ajn::InterfaceDescription*)iface)->GetMember(name);
    /*
     * only return the member if it is a MESSAGE_METHOD_CALL type all others return NULL
     */
    if (found_member && found_member->memberType == ajn::MESSAGE_METHOD_CALL) {
        member->iface = (alljoyn_interfacedescription)found_member->iface;
        member->memberType = (alljoyn_messagetype)found_member->memberType;
        member->name = found_member->name.c_str();
        member->signature = found_member->signature.c_str();
        member->returnSignature = found_member->returnSignature.c_str();
        member->argNames = found_member->argNames.c_str();

        member->internal_member = found_member;

    } else {
        found_member = NULL;
    }
    return (found_member == NULL ? QCC_FALSE : QCC_TRUE);
}

QStatus AJ_CALL alljoyn_interfacedescription_addsignal(alljoyn_interfacedescription iface,
                                                       const char* name,
                                                       const char* sig,
                                                       const char* argNames,
                                                       uint8_t annotation,
                                                       const char* accessPerms)
{
    QCC_DbgTrace(("%s", __FUNCTION__));
    return ((ajn::InterfaceDescription*)iface)->AddMember(ajn::MESSAGE_SIGNAL, name, sig, NULL, argNames, annotation, accessPerms);
}

QCC_BOOL AJ_CALL alljoyn_interfacedescription_getsignal(alljoyn_interfacedescription iface, const char* name, alljoyn_interfacedescription_member* member)
{
    QCC_DbgTrace(("%s", __FUNCTION__));
    const ajn::InterfaceDescription::Member* found_member = ((const ajn::InterfaceDescription*)iface)->GetMember(name);
    /*
     * only return the member if it is a MESSAGE_SIGNAL type all others return NULL
     */
    if (found_member && found_member->memberType == ajn::MESSAGE_SIGNAL) {
        member->iface = (alljoyn_interfacedescription)found_member->iface;
        member->memberType = (alljoyn_messagetype)found_member->memberType;
        member->name = found_member->name.c_str();
        member->signature = found_member->signature.c_str();
        member->returnSignature = found_member->returnSignature.c_str();
        member->argNames = found_member->argNames.c_str();

        member->internal_member = found_member;
    } else {
        found_member = NULL;
    }
    return (found_member == NULL ? QCC_FALSE : QCC_TRUE);
}

QCC_BOOL AJ_CALL alljoyn_interfacedescription_getproperty(const alljoyn_interfacedescription iface, const char* name,
                                                          alljoyn_interfacedescription_property* property)
{
    QCC_DbgTrace(("%s", __FUNCTION__));
    const ajn::InterfaceDescription::Property* found_prop = ((const ajn::InterfaceDescription*)iface)->GetProperty(name);
    if (found_prop != NULL) {
        property->name = found_prop->name.c_str();
        property->signature = found_prop->signature.c_str();
        property->access = found_prop->access;

        property->internal_property = found_prop;
    }
    return (found_prop == NULL ? QCC_FALSE : QCC_TRUE);
}

size_t AJ_CALL alljoyn_interfacedescription_getproperties(const alljoyn_interfacedescription iface,
                                                          alljoyn_interfacedescription_property* props,
                                                          size_t numProps)
{
    QCC_DbgTrace(("%s", __FUNCTION__));
    size_t ret = 0;
    const ajn::InterfaceDescription::Property** tempProps = NULL;
    if (props != NULL) {
        tempProps = new const ajn::InterfaceDescription::Property*[numProps];

        ret = ((const ajn::InterfaceDescription*)iface)->GetProperties(tempProps, numProps);
        for (size_t i = 0; i < ret; i++) {
            props[i].name = tempProps[i]->name.c_str();
            props[i].signature = tempProps[i]->signature.c_str();
            props[i].access = tempProps[i]->access;

            props[i].internal_property = tempProps[i];
        }
    } else {
        ret = ((const ajn::InterfaceDescription*)iface)->GetProperties();
    }
    if (tempProps != NULL) {
        delete [] tempProps;
    }

    return ret;
}

QStatus AJ_CALL alljoyn_interfacedescription_addproperty(alljoyn_interfacedescription iface, const char* name,
                                                         const char* signature, uint8_t access)
{
    QCC_DbgTrace(("%s", __FUNCTION__));
    return ((ajn::InterfaceDescription*)iface)->AddProperty(name, signature, access);
}

QCC_BOOL AJ_CALL alljoyn_interfacedescription_hasproperty(const alljoyn_interfacedescription iface, const char* name)
{
    QCC_DbgTrace(("%s", __FUNCTION__));
    return (((const ajn::InterfaceDescription*)iface)->HasProperty(name) == true ? QCC_TRUE : QCC_FALSE);
}

QCC_BOOL AJ_CALL alljoyn_interfacedescription_hasproperties(const alljoyn_interfacedescription iface)
{
    QCC_DbgTrace(("%s", __FUNCTION__));
    return (((const ajn::InterfaceDescription*)iface)->HasProperties() == true ? QCC_TRUE : QCC_FALSE);
}

QStatus AJ_CALL alljoyn_interfacedescription_addpropertyannotation(alljoyn_interfacedescription iface,
                                                                   const char* property,
                                                                   const char* name,
                                                                   const char* value)
{
    QCC_DbgTrace(("%s", __FUNCTION__));
    return ((ajn::InterfaceDescription*)iface)->AddPropertyAnnotation(property, name, value);
}

QCC_BOOL AJ_CALL alljoyn_interfacedescription_getpropertyannotation(alljoyn_interfacedescription iface,
                                                                    const char* property,
                                                                    const char* name,
                                                                    char* value,
                                                                    size_t* value_size)
{
    QCC_DbgTrace(("%s", __FUNCTION__));
    if (iface == NULL) {
        return QCC_FALSE;
    }
    if (property == NULL) {
        return QCC_FALSE;
    }
    if (name == NULL) {
        return QCC_FALSE;
    }

    QCC_BOOL return_value = QCC_FALSE;
    qcc::String out_val;
    const bool b = ((ajn::InterfaceDescription*)iface)->GetPropertyAnnotation(property, name, out_val);

    //Return the size of the string so memory can be allocated for the string
    if ((value != nullptr) && (*value_size >= 1)) {
        if (b) {
            strncpy(value, out_val.c_str(), *value_size);
            value[*value_size - 1] = '\0';
            return_value = QCC_TRUE;
        } else {
            //return empty string
            *value = '\0';
        }
    }
    *value_size = out_val.size() + 1;
    return return_value;
}

const char* AJ_CALL alljoyn_interfacedescription_getname(const alljoyn_interfacedescription iface)
{
    QCC_DbgTrace(("%s", __FUNCTION__));
    return ((const ajn::InterfaceDescription*)iface)->GetName();
}

size_t AJ_CALL alljoyn_interfacedescription_introspect(const alljoyn_interfacedescription iface, char* str, size_t buf, size_t indent)
{
    QCC_DbgTrace(("%s", __FUNCTION__));
    if (iface == nullptr) {
        return (size_t)0;
    }
    qcc::String s = ((const ajn::InterfaceDescription*)iface)->Introspect(indent);
    /*
     * it is ok to send in NULL for str when the user is only interested in the
     * size of the resulting string.
     */
    if ((str != nullptr) && (buf >= 1)) {
        strncpy(str, s.c_str(), buf);
        str[buf - 1] = '\0';
    }
    //size of the string plus the nul character
    return s.size() + 1;
}

QCC_BOOL AJ_CALL alljoyn_interfacedescription_issecure(const alljoyn_interfacedescription iface)
{
    QCC_DbgTrace(("%s", __FUNCTION__));
    return ((const ajn::InterfaceDescription*)iface)->IsSecure();
}

alljoyn_interfacedescription_securitypolicy AJ_CALL alljoyn_interfacedescription_getsecuritypolicy(const alljoyn_interfacedescription iface)
{
    QCC_DbgTrace(("%s", __FUNCTION__));
    return (alljoyn_interfacedescription_securitypolicy)((const ajn::InterfaceDescription*)iface)->GetSecurityPolicy();
}

void AJ_CALL alljoyn_interfacedescription_setdescriptionlanguage(alljoyn_interfacedescription iface, const char* language)
{
    QCC_DbgTrace(("%s", __FUNCTION__));
    if ((language != nullptr) && (language[0] != '\0')) {
        ajn::translatorC.AddTargetLanguage(language);
    }
#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif
#if defined(QCC_OS_GROUP_WINDOWS)
#pragma warning(push)
#pragma warning(disable: 4996)
#endif
    /*
     * We need to call a deprecated C++ accessor from this deprecated C accessor.
     * This call and the entire function will be removed when the deprecation phase is over.
     */
    ((ajn::InterfaceDescription*)iface)->SetDescriptionLanguage(language);
#if defined(QCC_OS_GROUP_WINDOWS)
#pragma warning(pop)
#endif
#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif
}

size_t AJ_CALL alljoyn_interfacedescription_getdescriptionlanguages(const alljoyn_interfacedescription iface, const char** languages, size_t size)
{
    QCC_DbgTrace(("%s", __FUNCTION__));
    size_t count = 0;
#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif
#if defined(QCC_OS_GROUP_WINDOWS)
#pragma warning(push)
#pragma warning(disable: 4996)
#endif
    /*
     * We need to call a deprecated C++ accessor from this deprecated C accessor.
     * This call and the entire function will be removed when the deprecation phase is over.
     */
    ajn::Translator* translator = ((const ajn::InterfaceDescription*)iface)->GetDescriptionTranslator();
#if defined(QCC_OS_GROUP_WINDOWS)
#pragma warning(pop)
#endif
#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif
    if (translator == &ajn::translatorC) {
        /* Get description languages from translator */
        size_t numTargetLanguages = translator->NumTargetLanguages();
        if (languages == nullptr || size == 0u) {
            return numTargetLanguages;
        }
        count = ajn::translatorC.CopyPointersToArray(languages, size);
    } else {
        if (languages == nullptr) {
            count = 1;
        } else if (size > 0) {
#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif
#if defined(QCC_OS_GROUP_WINDOWS)
#pragma warning(push)
#pragma warning(disable: 4996)
#endif
            languages[0] = ((const ajn::InterfaceDescription*)iface)->GetDescriptionLanguage();
#if defined(QCC_OS_GROUP_WINDOWS)
#pragma warning(pop)
#endif
#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif
            count = 1;
        } else {
            count = 0;
        }
    }
    return count;
}

size_t AJ_CALL alljoyn_interfacedescription_getdescriptionlanguages2(
    const alljoyn_interfacedescription iface, char* languages, size_t languagesSize)
{
    QCC_DbgTrace(("%s", __FUNCTION__));
    const qcc::String DELIMITER = ",";
    std::set<qcc::String> foundLanguages = ((ajn::InterfaceDescription*)iface)->GetDescriptionLanguages();
    size_t totalSize = 0;

    if (languages == nullptr || languagesSize == 0) {
        for (std::set<qcc::String>::const_iterator itL = foundLanguages.begin();
             itL != foundLanguages.end(); itL++) {
            totalSize += itL->size() + 1;
        }
        return totalSize;
    }

    for (std::set<qcc::String>::const_iterator itL = foundLanguages.begin();
         itL != foundLanguages.end(); itL++) {
        qcc::String candidate;
        if (totalSize != 0) {
            /* Not the first language in the array - prepend with the delimiter. */
            candidate = DELIMITER;
        }
        candidate += *itL;
        if (totalSize + candidate.size() + 1 > languagesSize) {
            /* No space for the candidate with the terminating NULL character - finish
             * without writing the candidate.
             */
            break;
        }
        strncpy(languages + totalSize, candidate.c_str(), candidate.size());
        totalSize += candidate.size();
    }
    languages[totalSize] = '\0';
    totalSize++;
    return totalSize;
}

void AJ_CALL alljoyn_interfacedescription_setdescription(alljoyn_interfacedescription iface, const char* description)
{
    QCC_DbgTrace(("%s", __FUNCTION__));
#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif
#if defined(QCC_OS_GROUP_WINDOWS)
#pragma warning(push)
#pragma warning(disable: 4996)
#endif
    /*
     * We need to call a deprecated C++ accessor from this deprecated C accessor.
     * This call and the entire function will be removed when the deprecation phase is over.
     */
    ((ajn::InterfaceDescription*)iface)->SetDescription(description);
#if defined(QCC_OS_GROUP_WINDOWS)
#pragma warning(pop)
#endif
#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif
}

QStatus AJ_CALL alljoyn_interfacedescription_setdescriptionforlanguage(alljoyn_interfacedescription iface, const char* description, const char* languageTag)
{
    QCC_DbgTrace(("%s", __FUNCTION__));
    QStatus status = ((ajn::InterfaceDescription*)iface)->SetDescriptionForLanguage(description, languageTag);
    return status;
}

size_t AJ_CALL alljoyn_interfacedescription_getdescriptionforlanguage(alljoyn_interfacedescription iface, char* description, size_t maxLanguageLength, const char* languageTag)
{
    QCC_DbgTrace(("%s", __FUNCTION__));
    qcc::String out_val;
    const bool found = ((ajn::InterfaceDescription*)iface)->GetDescriptionForLanguage(out_val, languageTag);

    if ((description != nullptr) && (maxLanguageLength >= 1)) {
        if (found) {
            strncpy(description, out_val.c_str(), maxLanguageLength);
            description[maxLanguageLength - 1] = '\0';
        } else {
            //return empty string
            *description = '\0';
        }
    }
    return out_val.size();
}


QStatus AJ_CALL alljoyn_interfacedescription_setmemberdescription(alljoyn_interfacedescription iface, const char* member, const char* description)
{
    QCC_DbgTrace(("%s", __FUNCTION__));
#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif
#if defined(QCC_OS_GROUP_WINDOWS)
#pragma warning(push)
#pragma warning(disable: 4996)
#endif
    /*
     * We need to call a deprecated C++ accessor from this deprecated C accessor.
     * This call and the entire function will be removed when the deprecation phase is over.
     */
    return ((ajn::InterfaceDescription*)iface)->SetMemberDescription(member, description);
#if defined(QCC_OS_GROUP_WINDOWS)
#pragma warning(pop)
#endif
#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif
}

QStatus AJ_CALL alljoyn_interfacedescription_setmemberdescriptionforlanguage(
    alljoyn_interfacedescription iface, const char* member, const char* description, const char* languageTag)
{
    QCC_DbgTrace(("%s", __FUNCTION__));
    return ((ajn::InterfaceDescription*)iface)->SetMemberDescriptionForLanguage(qcc::String(member), description, languageTag);
}

size_t AJ_CALL alljoyn_interfacedescription_getmemberdescriptionforlanguage(
    alljoyn_interfacedescription iface, const char* member, char* description, size_t maxLanguageLength, const char* languageTag)
{
    QCC_DbgTrace(("%s", __FUNCTION__));
    qcc::String out_val;
    const bool found = ((ajn::InterfaceDescription*)iface)->GetMemberDescriptionForLanguage(member, out_val, languageTag);

    if ((description != nullptr) && (maxLanguageLength >= 1)) {
        if (found) {
            strncpy(description, out_val.c_str(), maxLanguageLength);
            description[maxLanguageLength - 1] = '\0';
        } else {
            //return empty string
            *description = '\0';
        }
    }
    return out_val.size();
}

QStatus AJ_CALL alljoyn_interfacedescription_setargdescription(alljoyn_interfacedescription iface, const char* member, const char* argName, const char* description)
{
    QCC_DbgTrace(("%s", __FUNCTION__));
#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif
#if defined(QCC_OS_GROUP_WINDOWS)
#pragma warning(push)
#pragma warning(disable: 4996)
#endif
    /*
     * We need to call a deprecated C++ accessor from this deprecated C accessor.
     * This call and the entire function will be removed when the deprecation phase is over.
     */
    return ((ajn::InterfaceDescription*)iface)->SetArgDescription(member, argName, description);
#if defined(QCC_OS_GROUP_WINDOWS)
#pragma warning(pop)
#endif
#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif
}

QStatus AJ_CALL alljoyn_interfacedescription_setargdescriptionforlanguage(
    alljoyn_interfacedescription iface, const char* member, const char* arg, const char* description, const char* languageTag)
{
    QCC_DbgTrace(("%s", __FUNCTION__));
    return ((ajn::InterfaceDescription*)iface)->SetArgDescriptionForLanguage(member, arg, description, languageTag);
}

size_t AJ_CALL alljoyn_interfacedescription_getargdescriptionforlanguage(
    alljoyn_interfacedescription iface, const char* member, const char* arg, char* description, size_t maxLanguageLength, const char* languageTag)
{
    QCC_DbgTrace(("%s", __FUNCTION__));
    qcc::String out_val;
    const bool found = ((ajn::InterfaceDescription*)iface)->GetArgDescriptionForLanguage(member, arg, out_val, languageTag);

    if ((description != nullptr) && (maxLanguageLength >= 1)) {
        if (found) {
            strncpy(description, out_val.c_str(), maxLanguageLength);
            description[maxLanguageLength - 1] = '\0';
        } else {
            //return empty string
            *description = '\0';
        }
    }
    return out_val.size();
}

QStatus AJ_CALL alljoyn_interfacedescription_setpropertydescription(alljoyn_interfacedescription iface, const char* name, const char* description)
{
    QCC_DbgTrace(("%s", __FUNCTION__));
#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif
#if defined(QCC_OS_GROUP_WINDOWS)
#pragma warning(push)
#pragma warning(disable: 4996)
#endif
    /*
     * We need to call a deprecated C++ accessor from this deprecated C accessor.
     * This call and the entire function will be removed when the deprecation phase is over.
     */
    return ((ajn::InterfaceDescription*)iface)->SetPropertyDescription(name, description);
#if defined(QCC_OS_GROUP_WINDOWS)
#pragma warning(pop)
#endif
#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif
}

QStatus AJ_CALL alljoyn_interfacedescription_setpropertydescriptionforlanguage(
    alljoyn_interfacedescription iface, const char* name, const char* description, const char* languageTag)
{
    QCC_DbgTrace(("%s", __FUNCTION__));
    return ((ajn::InterfaceDescription*)iface)->SetPropertyDescriptionForLanguage(name, description, languageTag);
}

size_t AJ_CALL alljoyn_interfacedescription_getpropertydescriptionforlanguage(
    alljoyn_interfacedescription iface, const char* property, char* description, size_t maxLanguageLength, const char* languageTag)
{
    QCC_DbgTrace(("%s", __FUNCTION__));
    qcc::String out_val;
    const bool found = ((ajn::InterfaceDescription*)iface)->GetPropertyDescriptionForLanguage(property, out_val, languageTag);

    if ((description != nullptr) && (maxLanguageLength >= 1)) {
        if (found) {
            strncpy(description, out_val.c_str(), maxLanguageLength);
            description[maxLanguageLength - 1] = '\0';
        } else {
            //return empty string
            *description = '\0';
        }
    }
    return out_val.size();
}


void AJ_CALL alljoyn_interfacedescription_setdescriptiontranslationcallback(alljoyn_interfacedescription iface, alljoyn_interfacedescription_translation_callback_ptr translation_callback)
{
    QCC_DbgTrace(("%s", __FUNCTION__));
    ajn::translatorC.SetTranslationCallback(translation_callback);
#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif
#if defined(QCC_OS_GROUP_WINDOWS)
#pragma warning(push)
#pragma warning(disable: 4996)
#endif
    /*
     * We need to call a deprecated C++ accessor from this deprecated C accessor.
     * This call and the entire function will be removed when the deprecation phase is over.
     */
    ((ajn::InterfaceDescription*)iface)->SetDescriptionTranslator(&ajn::translatorC);
#if defined(QCC_OS_GROUP_WINDOWS)
#pragma warning(pop)
#endif
#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif
}

alljoyn_interfacedescription_translation_callback_ptr AJ_CALL alljoyn_interfacedescription_getdescriptiontranslationcallback(const alljoyn_interfacedescription iface)
{
    QCC_DbgTrace(("%s", __FUNCTION__));
#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif
#if defined(QCC_OS_GROUP_WINDOWS)
#pragma warning(push)
#pragma warning(disable: 4996)
#endif
    /*
     * We need to call a deprecated C++ accessor from this deprecated C accessor.
     * This call and the entire function will be removed when the deprecation phase is over.
     */
    ajn::Translator* translator = ((const ajn::InterfaceDescription*)iface)->GetDescriptionTranslator();
#if defined(QCC_OS_GROUP_WINDOWS)
#pragma warning(pop)
#endif
#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif

    if (translator == &ajn::translatorC) {
        return ajn::translatorC.GetTranslationCallback();
    }
    return nullptr;
}

QCC_BOOL AJ_CALL alljoyn_interfacedescription_hasdescription(const alljoyn_interfacedescription iface)
{
    QCC_DbgTrace(("%s", __FUNCTION__));
    return ((const ajn::InterfaceDescription*)iface)->HasDescription();
}

QStatus AJ_CALL alljoyn_interfacedescription_addargannotation(alljoyn_interfacedescription iface, const char* member, const char* argName, const char* name, const char* value)
{
    QCC_DbgTrace(("%s", __FUNCTION__));
    return ((ajn::InterfaceDescription*)iface)->AddArgAnnotation(member, argName, name, value);
}

QCC_BOOL AJ_CALL alljoyn_interfacedescription_getmemberargannotation(const alljoyn_interfacedescription iface, const char* member, const char* argName, const char* name, char* value, size_t* value_size)
{
    QCC_DbgTrace(("%s", __FUNCTION__));
    QCC_BOOL return_value = QCC_FALSE;
    qcc::String out_val;
    bool b = ((const ajn::InterfaceDescription*)iface)->GetArgAnnotation(member, argName, name, out_val);

    if ((value != nullptr) && (*value_size >= 1)) {
        if (b) {
            strncpy(value, out_val.c_str(), *value_size);
            value[*value_size - 1] = '\0';
            return_value = QCC_TRUE;
        } else {
            //return empty string
            *value = '\0';
        }
    }
    *value_size = out_val.size() + 1;
    return return_value;
}

QCC_BOOL AJ_CALL alljoyn_interfacedescription_eql(const alljoyn_interfacedescription one,
                                                  const alljoyn_interfacedescription other)
{
    QCC_DbgTrace(("%s", __FUNCTION__));
    const ajn::InterfaceDescription& _one = *((const ajn::InterfaceDescription*)one);
    const ajn::InterfaceDescription& _other = *((const ajn::InterfaceDescription*)other);

    return (_one == _other ? QCC_TRUE : QCC_FALSE);
}

QCC_BOOL AJ_CALL alljoyn_interfacedescription_member_eql(const alljoyn_interfacedescription_member one,
                                                         const alljoyn_interfacedescription_member other)
{
    QCC_DbgTrace(("%s", __FUNCTION__));
    const ajn::InterfaceDescription::Member _one = *((ajn::InterfaceDescription::Member*)one.internal_member);
    const ajn::InterfaceDescription::Member _other = *((ajn::InterfaceDescription::Member*)other.internal_member);

    return (_one == _other ? QCC_TRUE : QCC_FALSE);
}

QCC_BOOL AJ_CALL alljoyn_interfacedescription_property_eql(const alljoyn_interfacedescription_property one,
                                                           const alljoyn_interfacedescription_property other)
{
    QCC_DbgTrace(("%s", __FUNCTION__));
    const ajn::InterfaceDescription::Property _one = *((ajn::InterfaceDescription::Property*)one.internal_property);
    const ajn::InterfaceDescription::Property _other = *((ajn::InterfaceDescription::Property*)other.internal_property);

    return (_one == _other ? QCC_TRUE : QCC_FALSE);
}
