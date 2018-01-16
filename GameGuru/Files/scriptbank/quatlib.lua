-- Quaternions module for GameGuru
-- Written by Chris Stapleton

local Q = {}

-- import section
local sin	= math.sin
local cos	= math.cos
local abs	= math.abs
local atan2 = math.atan2
local asin	= math.asin
local pi	= math.pi
local sqrt	= math.sqrt

_ENV = Q


-- guat = {w, x, y, z}
-- roll  (bank)     : rotation around Z axiz  (radians!!)
-- pitch (attitude) : rotation around X axis  (radians!!)
-- yaw   (heading)  : rotation around Y axis  (radians!!)

function Q.FromEuler (pitch, yaw, roll)    -- radians

	local sr = sin(roll/2)
	local sp = sin(pitch/2)
	local sy = sin(yaw/2)
	local cr = cos(roll/2)
	local cp = cos(pitch/2)
	local cy = cos(yaw/2)
		
	local cycp = cy * cp
	local sysp = sy * sp
	local sycp = sy * cp
	local cysp = cy * sp
	
	return {w = (sr * sysp) + (cr * cycp),
			x = (sr * sycp) - (cr * cysp),
			y = (sr * cysp) + (cr * sycp),
			z = (cr * sysp) - (sr * cycp)}
end

function Q.ToEuler (q)
 
	local sqw = q.w*q.w
	local sqx = q.x*q.x
	local sqy = q.y*q.y
	local sqz = q.z*q.z
	
	local h = -2.0 * (q.x*q.z - q.y*q.w)
	
	if abs(h) < 0.99999 then
	
		return  -atan2(2.0 * (q.y*q.z + q.x*q.w),(-sqx - sqy + sqz + sqw)),  -- x ang
				asin(-2.0 * (q.x*q.z - q.y*q.w)),                           -- y ang
				-atan2(2.0 * (q.x*q.y + q.z*q.w),(sqx - sqy - sqz + sqw))    -- z ang
				
	else
		return  -atan2(2.0 * (q.y*q.z + q.x*q.w),(-sqx - sqy + sqz + sqw)),  -- x ang
				(pi / 2) * h,                                               -- y ang
				-atan2(2.0 * (q.x*q.y + q.z*q.w),(sqx - sqy - sqz + sqw))    -- z ang				
	end
end

function Q.Conjugate (q)
	return {w = q.w, x = -q.x, y = -q.y, z = -q.z}
end
 
function Q.Normalise (q)
	local n = sqrt(x*x + y*y + z*z + w*w)

	if n == 0 then
		return {w = 1, x = 0, y = 0, z = 0}
	else
		return {w = q.w / n, x = q.x / n, y = q.y / n, z = q.z / n}
	end
end

function Q.Add (q1, q2)
	return {w = q1.w + q2.w, x = q1.x + q2.x, y = q1.y + q2.y, z = q1.z + q2.z}
end
		
function Q.Mul (q1, q2)

	local A = (q1.w + q1.x)*(q2.w + q2.x)
	local B = (q1.z - q1.y)*(q2.y - q2.z)
	local C = (q1.w - q1.x)*(q2.y + q2.z)
	local D = (q1.y + q1.z)*(q2.w - q2.x)
	local E = (q1.x + q1.z)*(q2.x + q2.y)
	local F = (q1.x - q1.z)*(q2.x - q2.y)
	local G = (q1.w + q1.y)*(q2.w - q2.z)
	local H = (q1.w - q1.y)*(q2.w + q2.z)

	return {w = B + (-E - F + G + H)/2,
		    x = A - ( E + F + G + H)/2,
			y = C + ( E - F + G - H)/2,
			z = D + ( E - F - G + H)/2}
end

return Q