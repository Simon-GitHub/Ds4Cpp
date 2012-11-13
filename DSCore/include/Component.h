//   Copyright 2012 - Global Vision Systems
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

/*
 * Component.h
 *
 *  Created on: 4 mai 2012
 *      Author: simon
 */
#ifndef COMPONENT_H_
#define COMPONENT_H_
#pragma warning( disable: 4251 )

#include <string>

#include <usModule.h>
#include <usModuleContext.h>
#include <usServiceProperties.h>
#include <usServiceReference.h>

#include "ComponentFactoryImpl.h"
#include "ComponentDescriptor.h"
#include "ComponentInstance.h"
#include "ComponentHandle.h"

namespace ds4cpp
{
class US_ABI_EXPORT Component
{
	friend class ComponentInstance ;
	friend class ComponentManagerImpl ;
private:
	bool                                                factory ;
    std::vector<ComponentInstance*>                     instances ;
    ComponentHandle*                                    handle ;
    const us::Module*                                   module ;


    ComponentHandle* getHandle() ;
    
	void *callCreate(ComponentInstance *componentInstance) ;
	void callActivate(ComponentInstance *componentInstance) ;
	void callBind(ComponentInstance* componentInstance, const us::ServiceReference& SvcReference, const std::string& interfaceName, ComponentReference::Cardinality cardinality) ;

	void publishServices(ComponentInstance* componentInstance) ;

    Component(const us::Module* module, const ComponentDescriptor& descriptor) ;
    virtual ~Component() ;

public:
    const ComponentDescriptor descriptor ;

	/** Create an empty Component instance */
	ComponentInstance* newEmptyComponentInstance(const us::ServiceProperties& overrideProperties) ;

	int getNumInstance() const ;
} ;
}       /* namespace ds4cpp */
#endif /* COMPONENT_H_ */
