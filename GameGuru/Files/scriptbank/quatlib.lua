-- Quaternions module for GameGuru
-- Written by Chris Stapleton

local Q = {}

-- import section
local sin	= math.sin
local cos	= math.cos
local abs	= math.abs
local acos  = math.acos
local atan2 = math.atan2
local asin	= math.asin
local pi	= math.pi
local sqrt	= math.sqrt

-- new engine functions
local QuatToEuler  = QuatToEuler
local EulerToQuat  = EulerToQuat
local QuatMultiply = QuatMultiply
local QuatLERP     = QuatLERP
local QuatSLERP    = QuatSLERP

_ENV = Q


-- guat = {w, x, y, z}
-- roll  (bank)     : rotation around Z axiz  (radians!!)
-- pitch (attitude) : rotation around X axis  (radians!!)
-- yaw   (heading)  : rotation around Y axis  (radians!!)

function Q.FromEuler (pitch, yaw, roll)    -- radians

	local x, y, z, w =  EulerToQuat( pitch, yaw, roll )

	return { w = w, x = x, y = y, z = z }
		
--	local sr = sin(roll/2)
--	local sp = sin(pitch/2)
--	local sy = sin(yaw/2)
--	local cr = cos(roll/2)
--	local cp = cos(pitch/2)
--	local cy = cos(yaw/2)
		
--	local cycp = cy * cp
--	local sysp = sy * sp
--	local sycp = sy * cp
--	local cysp = cy * sp
	
--	return {w = (sr * sysp) + (cr * cycp),
--			x = (sr * sycp) - (cr * cysp),
--			y = (sr * cysp) + (cr * sycp),
--			z = (cr * sysp) - (sr * cycp)}
end

function Q.ToEuler (q)
 
	return QuatToEuler( q.x, q.y, q.z, q.w )
	
-- Old Lua code moved into GG engine	
	
--	local sqw = q.w*q.w
--	local sqx = q.x*q.x
--	local sqy = q.y*q.y
--	local sqz = q.z*q.z
	
--	local h = -2.0 * (q.x*q.z - q.y*q.w)
	
--	if abs(h) < 0.99999 then
	
--		return  -atan2(2.0 * (q.y*q.z + q.x*q.w),(-sqx - sqy + sqz + sqw)),  -- x ang
--				asin(-2.0 * (q.x*q.z - q.y*q.w)),                           -- y ang
--				-atan2(2.0 * (q.x*q.y + q.z*q.w),(sqx - sqy - sqz + sqw))    -- z ang
				
--	else
--		return  -atan2(2.0 * (q.y*q.z + q.x*q.w),(-sqx - sqy + sqz + sqw)),  -- x ang
--				(pi / 2) * h,                                               -- y ang
--				-atan2(2.0 * (q.x*q.y + q.z*q.w),(sqx - sqy - sqz + sqw))    -- z ang				
--	end
end

function Q.Conjugate (q)
	return { w = q.w, x = -q.x, y = -q.y, z = -q.z }
end
 
function Q.Normalise (q)
	local n = sqrt( q.w * q.w + q.x * q.x + q.y * q.y + q.z * q.z )

	if n == 0 then
		return { w = 1, x = 0, y = 0, z = 0}
	else
		return { w = q.w / n, x = q.x / n, y = q.y / n, z = q.z / n }
	end
end

function Q.Add (q1, q2)
	return { w = q1.w + q2.w, x = q1.x + q2.x, y = q1.y + q2.y, z = q1.z + q2.z }
end
		
function Q.Mul (q1, q2)

	local x, y, z, w = QuatMultiply( q1.x, q1.y, q1.z, q1.w,
	                                 q2.x, q2.y, q2.z, q2.w )
									 
	return { w = w, x = x, y = y, z = z }

-- Old Lua code moved into GG engine	
--	local A = (q1.w + q1.x)*(q2.w + q2.x)
--	local B = (q1.z - q1.y)*(q2.y - q2.z)
--	local C = (q1.w - q1.x)*(q2.y + q2.z)
--	local D = (q1.y + q1.z)*(q2.w - q2.x)
--	local E = (q1.x + q1.z)*(q2.x + q2.y)
--	local F = (q1.x - q1.z)*(q2.x - q2.y)
--	local G = (q1.w + q1.y)*(q2.w - q2.z)
--	local H = (q1.w - q1.y)*(q2.w + q2.z)

--	return {w = B + (-E - F + G + H)/2,
--		    x = A - ( E + F + G + H)/2,
--			y = C + ( E - F + G - H)/2,
--			z = D + ( E - F - G + H)/2}
end

function Q.SLerp( qa, qb, t )

	local x, y, z, w = QuatSLERP( qa.x, qa.y, qa.z, qa.w,
	                              qb.x, qb.y, qb.z, qb.w, t )

	return { w = w, x = x, y = y, z = z }

-- original Lua code, use engine version instead	
	-- Calculate angle between them.
--	local cosHalfTheta = qa.w * qb.w + qa.x * qb.x + qa.y * qb.y + qa.z * qb.z
		
	-- if qa=qb or qa=-qb then theta = 0 and we can return qa
--	if abs( cosHalfTheta ) >= 1.0 then
--		return { w = qa.w, x = qa.x, y = qa.y, z = qa.z }
--	end

--	if cosHalfTheta < 0 then
--		qb = { w = -qb.w, x = -qb.x, y = -qb.y, z = qb.z }
--		cosHalfTheta = -cosHalfTheta
--	end	
	
	-- Calculate temporary values.
--	local halfTheta = acos( cosHalfTheta )
	
--	local sinHalfTheta = sqrt( 1.0 - cosHalfTheta * cosHalfTheta )

	-- if theta = 180 degrees then result is not fully defined
	-- we could rotate around any axis normal to qa or qb
--	if abs( sinHalfTheta ) < 0.001 then
--		return { w = qa.w * 0.5 + qb.w * 0.5,
--		         x = qa.x * 0.5 + qb.x * 0.5,
--				 y = qa.y * 0.5 + qb.y * 0.5,
--				 z = qa.z * 0.5 + qb.z * 0.5 }
--	end
	
--	local ratioA = sin( ( 1 - t ) * halfTheta ) / sinHalfTheta
--	local ratioB = sin( t * halfTheta ) / sinHalfTheta 

	--calculate Quaternion.
--	return { w = qa.w * ratioA + qb.w * ratioB,
--	         x = qa.x * ratioA + qb.x * ratioB,
--	         y = qa.y * ratioA + qb.y * ratioB,
--	         z = qa.z * ratioA + qb.z * ratioB }
end

function Q.NLerp( qa, qb, t)

	local x, y, z, w = QuatLERP( qa.x, qa.y, qa.z, qa.w,
	                             qb.x, qb.y, qb.z, qb.w, t )

	return Normalise( {w = w, x = x, y = y, z = z } )
	
-- original Lua code moved into GG engine
--	local at = 1 - t
--	local bt = t
--	if ( qa.x * qb.x + qa.y * qb.y + qa.z * qb.z + qa.w * qb.w ) < 0 then bt = -t end

--	return Normalise( { w = w * at + qb.w * bt, 
--	                    x = qa.x * at + qb.x * bt, 
--			            y = qa.y * at + qb.y * bt,
--			            z = qa.z * at + qb.z * bt } )
end
	
return Q