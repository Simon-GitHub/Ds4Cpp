//   Copyright 2012 - Global Vision Systems
//   Based on library CppMicroServices, Copyright (c) German Cancer Research Center,
//                                    Division of Medical and Biological Informatics
//
//   Licensed under the Apache License, Version 2.0 (the "License");
//   you may not use this file except in compliance with the License.
//   You may obtain a copy of the License at
//
//       http://www.apache.org/licenses/LICENSE-2.0
//
//   Unless required by applicable law or agreed to in writing, software
//   distributed under the License is distributed on an "AS IS" BASIS,
//   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//   See the License for the specific language governing permissions and
//   limitations under the License.

#include "ComponentHandle.h"
#ifndef US_PLATFORM_POSIX
#include <strsafe.h>
#endif

namespace ds4cpp
{



ComponentHandle::ComponentHandle(const std::string& name) : name(name), osHandle(0)
{
}

ComponentHandle::~ComponentHandle()
{
}

bool ComponentHandle::isLoaded() const 
{
	return osHandle != 0 ;
}

ComponentHandle* ComponentHandle::load()
{
    if (osHandle)       // already loaded
    {
        return this ;
    }
    std::string libPath = name ;
#ifdef US_PLATFORM_POSIX
    osHandle = dlopen(libPath.c_str(), RTLD_LAZY | RTLD_GLOBAL) ;
    if (!osHandle)
    {
        const char* err = dlerror() ;
        throw std::runtime_error(err ? std::string(err) : libPath) ;
    }
#else
    osHandle = LoadLibrary(libPath.c_str()) ;
    if (!osHandle)
    {
        std::string             errMsg = "Loading " ;
        std::ostringstream      oss ;
        oss << GetLastError() ;
        errMsg.append(libPath).append("failed with error: ").append(oss.str()) ;
        throw std::runtime_error(errMsg) ;
    }
#endif
    return this ;
}

void ComponentHandle::unload()
{
    if (osHandle)
    {
#ifdef US_PLATFORM_POSIX
        dlclose(osHandle) ;
#else
        FreeLibrary((HMODULE)osHandle) ;
#endif
        osHandle = 0 ;
    }
}

void* ComponentHandle::createObject(const std::string& createFunName, const us::ServiceProperties& componentParameters)
{

    void* (*create)() ;
	void* (*createParam)(const ::us::ServiceProperties& parameters) ;

	std::string createParamName = createFunName + "_param" ;
#ifdef US_PLATFORM_POSIX
    createParam = (void* (*)(const ::us::ServiceProperties& parameters))dlsym(osHandle, createParamName.c_str()) ;
    create = (void* (*)())dlsym(osHandle, createFunName.c_str()) ;
    if (!create && !createParam)
    {
        const char* err = dlerror() ;
        throw std::runtime_error(err ? std::string(err) : createFunName) ;
    }
#else
	createParam = (void* (*)(const ::us::ServiceProperties& parameters)) ::GetProcAddress((HMODULE)osHandle, createParamName.c_str()) ;
    create = (void* (*)()) ::GetProcAddress((HMODULE)osHandle, createFunName.c_str()) ;
    if (!create && !createParam)
    {
        std::string             errMsg = "create " ;
        std::ostringstream      oss ;
        oss << GetLastError() ;
        errMsg.append(createFunName).append("failed with error: ").append(oss.str()) ;
        throw std::runtime_error(errMsg) ;
    }
#endif
	if (createParam != NULL)
		return createParam(componentParameters) ;
    return create() ;
}

void ComponentHandle::callMethod1(const std::string& methodName, void* instance, void* param)
{
    void (* funCallMethod1)(void*, void*) ;
#ifdef US_PLATFORM_POSIX
    funCallMethod1 = (void (*)(void*,
                               void*))dlsym(osHandle, methodName.c_str()) ;
    if (!funCallMethod1)
    {
        const char* err = dlerror() ;
        throw std::runtime_error(err ? std::string(err) : methodName) ;
    }
#else
    funCallMethod1 = (void (*)(void*, void*)) ::GetProcAddress((HMODULE)osHandle, methodName.c_str()) ;
    if (!funCallMethod1)
    {
        std::string             errMsg = "callMethod1 " ;
        std::ostringstream      oss ;
        oss << GetLastError() ;
        errMsg.append(methodName).append("failed with error: ").append(oss.str()) ;
        throw std::runtime_error(errMsg) ;
    }
#endif
    return funCallMethod1(instance, param) ;
}

void ComponentHandle::callActivate(const std::string& methodName, void* instance, const ::us::ServiceProperties& param)
{
	// Typedefs
	typedef void (* Activate)(void*) ;
	typedef void (* ActivateParam)(void*, const ::us::ServiceProperties&) ;

	// Method
    Activate funActivateMethod = NULL;
	ActivateParam funActivateMethodParam = NULL ;

	std::string activateParamName = methodName + "_param" ;
#ifdef US_PLATFORM_POSIX
	funActivateMethodParam = (ActivateParam) dlsym(osHandle, activateParamName.c_str()) ;
    funActivateMethod = (Activate) dlsym(osHandle, methodName.c_str()) ;
    if (!funActivateMethod && !funActivateMethodParam)
    {
        const char* err = dlerror() ;

        // fail silently if activate isn't here!
        US_DEBUG << "DEBUG: " << name << ": Couldn't find activate method" ;
    }
#else
	funActivateMethodParam = (ActivateParam) ::GetProcAddress((HMODULE)osHandle, activateParamName.c_str()) ;
    funActivateMethod = (Activate) ::GetProcAddress((HMODULE)osHandle, methodName.c_str()) ;
    if (!funActivateMethod && !funActivateMethodParam)
    {
        std::string             errMsg = "callActivate " ;
        std::ostringstream      oss ;
        oss << GetLastError() ;
        errMsg.append(methodName).append("failed with error: ").append(oss.str()) ;

        // fail silently if activate isn't here!
        US_DEBUG << "DEBUG: " << name << " " << errMsg ;
    }
#endif
	if (funActivateMethodParam)
	{
		funActivateMethodParam(instance, param) ;
		return ;
	}
    if (funActivateMethod)
    {
        funActivateMethod(instance) ;
    }
}

void ComponentHandle::callDeactivate(const std::string& methodName, void* instance)
{
	// Typedefs
	typedef void (* Deactivate)(void*) ;

	// Method
    Deactivate funDeactivateMethod = NULL;
#ifdef US_PLATFORM_POSIX
    funDeactivateMethod = (Deactivate) dlsym(osHandle, methodName.c_str()) ;
    if (!funDeactivateMethod)
    {
        const char* err = dlerror() ;

        // fail silently if activate isn't here!
        US_DEBUG << name << ": Couldn't find deactivate method" ;
    }
#else
    funDeactivateMethod = (Deactivate) ::GetProcAddress((HMODULE)osHandle, methodName.c_str()) ;
    if (!funDeactivateMethod)
    {
        std::string             errMsg = "callDeactivate " ;
        std::ostringstream      oss ;
        oss << GetLastError() ;
        errMsg.append(methodName).append("failed with error: ").append(oss.str()) ;

        // fail silently if activate isn't here!
        US_DEBUG << name << " " << errMsg ;
		return ;
    }
#endif
    funDeactivateMethod(instance) ;
}

}
