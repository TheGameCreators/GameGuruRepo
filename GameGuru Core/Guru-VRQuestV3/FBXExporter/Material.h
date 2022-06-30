#pragma once
#include <vector>
#include <string>
#include <fstream>
#include "Vertex.h"

class Material
{
public:
	std::string mName;
	XMFLOAT3 mAmbient;
	XMFLOAT3 mDiffuse;
	XMFLOAT3 mEmissive;
	double mTransparencyFactor;
	std::string mDiffuseMapName;
	std::string mEmissiveMapName;
	std::string mGlossMapName;
	std::string mNormalMapName;
	std::string mSpecularMapName;

	virtual void WriteToStream(std::ostream& inStream) = 0;
};

class LambertMaterial : public Material
{
public:

	void WriteToStream(std::ostream& inStream)
	{
		inStream << "Ambient: " << mAmbient.x << " " << mAmbient.y << " " << mAmbient.z << std::endl;
		inStream << "Diffuse: " << mDiffuse.x << " " << mDiffuse.y << " " << mDiffuse.z << std::endl;
		inStream << "Emissive: " << mEmissive.x << " " << mEmissive.y << " " << mEmissive.z << std::endl;

		if (!mDiffuseMapName.empty())
		{
			inStream << "DiffuseMap: " << mDiffuseMapName << std::endl;
		}

		if (!mNormalMapName.empty())
		{
			inStream << "NormalMap: " << mNormalMapName << std::endl;
		}
	}
};

class PhongMaterial : public Material
{
public:
	XMFLOAT3 mSpecular;
	XMFLOAT3 mReflection;
	double mSpecularPower;
	double mShininess;
	double mReflectionFactor;

	void WriteToStream(std::ostream& inStream)
	{
		inStream << "Ambient: " << mAmbient.x << " " << mAmbient.y << " " << mAmbient.z << std::endl;
		inStream << "Diffuse: " << mDiffuse.x << " " << mDiffuse.y << " " << mDiffuse.z << std::endl;
		inStream << "Emissive: " << mEmissive.x << " " << mEmissive.y << " " << mEmissive.z << std::endl;
		inStream << "Specular: " << mSpecular.x << " " << mSpecular.y << " " << mSpecular.z << std::endl;
		inStream << "SpecPower: " << mSpecularPower << std::endl;
		inStream << "Reflectivity: " << mReflection.x << " " << mReflection.y << " " << mReflection.z << std::endl;
		
		if (!mDiffuseMapName.empty())
		{
			inStream << "DiffuseMap: " << mDiffuseMapName << std::endl;
		}

		if(!mNormalMapName.empty())
		{
			inStream << "NormalMap: " << mNormalMapName << std::endl;
		}
	}
};