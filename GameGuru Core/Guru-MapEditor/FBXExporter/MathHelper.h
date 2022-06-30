#include <Windows.h>
#include <stdint.h>
#include <xnamath.h>

class MathHelper
{
public:

	static const XMFLOAT3 vector3Epsilon;
	static const XMFLOAT2 vector2Epsilon;
	static const XMFLOAT3 vector3True;
	static const XMFLOAT2 vector2True;

	static bool CompareVector2WithEpsilon(const XMFLOAT2& lhs, const XMFLOAT2& rhs);
	static bool CompareVector3WithEpsilon(const XMFLOAT3& lhs, const XMFLOAT3& rhs);
};