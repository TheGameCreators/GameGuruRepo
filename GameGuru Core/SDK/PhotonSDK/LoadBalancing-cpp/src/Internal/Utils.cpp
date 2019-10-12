/* Exit Games Photon LoadBalancing - C++ Client Lib
 * Copyright (C) 2004-2018 by Exit Games GmbH. All rights reserved.
 * http://www.photonengine.com
 * mailto:developer@photonengine.com
 */

#include "LoadBalancing-cpp/inc/Internal/Utils.h"

namespace ExitGames
{
	namespace LoadBalancing
	{
		namespace Internal
		{
			using namespace Common;

			Hashtable Utils::stripToCustomProperties(const Hashtable& properties)
			{
				Hashtable retVal;
				const JVector<Object>& keys = properties.getKeys();
				for(unsigned int i=0; i<keys.getSize(); i++)
					if(keys[i].getType() == TypeCode::STRING)
						retVal.put(keys[i], properties[i]);
				return retVal;
			}

			Hashtable Utils::stripKeysWithNullValues(const Hashtable& orig)
			{
				Hashtable stripped;
				const JVector<Object>& keys = orig.getKeys();
				for(unsigned int i=0; i<orig.getSize(); i++)
					if(orig[i] != Object())
						stripped.put(keys[i], orig[i]);
				return stripped;
			}
		}
	}
}