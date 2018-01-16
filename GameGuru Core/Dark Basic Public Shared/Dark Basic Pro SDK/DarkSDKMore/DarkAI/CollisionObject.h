#ifndef COLOBJECT_H
#define COLOBJECT_H

#include "Point.h"
#include "Vector.h"

struct sObject;

//Template for all collision objects
//All collision objects are at least a bounding sphere for quick rejection
class CollisionObject
{
    public:

		int iObjID;
    
        CollisionObject() { pNextObject = 0; bCollisionOn = true; }
        virtual ~CollisionObject() {}
        
        virtual bool Intersects( const Point* p, const Vector* v, int dir, float *dist ) const = 0;
		virtual void MakeCollisionObject( sObject* pObject ) = 0;

		CollisionObject *pNextObject;

		virtual int GetCollisionType() = 0;

		void SetCollisionOn( ) { bCollisionOn = true; }
		void SetCollisionOff( ) { bCollisionOn = false; }
    
	protected:

		bool bCollisionOn;
        
};

#endif
