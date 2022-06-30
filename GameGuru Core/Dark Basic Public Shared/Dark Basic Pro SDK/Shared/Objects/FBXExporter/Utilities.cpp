#include "Utilities.h"

void Utilities::WriteMatrix(std::ostream& inStream, FbxMatrix& inMatrix, bool inIsRoot)
{
	inStream << "<mat>" << static_cast<float>(inMatrix.Get(0, 0)) << "," << static_cast<float>(inMatrix.Get(0, 1)) << "," << static_cast<float>(inMatrix.Get(0, 2)) << "," << static_cast<float>(inMatrix.Get(0, 3)) << ","
		<< static_cast<float>(inMatrix.Get(1, 0)) << "," << static_cast<float>(inMatrix.Get(1, 1)) << "," << static_cast<float>(inMatrix.Get(1, 2)) << "," << static_cast<float>(inMatrix.Get(1, 3)) << ","
		<< static_cast<float>(inMatrix.Get(2, 0)) << "," << static_cast<float>(inMatrix.Get(2, 1)) << "," << static_cast<float>(inMatrix.Get(2, 2)) << "," << static_cast<float>(inMatrix.Get(2, 3)) << ","
		<< static_cast<float>(inMatrix.Get(3, 0)) << "," << static_cast<float>(inMatrix.Get(3, 1)) << "," << static_cast<float>(inMatrix.Get(3, 2)) << "," << static_cast<float>(inMatrix.Get(3, 3)) << "</mat>\n";
}

void Utilities::PrintMatrix(FbxMatrix& inMatrix)
{
	FbxString lMatrixValue;
	for (int k = 0; k<4; ++k)
	{
		FbxVector4 lRow = inMatrix.GetRow(k);
		char        lRowValue[1024];

		FBXSDK_sprintf(lRowValue, 1024, "%9.4f %9.4f %9.4f %9.4f\n", lRow[0], lRow[1], lRow[2], lRow[3]);
		lMatrixValue += FbxString("        ") + FbxString(lRowValue);
	}

	std::cout << lMatrixValue.Buffer();
}

FbxAMatrix Utilities::GetGeometryTransformation(FbxNode* inNode)
{
	if (!inNode)
	{
		throw std::exception("Null for mesh geometry");
	}

	const FbxVector4 lT = inNode->GetGeometricTranslation(FbxNode::eSourcePivot);
	const FbxVector4 lR = inNode->GetGeometricRotation(FbxNode::eSourcePivot);
	const FbxVector4 lS = inNode->GetGeometricScaling(FbxNode::eSourcePivot);

	return FbxAMatrix(lT, lR, lS);
}

std::string Utilities::GetFileName(const std::string& inInput)
{
	std::string seperator("\\");
	unsigned int pos = inInput.find_last_of(seperator);
	if(pos != std::string::npos)
	{
		return inInput.substr(pos + 1);
	}
	else
	{
		return inInput;
	}
}

std::string Utilities::RemoveSuffix(const std::string& inInput)
{
	std::string seperator(".");
	unsigned int pos = inInput.find_last_of(seperator);
	if (pos != std::string::npos)
	{
		return inInput.substr(0, pos);
	}
	else
	{
		return inInput;
	}
}