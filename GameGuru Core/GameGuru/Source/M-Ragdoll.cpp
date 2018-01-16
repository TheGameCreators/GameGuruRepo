//----------------------------------------------------
//--- GAMEGURU - M-Ragdoll
//----------------------------------------------------

#include "gameguru.h"


//RAGDOLL Module


void ragdoll_init ( void )
{

	//  Create Joint Rotations and Limits
//  `g.KneeJoint.rotationVec3 = newvector3(90.0, 0.0, 0.0)

//  `g.KneeJoint.limitsVec2 = newvector2(-60.0, 60.0)

	g.KneeJoint.rotationVec3 = newvector3(90.0, 0.0, 0.0);
	g.KneeJoint.limitsVec2 = newvector2(10.0, 140.0);
//  `g.RightElbowJoint.rotationVec3 = newvector3(-120.0, 0.0, 0.0)

//  `g.RightElbowJoint.limitsVec2 = newvector2(0.0, 130.0)

	g.RightElbowJoint.rotationVec3 = newvector3(270.0, 0.0, 0.0);
	g.RightElbowJoint.limitsVec2 = newvector2(-20.0, 150.0);
//  `g.LeftElbowJoint.rotationVec3 = newvector3(270.0, 0.0, 0.0)

//  `g.LeftElbowJoint.limitsVec2 = newvector2(0.0, 130.0)

	g.LeftElbowJoint.rotationVec3 = newvector3(270.0, 0.0, 0.0);
	g.LeftElbowJoint.limitsVec2 = newvector2(-20.0, 150.0);
//  `rightWristJoint.rotationVec3 = newvector3(0.0, 45.0, 0.0)

//  `rightWristJoint.limitsVec2 = newvector2(-40.0, 40.0)

	g.rightWristJoint.rotationVec3 = newvector3(0.0, 45.0, 0.0);
	g.rightWristJoint.limitsVec2 = newvector2(-10.0, 10.0);
//  `leftWristJoint.rotationVec3 = newvector3(0.0, 45.0, 0.0)

//  `leftWristJoint.limitsVec2 = newvector2(-40.0, 40.0)

	g.leftWristJoint.rotationVec3 = newvector3(0.0, 45.0, 0.0);
	g.leftWristJoint.limitsVec2 = newvector2(-10.0, 10.0);
//  `WaistJoint.rotationVec3 = newvector3(90.0, 0.0, 0.0)

//  `WaistJoint.limitsVec2 = newvector2(-50.0, 50.0)

	g.WaistJoint.rotationVec3 = newvector3(90.0, 20.0, 20.0);
	g.WaistJoint.limitsVec2 = newvector2(0,120);
	g.HipJoint.rotationVec3 = newvector3(45.0, 45.0, 45.0);
	g.HipJoint.limitsVec3 = newvector3(15.0, 15.0, 15.0);
//  ``HipJoint.rotationVec3 = newvector3(90.0, 20.0, 20.0)

//  ``HipJoint.limitsVec2 = newvector2(-10.0, 10.0)

//  `RightShoulderJoint.rotationVec3 = newvector3(0.0,0.0,0.0)

//  `RightShoulderJoint.limitsVec3 = newvector3(45.0,45.0,10.0)

	g.RightShoulderJoint.rotationVec3 = newvector3(0.0,0.0,0.0);
	g.RightShoulderJoint.limitsVec3 = newvector3(30.0,30.0,10.0);
//  `LeftShoulderJoint.rotationVec3 = newvector3(0.0,0.0,0.0)

//  `LeftShoulderJoint.limitsVec3 = newvector3(45.0,45.0,10.0)

	g.LeftShoulderJoint.rotationVec3 = newvector3(0.0,0.0,0.0);
	g.LeftShoulderJoint.limitsVec3 = newvector3(30.0,30.0,10.0);
//  `NeckJoint.rotationVec3 = newvector3(0.0,0.0,0.0)

//  `NeckJoint.limitsVec3 = newvector3(24.0,11.5,8.0)

	g.NeckJoint.rotationVec3 = newvector3(0.0,0.0,0.0);
	g.NeckJoint.limitsVec3 = newvector3(24.0,11.5,8.0);

	//  new ragdoll array
	g.RagdollsMax=0;
	Dim (  t.Ragdolls,g.RagdollsMax );
	t.Ragdolls[g.RagdollsMax].obj=0;

return;

}

void ragdoll_free ( void )
{

	//  Create Joint Rotations and Limits
	if (  vector3exist(g.KneeJoint.rotationVec3) ) 
	{
		t.result = deletevector3(g.KneeJoint.rotationVec3);
		t.result = deletevector3(g.KneeJoint.limitsVec2);
		t.result = deletevector3(g.RightElbowJoint.rotationVec3);
		t.result = deletevector3(g.RightElbowJoint.limitsVec2);
		t.result = deletevector3(g.LeftElbowJoint.rotationVec3);
		t.result = deletevector3(g.LeftElbowJoint.limitsVec2);
		t.result = deletevector3(g.rightWristJoint.rotationVec3);
		t.result = deletevector3(g.rightWristJoint.limitsVec2);
		t.result = deletevector3(g.leftWristJoint.rotationVec3);
		t.result = deletevector3(g.leftWristJoint.limitsVec2);
		t.result = deletevector3(g.WaistJoint.rotationVec3);
		t.result = deletevector3(g.WaistJoint.limitsVec2);
		t.result = deletevector3(g.HipJoint.rotationVec3);
		t.result = deletevector3(g.HipJoint.limitsVec3);
		t.result = deletevector3(g.RightShoulderJoint.rotationVec3);
		t.result = deletevector3(g.RightShoulderJoint.limitsVec3);
		t.result = deletevector3(g.LeftShoulderJoint.rotationVec3);
		t.result = deletevector3(g.LeftShoulderJoint.limitsVec3);
		t.result = deletevector3(g.NeckJoint.rotationVec3);
		t.result = deletevector3(g.NeckJoint.limitsVec3);
	}
}

void ragdoll_setcollisionmask ( int iMode )
{
	// FLAGS (1-terrain, 2-objects, 4-capsule)
	// 101016 - set the collision mask used by the ragdoll (end collision field can switch off ragdoll collision from plr)
	g.tphys_CollisionGroup.group0 = 1;
	g.tphys_CollisionGroup.group1 = 2;
	g.tphys_CollisionGroup.group2 = 4;
	if ( iMode == 1 )
	{
		// ragdoll collides with terrain, other entities and player
		g.tphys_CollisionMask.mask0 = 1;
		g.tphys_CollisionMask.mask1 = 2;
		g.tphys_CollisionMask.mask2 = 4;
	}
	else
	{
		// ragdoll collides with terrain, other entities but not player
		g.tphys_CollisionMask.mask0 = 1;
		g.tphys_CollisionMask.mask1 = 2;
		g.tphys_CollisionMask.mask2 = 0;
	}
}

void ragdoll_create ( void )
{
	// try different BIP designations (ideally BIP01_)
	bool bProducedRagdoll = false;
	for ( int iTryBips = 0; iTryBips<2; iTryBips++ )
	{
		cstr pBitPrefix = "";
		if ( iTryBips==0 ) pBitPrefix = "Bip01";
		if ( iTryBips==1 ) pBitPrefix = "Bip002";

		//  first ensure the object is ragdollable
		t.tbip01pelvis=getlimbbyname(t.tphyobj, (pBitPrefix+cstr("_Pelvis")).Get() );
		t.tbip01spine=getlimbbyname(t.tphyobj, (pBitPrefix+cstr("_Spine")).Get() );

		//  receives tphyobj
		if (  BPhys_RagdollExist(t.tphyobj) == 0 && t.tbip01pelvis >= 0 && t.tbip01spine >= 0 ) 
		{
			// we have what we need for a ragdoll
			bProducedRagdoll = true;

		//  StopAnimation (  immediately and record frame )
		if (  GetPlaying(t.tphyobj)  ==  1  )  StopObject (  t.tphyobj );

		//  fix pivot to normal and reset manual limb adjustments (done inside 'reset object pivot')
		ResetObjectPivot (  t.tphyobj );
		RotateObject (  t.tphyobj,0,ObjectAngleY(t.tphyobj)+180.0,0 );

		//  switch OFF LOD as ragdoll deforms for some reason (fix in future)
		SetObjectLOD (  t.tphyobj,1,50000 );
		SetObjectLOD (  t.tphyobj,2,50000 );

		//  create the ragdoll bones using the original objects bone positions then
		//  add all the original objects bones that will be moved by the ragdoll bone,
		//  set collisions groups so bones won't collide with other bones.
		Dim ( t.limidrecord , 500) ; for ( t.c = 0 ; t.c <= 500 ; t.c++ ) t.limidrecord[t.c]=0;

		// create ragdoll, and pass in weight from character
		float fWeight = t.entityelement[t.tphye].eleprof.phyweight;
		BPhys_RagDollBegin ( t.tphyobj, 175.0 * (fWeight/100.0f) );

		if (  getlimbbyname(t.tphyobj, "Crotch") == -1 ) 
		{
			if (  t.tbip01pelvis != -1 && t.tbip01spine != -1 ) 
			{
				g.Pelvis = BPhys_RagDollAddBone(t.tbip01pelvis,t.tbip01spine,10,g.tphys_CollisionGroup.group2,g.tphys_CollisionMask.mask0 + g.tphys_CollisionMask.mask1 + g.tphys_CollisionMask.mask2);
				t.tlmb = getlimbbyname(t.tphyobj, (pBitPrefix+cstr("_Pelvis")).Get()) ; if (  t.tlmb != -1 ) { BPhys_RagDollBoneAddLimbID (  g.Pelvis, t.tlmb  ) ; t.limidrecord[t.tlmb] = 1; }
				t.tlmb = getlimbbyname(t.tphyobj, pBitPrefix.Get()) ; if (  t.tlmb != -1 ) { BPhys_RagDollBoneAddLimbID (  g.Pelvis, t.tlmb  ) ; t.limidrecord[t.tlmb] = 1; }
			}
		}
		else
		{
			g.Pelvis = BPhys_RagDollAddBone(getlimbbyname(t.tphyobj, "Crotch"), getlimbbyname(t.tphyobj, (pBitPrefix+cstr("_Spine")).Get()),10,g.tphys_CollisionGroup.group2,g.tphys_CollisionMask.mask0 + g.tphys_CollisionMask.mask1 + g.tphys_CollisionMask.mask2);
			t.tlmb = getlimbbyname(t.tphyobj, (pBitPrefix+cstr("_Pelvis")).Get()) ; if (  t.tlmb != -1 ) { BPhys_RagDollBoneAddLimbID (  g.Pelvis, t.tlmb  ) ; t.limidrecord[t.tlmb] = 1; }
			t.tlmb = getlimbbyname(t.tphyobj, "Crotch") ; if (  t.tlmb != -1 ) { BPhys_RagDollBoneAddLimbID (  g.Pelvis, t.tlmb  ) ; t.limidrecord[t.tlmb] = 1; }
			t.tlmb = getlimbbyname(t.tphyobj, pBitPrefix.Get()) ; if (  t.tlmb != -1 ) { BPhys_RagDollBoneAddLimbID (  g.Pelvis, t.tlmb  ) ; t.limidrecord[t.tlmb] = 1; }
		}

			if (  getlimbbyname(t.tphyobj, (pBitPrefix+cstr("_Spine")).Get()) != -1 && getlimbbyname(t.tphyobj, (pBitPrefix+cstr("_Neck")).Get()) != -1 ) 
		{
				t.Torso = BPhys_RagDollAddBone(getlimbbyname(t.tphyobj, (pBitPrefix+cstr("_Spine")).Get()), getlimbbyname(t.tphyobj, (pBitPrefix+cstr("_Neck")).Get()), 12,g.tphys_CollisionGroup.group0,g.tphys_CollisionMask.mask0 + g.tphys_CollisionMask.mask1 + g.tphys_CollisionMask.mask2);
				t.tlmb = getlimbbyname(t.tphyobj, (pBitPrefix+cstr("_Spine")).Get()) ; if (  t.tlmb != -1 ) { BPhys_RagDollBoneAddLimbID (  t.Torso, t.tlmb ) ; t.limidrecord[t.tlmb] = 1; }
				t.tlmb = getlimbbyname(t.tphyobj, (pBitPrefix+cstr("_Spine1")).Get()) ; if (  t.tlmb != -1 ) { BPhys_RagDollBoneAddLimbID (  t.Torso, t.tlmb ) ; t.limidrecord[t.tlmb] = 1; }
				t.tlmb = getlimbbyname(t.tphyobj, (pBitPrefix+cstr("_Spine2")).Get()) ; if (  t.tlmb != -1 ) { BPhys_RagDollBoneAddLimbID (  t.Torso, t.tlmb ) ; t.limidrecord[t.tlmb] = 1; }
				t.tlmb = getlimbbyname(t.tphyobj, (pBitPrefix+cstr("_L_Clavicle")).Get()) ; if (  t.tlmb != -1 ) { BPhys_RagDollBoneAddLimbID (  t.Torso, t.tlmb ) ; t.limidrecord[t.tlmb] = 1; }
				t.tlmb = getlimbbyname(t.tphyobj, (pBitPrefix+cstr("_R_Clavicle")).Get()) ; if (  t.tlmb != -1 ) { BPhys_RagDollBoneAddLimbID (  t.Torso, t.tlmb ) ; t.limidrecord[t.tlmb] = 1; }
		}

			if (  getlimbbyname(t.tphyobj, (pBitPrefix+cstr("_Neck")).Get()) != -1 ) 
		{
				if (  getlimbbyname(t.tphyobj, (pBitPrefix+cstr("_HeadNub")).Get()) != -1 ) 
			{
					t.Head = BPhys_RagDollAddBone(getlimbbyname(t.tphyobj, (pBitPrefix+cstr("_Neck")).Get()), getlimbbyname(t.tphyobj, (pBitPrefix+cstr("_HeadNub")).Get()), 9,g.tphys_CollisionGroup.group0,g.tphys_CollisionMask.mask0 + g.tphys_CollisionMask.mask1 + g.tphys_CollisionMask.mask2);
					t.tlmb = getlimbbyname(t.tphyobj, (pBitPrefix+cstr("_HeadNub")).Get()) ; if (  t.tlmb != -1 ) { BPhys_RagDollBoneAddLimbID (  t.Head, t.tlmb  ); t.limidrecord[t.tlmb] = 1; }
			}
			else
			{
					t.Head = BPhys_RagDollAddBone(getlimbbyname(t.tphyobj, (pBitPrefix+cstr("_Neck")).Get()), getlimbbyname(t.tphyobj, (pBitPrefix+cstr("_Head")).Get()), 9,g.tphys_CollisionGroup.group0,g.tphys_CollisionMask.mask0 + g.tphys_CollisionMask.mask1 + g.tphys_CollisionMask.mask2);
			}
				t.tlmb = getlimbbyname(t.tphyobj, (pBitPrefix+cstr("_Neck")).Get()) ; if (  t.tlmb != -1 ) { BPhys_RagDollBoneAddLimbID (  t.Head, t.tlmb  ); t.limidrecord[t.tlmb] = 1; }
				t.tlmb = getlimbbyname(t.tphyobj, (pBitPrefix+cstr("_Head")).Get()) ; if (  t.tlmb != -1 ) { BPhys_RagDollBoneAddLimbID (  t.Head, t.tlmb  ); t.limidrecord[t.tlmb] = 1; }
			if (  getlimbbyname(t.tphyobj, "Head_Bone") != -1 ) 
			{
				t.tlmb = getlimbbyname(t.tphyobj, "Head_Bone") ; if (  t.tlmb != -1 ) { BPhys_RagDollBoneAddLimbID (  t.Head, t.tlmb  ); t.limidrecord[t.tlmb] = 1; }
			}
				if (  getlimbbyname(t.tphyobj, (pBitPrefix+cstr("_Ponytail1")).Get()) != -1 ) 
			{
					t.tlmb = getlimbbyname(t.tphyobj, (pBitPrefix+cstr("_Ponytail1")).Get()) ; if (  t.tlmb != -1 ) { BPhys_RagDollBoneAddLimbID (  t.Head, t.tlmb  ); t.limidrecord[t.tlmb] = 1; }
			}
				if (  getlimbbyname(t.tphyobj, (pBitPrefix+cstr("_Ponytail1Nub")).Get()) != -1 ) 
			{
					t.tlmb = getlimbbyname(t.tphyobj, (pBitPrefix+cstr("_Ponytail1Nub")).Get()) ; if (  t.tlmb != -1 ) { BPhys_RagDollBoneAddLimbID (  t.Head, t.tlmb ); t.limidrecord[t.tlmb] = 1; }
			}
		}
		t.tlmb = getlimbbyname(t.tphyobj, "Jaw_Bone") ; if (  t.tlmb != -1 ) { BPhys_RagDollBoneAddLimbID (  t.Head, t.tlmb  ) ; t.limidrecord[t.tlmb] = 1; }
		t.tlmb = getlimbbyname(t.tphyobj, "Jaw_End") ; if (  t.tlmb != -1 ) { BPhys_RagDollBoneAddLimbID (  t.Head, t.tlmb  ) ; t.limidrecord[t.tlmb] = 1; }
		t.tlmb = getlimbbyname(t.tphyobj, "L_Mouth_Down") ; if (  t.tlmb != -1 ) { BPhys_RagDollBoneAddLimbID (  t.Head, t.tlmb  ) ; t.limidrecord[t.tlmb] = 1; }
		t.tlmb = getlimbbyname(t.tphyobj, "R_Mouth_Down") ; if (  t.tlmb != -1 ) { BPhys_RagDollBoneAddLimbID (  t.Head, t.tlmb  ) ; t.limidrecord[t.tlmb] = 1; }
		t.tlmb = getlimbbyname(t.tphyobj, "M_Mouth_Down") ; if (  t.tlmb != -1 ) { BPhys_RagDollBoneAddLimbID (  t.Head, t.tlmb  ) ; t.limidrecord[t.tlmb] = 1; }
		t.tlmb = getlimbbyname(t.tphyobj, "L_Mouth_Up") ; if (  t.tlmb != -1 ) { BPhys_RagDollBoneAddLimbID (  t.Head, t.tlmb  ) ; t.limidrecord[t.tlmb] = 1; }
		t.tlmb = getlimbbyname(t.tphyobj, "R_Mouth_Up") ; if (  t.tlmb != -1 ) { BPhys_RagDollBoneAddLimbID (  t.Head, t.tlmb  ) ; t.limidrecord[t.tlmb] = 1; }
		t.tlmb = getlimbbyname(t.tphyobj, "M_Mouth_Up") ; if (  t.tlmb != -1 ) { BPhys_RagDollBoneAddLimbID (  t.Head, t.tlmb  ) ; t.limidrecord[t.tlmb] = 1; }
		t.tlmb = getlimbbyname(t.tphyobj, "L_Cheek") ; if (  t.tlmb != -1 ) { BPhys_RagDollBoneAddLimbID (  t.Head, t.tlmb  ) ; t.limidrecord[t.tlmb] = 1; }
		t.tlmb = getlimbbyname(t.tphyobj, "R_Cheek") ; if (  t.tlmb != -1 ) { BPhys_RagDollBoneAddLimbID (  t.Head, t.tlmb  ) ; t.limidrecord[t.tlmb] = 1; }
		t.tlmb = getlimbbyname(t.tphyobj, "L_Brow_inner") ; if (  t.tlmb != -1 ) { BPhys_RagDollBoneAddLimbID (  t.Head, t.tlmb  ) ; t.limidrecord[t.tlmb] = 1; }
		t.tlmb = getlimbbyname(t.tphyobj, "R_brow_inner") ; if (  t.tlmb != -1 ) { BPhys_RagDollBoneAddLimbID (  t.Head, t.tlmb  ) ; t.limidrecord[t.tlmb] = 1; }
		t.tlmb = getlimbbyname(t.tphyobj, "L_Brow_outer") ; if (  t.tlmb != -1 ) { BPhys_RagDollBoneAddLimbID (  t.Head, t.tlmb  ) ; t.limidrecord[t.tlmb] = 1; }
		t.tlmb = getlimbbyname(t.tphyobj, "R_brow_outer") ; if (  t.tlmb != -1 ) { BPhys_RagDollBoneAddLimbID (  t.Head, t.tlmb  ) ; t.limidrecord[t.tlmb] = 1; }
		t.tlmb = getlimbbyname(t.tphyobj, "L_Mouth") ; if (  t.tlmb != -1 ) { BPhys_RagDollBoneAddLimbID (  t.Head, t.tlmb  ) ; t.limidrecord[t.tlmb] = 1; }
		t.tlmb = getlimbbyname(t.tphyobj, "R_Mouth") ; if (  t.tlmb != -1 ) { BPhys_RagDollBoneAddLimbID (  t.Head, t.tlmb  ) ; t.limidrecord[t.tlmb] = 1; }
		t.tlmb = getlimbbyname(t.tphyobj, "R_eyelid_up") ; if (  t.tlmb != -1 ) { BPhys_RagDollBoneAddLimbID (  t.Head, t.tlmb  ) ; t.limidrecord[t.tlmb] = 1; }
		t.tlmb = getlimbbyname(t.tphyobj, "L_eyelid_up") ; if (  t.tlmb != -1 ) { BPhys_RagDollBoneAddLimbID (  t.Head, t.tlmb  ) ; t.limidrecord[t.tlmb] = 1; }
		t.tlmb = getlimbbyname(t.tphyobj, "R_eyelid_down") ; if (  t.tlmb != -1 ) { BPhys_RagDollBoneAddLimbID (  t.Head, t.tlmb  ) ; t.limidrecord[t.tlmb] = 1; }
		t.tlmb = getlimbbyname(t.tphyobj, "L_eyelid_down") ; if (  t.tlmb != -1 ) { BPhys_RagDollBoneAddLimbID (  t.Head, t.tlmb  ) ; t.limidrecord[t.tlmb] = 1; }

			if (  getlimbbyname(t.tphyobj, (pBitPrefix+cstr("_L_UpperArm")).Get()) != -1 && getlimbbyname(t.tphyobj, (pBitPrefix+cstr("_L_Forearm")).Get()) != -1 ) 
		{
				t.L_UpperArm = BPhys_RagDollAddBone(getlimbbyname(t.tphyobj, (pBitPrefix+cstr("_L_UpperArm")).Get()), getlimbbyname(t.tphyobj, (pBitPrefix+cstr("_L_Forearm")).Get()), 9,g.tphys_CollisionGroup.group0,g.tphys_CollisionMask.mask0 + g.tphys_CollisionMask.mask1 );
				t.tlmb = getlimbbyname(t.tphyobj, (pBitPrefix+cstr("_L_UpperArm")).Get()) ; if (  t.tlmb != -1 ) { BPhys_RagDollBoneAddLimbID (  t.L_UpperArm, t.tlmb  ); t.limidrecord[t.tlmb] = 1; }
		}

			if (  getlimbbyname(t.tphyobj, (pBitPrefix+cstr("_L_Forearm")).Get()) != -1 && getlimbbyname(t.tphyobj, (pBitPrefix+cstr("_L_Hand")).Get()) != -1 ) 
		{
				g.L_Forearm = BPhys_RagDollAddBone(getlimbbyname(t.tphyobj, (pBitPrefix+cstr("_L_Forearm")).Get()), getlimbbyname(t.tphyobj, (pBitPrefix+cstr("_L_Hand")).Get()), 7,g.tphys_CollisionGroup.group0,g.tphys_CollisionMask.mask0 + g.tphys_CollisionMask.mask2);
				t.tlmb = getlimbbyname(t.tphyobj, (pBitPrefix+cstr("_L_Forearm")).Get()) ; if (  t.tlmb != -1 ) { BPhys_RagDollBoneAddLimbID (  g.L_Forearm, t.tlmb  ) ; t.limidrecord[t.tlmb] = 1; }
		}

			if (  getlimbbyname(t.tphyobj, (pBitPrefix+cstr("_L_Hand")).Get()) != -1 ) 
		{
				if (  getlimbbyname(t.tphyobj, (pBitPrefix+cstr("_L_Finger0Nub")).Get()) != -1 ) 
			{
					g.L_Hand = BPhys_RagDollAddBone(getlimbbyname(t.tphyobj, (pBitPrefix+cstr("_L_Hand")).Get()), getlimbbyname(t.tphyobj, (pBitPrefix+cstr("_L_Finger0Nub")).Get()), 5,g.tphys_CollisionGroup.group0,g.tphys_CollisionMask.mask0 + g.tphys_CollisionMask.mask1 + g.tphys_CollisionMask.mask2);
					t.tlmb = getlimbbyname(t.tphyobj, (pBitPrefix+cstr("_L_Finger0Nub")).Get()) ; if (  t.tlmb != -1 ) { BPhys_RagDollBoneAddLimbID (  g.L_Hand, t.tlmb  ) ; t.limidrecord[t.tlmb] = 1; }
			}
			else
			{
					if (  getlimbbyname(t.tphyobj, (pBitPrefix+cstr("_L_Finger0")).Get()) != -1 ) 
				{
						g.L_Hand = BPhys_RagDollAddBone(getlimbbyname(t.tphyobj, (pBitPrefix+cstr("_L_Hand")).Get()), getlimbbyname(t.tphyobj, (pBitPrefix+cstr("_L_Finger0")).Get()), 5,g.tphys_CollisionGroup.group0,g.tphys_CollisionMask.mask0 + g.tphys_CollisionMask.mask1 + g.tphys_CollisionMask.mask2);
				}
				else
				{
						g.L_Hand = BPhys_RagDollAddBone(getlimbbyname(t.tphyobj, (pBitPrefix+cstr("_L_Hand")).Get()), getlimbbyname(t.tphyobj, (pBitPrefix+cstr("_L_Finger")).Get()), 5,g.tphys_CollisionGroup.group0,g.tphys_CollisionMask.mask0 + g.tphys_CollisionMask.mask1 + g.tphys_CollisionMask.mask2);
				}
			}
				t.tlmb = getlimbbyname(t.tphyobj, (pBitPrefix+cstr("_L_Hand")).Get()) ; if (  t.tlmb != -1 ) { BPhys_RagDollBoneAddLimbID (  g.L_Hand, t.tlmb  ) ; t.limidrecord[t.tlmb] = 1; }
				t.tlmb = getlimbbyname(t.tphyobj, (pBitPrefix+cstr("_L_Finger0")).Get()) ; if (  t.tlmb != -1 ) { BPhys_RagDollBoneAddLimbID (  g.L_Hand, t.tlmb ) ; t.limidrecord[t.tlmb] = 1; }
				t.tlmb = getlimbbyname(t.tphyobj, (pBitPrefix+cstr("_L_Finger")).Get()) ; if (  t.tlmb != -1 ) { BPhys_RagDollBoneAddLimbID (  g.L_Hand, t.tlmb ) ; t.limidrecord[t.tlmb] = 1; }
		}

			if (  getlimbbyname(t.tphyobj, (pBitPrefix+cstr("_L_Calf")).Get()) != -1 && getlimbbyname(t.tphyobj, (pBitPrefix+cstr("_L_Thigh")).Get()) != -1 ) 
		{
				t.L_Thigh = BPhys_RagDollAddBone(getlimbbyname(t.tphyobj, (pBitPrefix+cstr("_L_Calf")).Get() ), getlimbbyname(t.tphyobj, (pBitPrefix+cstr("_L_Thigh")).Get() ), 8,g.tphys_CollisionGroup.group2,g.tphys_CollisionMask.mask0 + g.tphys_CollisionMask.mask2);
				t.tlmb = getlimbbyname(t.tphyobj, (pBitPrefix+cstr("_L_Thigh")).Get()) ; if (  t.tlmb != -1 ) { BPhys_RagDollBoneAddLimbID (  t.L_Thigh, t.tlmb  ) ; t.limidrecord[t.tlmb] = 1; }
		}

		if (  getlimbbyname(t.tphyobj, "l_Heel") == -1 ) 
		{
				if (  getlimbbyname(t.tphyobj, (pBitPrefix+cstr("_L_Foot")).Get()) != -1 && getlimbbyname(t.tphyobj, (pBitPrefix+cstr("_L_Calf")).Get()) != -1 ) 
			{
					g.L_Calf = BPhys_RagDollAddBone2(getlimbbyname(t.tphyobj, (pBitPrefix+cstr("_L_Foot")).Get()), getlimbbyname(t.tphyobj, (pBitPrefix+cstr("_L_Calf")).Get()), 8, 1.5, g.tphys_CollisionGroup.group2,g.tphys_CollisionMask.mask0 + g.tphys_CollisionMask.mask1 + g.tphys_CollisionMask.mask2);
					t.tlmb = getlimbbyname(t.tphyobj, (pBitPrefix+cstr("_L_Calf")).Get()) ; if (  t.tlmb != -1 ) { BPhys_RagDollBoneAddLimbID (  g.L_Calf, t.tlmb  ) ; t.limidrecord[t.tlmb] = 1; }
					t.tlmb = getlimbbyname(t.tphyobj, (pBitPrefix+cstr("_L_Foot")).Get()) ; if (  t.tlmb != -1 ) { BPhys_RagDollBoneAddLimbID (  g.L_Calf, t.tlmb  ) ; t.limidrecord[t.tlmb] = 1; }
			}
		}
		else
		{
				if (  getlimbbyname(t.tphyobj, "l_Heel") != -1 && getlimbbyname(t.tphyobj, (pBitPrefix+cstr("_L_Calf")).Get()) != -1 ) 
			{
					g.L_Calf = BPhys_RagDollAddBone(getlimbbyname(t.tphyobj, "l_Heel"), getlimbbyname(t.tphyobj, (pBitPrefix+cstr("_L_Calf")).Get()), 6,g.tphys_CollisionGroup.group2,g.tphys_CollisionMask.mask0 + g.tphys_CollisionMask.mask1 + g.tphys_CollisionMask.mask2);
					t.tlmb = getlimbbyname(t.tphyobj, (pBitPrefix+cstr("_L_Calf")).Get()) ; if (  t.tlmb != -1 ) { BPhys_RagDollBoneAddLimbID (  g.L_Calf, t.tlmb  ) ; t.limidrecord[t.tlmb] = 1; }
					t.tlmb = getlimbbyname(t.tphyobj, (pBitPrefix+cstr("_L_Foot")).Get()) ; if (  t.tlmb != -1 ) { BPhys_RagDollBoneAddLimbID (  g.L_Calf, t.tlmb  ) ; t.limidrecord[t.tlmb] = 1; }
				t.tlmb = getlimbbyname(t.tphyobj, "l_Heel") ; if (  t.tlmb != -1 ) { BPhys_RagDollBoneAddLimbID (  g.L_Calf, t.tlmb  ) ; t.limidrecord[t.tlmb] = 1; }
			}
		}

			if (  getlimbbyname(t.tphyobj, (pBitPrefix+cstr("_R_UpperArm")).Get()) != -1 && getlimbbyname(t.tphyobj, (pBitPrefix+cstr("_R_Forearm")).Get()) != -1 ) 
		{
				t.R_UpperArm = BPhys_RagDollAddBone(getlimbbyname(t.tphyobj, (pBitPrefix+cstr("_R_UpperArm")).Get()), getlimbbyname(t.tphyobj, (pBitPrefix+cstr("_R_Forearm")).Get()), 9,g.tphys_CollisionGroup.group0,g.tphys_CollisionMask.mask0 + g.tphys_CollisionMask.mask1 );
				t.tlmb = getlimbbyname(t.tphyobj, (pBitPrefix+cstr("_R_UpperArm")).Get()) ; if (  t.tlmb != -1 ) { BPhys_RagDollBoneAddLimbID (  t.R_UpperArm, t.tlmb  ) ; t.limidrecord[t.tlmb] = 1; }
		}

			if (  getlimbbyname(t.tphyobj, (pBitPrefix+cstr("_R_Forearm")).Get()) != -1 && getlimbbyname(t.tphyobj, (pBitPrefix+cstr("_R_Hand")).Get()) != -1 ) 
		{
				t.R_Forearm = BPhys_RagDollAddBone(getlimbbyname(t.tphyobj, (pBitPrefix+cstr("_R_Forearm")).Get()), getlimbbyname(t.tphyobj, (pBitPrefix+cstr("_R_Hand")).Get()), 7,g.tphys_CollisionGroup.group0,g.tphys_CollisionMask.mask0 + g.tphys_CollisionMask.mask2);
				t.tlmb = getlimbbyname(t.tphyobj, (pBitPrefix+cstr("_R_Forearm")).Get()) ; if (  t.tlmb != -1 ) { BPhys_RagDollBoneAddLimbID (  t.R_Forearm, t.tlmb  ) ; t.limidrecord[t.tlmb] = 1; }
		}

			if (  getlimbbyname(t.tphyobj, (pBitPrefix+cstr("_R_Hand")).Get()) != -1 ) 
		{
				if (  getlimbbyname(t.tphyobj, (pBitPrefix+cstr("_R_Finger0Nub")).Get()) != -1 ) 
			{
					t.R_Hand = BPhys_RagDollAddBone(getlimbbyname(t.tphyobj, (pBitPrefix+cstr("_R_Hand")).Get()), getlimbbyname(t.tphyobj, (pBitPrefix+cstr("_R_Finger0Nub")).Get()), 5,g.tphys_CollisionGroup.group0,g.tphys_CollisionMask.mask0 + g.tphys_CollisionMask.mask1 + g.tphys_CollisionMask.mask2);
					t.tlmb = getlimbbyname(t.tphyobj, (pBitPrefix+cstr("_R_Finger0Nub")).Get()) ; if (  t.tlmb != -1 ) { BPhys_RagDollBoneAddLimbID (  t.R_Hand, t.tlmb  ) ; t.limidrecord[t.tlmb] = 1; }
			}
			else
			{
					t.R_Hand = BPhys_RagDollAddBone(getlimbbyname(t.tphyobj, (pBitPrefix+cstr("_R_Hand")).Get()), getlimbbyname(t.tphyobj, (pBitPrefix+cstr("_R_Finger0")).Get()), 5,g.tphys_CollisionGroup.group0,g.tphys_CollisionMask.mask0 + g.tphys_CollisionMask.mask1 + g.tphys_CollisionMask.mask2);
			}
				t.tlmb = getlimbbyname(t.tphyobj, (pBitPrefix+cstr("_R_Hand")).Get()) ; if (  t.tlmb != -1 ) { BPhys_RagDollBoneAddLimbID (  t.R_Hand, t.tlmb  ) ; t.limidrecord[t.tlmb] = 1; }
				t.tlmb = getlimbbyname(t.tphyobj, (pBitPrefix+cstr("_R_Finger0")).Get()) ; if (  t.tlmb != -1 ) { BPhys_RagDollBoneAddLimbID (  t.R_Hand, t.tlmb ) ; t.limidrecord[t.tlmb] = 1; };
			if (  getlimbbyname(t.tphyobj, "FIRESPOT") != -1 ) 
			{
				t.tlmb = getlimbbyname(t.tphyobj, "FIRESPOT") ; if (  t.tlmb != -1 ) { BPhys_RagDollBoneAddLimbID (  t.R_Hand, t.tlmb ) ; t.limidrecord[t.tlmb] = 1; }
			}
		}

			if (  getlimbbyname(t.tphyobj, (pBitPrefix+cstr("_R_Calf")).Get()) != -1 ) 
		{
				t.R_Thigh = BPhys_RagDollAddBone(getlimbbyname(t.tphyobj, (pBitPrefix+cstr("_R_Calf")).Get() ), getlimbbyname(t.tphyobj, (pBitPrefix+cstr("_R_Thigh")).Get()), 8,g.tphys_CollisionGroup.group2,g.tphys_CollisionMask.mask0 + g.tphys_CollisionMask.mask2);
				t.tlmb = getlimbbyname(t.tphyobj, (pBitPrefix+cstr("_R_Thigh")).Get()) ; if (  t.tlmb != -1 ) { BPhys_RagDollBoneAddLimbID (  t.R_Thigh, t.tlmb  ) ; t.limidrecord[t.tlmb] = 1; }
		}

		if (  getlimbbyname(t.tphyobj, "r_Heel") == -1 ) 
		{
				if (  getlimbbyname(t.tphyobj, (pBitPrefix+cstr("_R_Foot")).Get()) != -1 && getlimbbyname(t.tphyobj, (pBitPrefix+cstr("_R_Calf")).Get()) != -1 ) 
			{
					t.R_Calf = BPhys_RagDollAddBone2(getlimbbyname(t.tphyobj, (pBitPrefix+cstr("_R_Foot")).Get()), getlimbbyname(t.tphyobj, (pBitPrefix+cstr("_R_Calf")).Get()), 8, 1.5, g.tphys_CollisionGroup.group2,g.tphys_CollisionMask.mask0 + g.tphys_CollisionMask.mask1 + g.tphys_CollisionMask.mask2);
					t.tlmb = getlimbbyname(t.tphyobj, (pBitPrefix+cstr("_R_Calf")).Get()) ; if (  t.tlmb != -1 ) { BPhys_RagDollBoneAddLimbID (  t.R_Calf, t.tlmb  ) ; t.limidrecord[t.tlmb] = 1; }
					t.tlmb = getlimbbyname(t.tphyobj, (pBitPrefix+cstr("_R_Foot")).Get()) ; if (  t.tlmb != -1 ) { BPhys_RagDollBoneAddLimbID (  t.R_Calf, t.tlmb  ) ; t.limidrecord[t.tlmb] = 1; }
			}
		}
		else
		{
				t.R_Calf = BPhys_RagDollAddBone(getlimbbyname(t.tphyobj, "r_Heel"), getlimbbyname(t.tphyobj, (pBitPrefix+cstr("_R_Calf")).Get()), 6,g.tphys_CollisionGroup.group2,g.tphys_CollisionMask.mask0 + g.tphys_CollisionMask.mask1 + g.tphys_CollisionMask.mask2);
				t.tlmb = getlimbbyname(t.tphyobj, (pBitPrefix+cstr("_R_Calf")).Get()) ; if (  t.tlmb != -1 ) { BPhys_RagDollBoneAddLimbID (  t.R_Calf, t.tlmb  ) ; t.limidrecord[t.tlmb] = 1; }
				t.tlmb = getlimbbyname(t.tphyobj, (pBitPrefix+cstr("_R_Foot")).Get()) ; if (  t.tlmb != -1 ) { BPhys_RagDollBoneAddLimbID (  t.R_Calf, t.tlmb  ) ; t.limidrecord[t.tlmb] = 1; }
			t.tlmb = getlimbbyname(t.tphyobj, "r_Heel") ; if (  t.tlmb != -1 ) { BPhys_RagDollBoneAddLimbID (  t.R_Calf, t.tlmb  ) ; t.limidrecord[t.tlmb] = 1; }
		}

		//  Detect limbs not known by name, and add to hierarchy where they where found
		PerformCheckListForLimbs (  t.tphyobj );
		t.modellastknownlimb=g.Pelvis;
		for ( t.c = 1 ; t.c<=  ChecklistQuantity()-1; t.c++ )
		{
			//  track known limb names
			t.tokay=-1 ; t.tname_s=Lower(ChecklistString(t.c));
				if (  t.tname_s == cstr(Lower(pBitPrefix.Get()))  )  t.tokay = g.Pelvis;
				if (  t.tname_s == cstr(Lower((pBitPrefix+cstr("_Pelvis")).Get()))  )  t.tokay = g.Pelvis;
				if (  t.tname_s == cstr(Lower((pBitPrefix+cstr("_Spine")).Get()))  )  t.tokay = t.Torso;
				if (  t.tname_s == cstr(Lower((pBitPrefix+cstr("_Spine1")).Get()))  )  t.tokay = t.Torso;
				if (  t.tname_s == cstr(Lower((pBitPrefix+cstr("_Spine2")).Get()))  )  t.tokay = t.Torso;
				if (  t.tname_s == cstr(Lower((pBitPrefix+cstr("_L_Clavicle")).Get()))  )  t.tokay = t.Torso;
				if (  t.tname_s == cstr(Lower((pBitPrefix+cstr("_R_Clavicle")).Get()))  )  t.tokay = t.Torso;
				if (  t.tname_s == cstr(Lower((pBitPrefix+cstr("_Neck")).Get()))  )  t.tokay = t.Head;
				if (  t.tname_s == cstr(Lower((pBitPrefix+cstr("_Head")).Get()))  )  t.tokay = t.Head;
				if (  t.tname_s == cstr(Lower((pBitPrefix+cstr("_HeadNub")).Get()))  )  t.tokay = t.Head;
				if (  t.tname_s == cstr(Lower((pBitPrefix+cstr("_Ponytail1")).Get()))  )  t.tokay = t.Head;
				if (  t.tname_s == cstr(Lower((pBitPrefix+cstr("_Ponytail1Nub")).Get()))  )  t.tokay = t.Head;
				if (  t.tname_s == cstr(Lower((pBitPrefix+cstr("_L_UpperArm")).Get()))  )  t.tokay = t.L_UpperArm;
				if (  t.tname_s == cstr(Lower((pBitPrefix+cstr("_L_Forearm")).Get()))  )  t.tokay = t.L_UpperArm;
				if (  t.tname_s == cstr(Lower((pBitPrefix+cstr("_L_Hand")).Get()))  )  t.tokay = g.L_Hand;
				if (  t.tname_s == cstr(Lower((pBitPrefix+cstr("_L_Finger0")).Get()))  )  t.tokay = g.L_Hand;
				if (  t.tname_s == cstr(Lower((pBitPrefix+cstr("_L_Finger0Nub")).Get()))  )  t.tokay = g.L_Hand;
				if (  t.tname_s == cstr(Lower((pBitPrefix+cstr("_L_Thigh")).Get()))  )  t.tokay = t.L_Thigh;
				if (  t.tname_s == cstr(Lower((pBitPrefix+cstr("_L_Calf")).Get()))  )  t.tokay = g.L_Calf;
				if (  t.tname_s == cstr(Lower((pBitPrefix+cstr("_L_Foot")).Get()))  )  t.tokay = g.L_Calf;
			if (  t.tname_s == cstr(Lower("L_Mouth_Up"))  )  t.tokay = t.Head;
			if (  t.tname_s == cstr(Lower("L_Mouth_Down"))  )  t.tokay = t.Head;
			if (  t.tname_s == cstr(Lower("L_Cheek"))  )  t.tokay = t.Head;
			if (  t.tname_s == cstr(Lower("L_Brow_inner"))  )  t.tokay = t.Head;
			if (  t.tname_s == cstr(Lower("L_Brow_outer"))  )  t.tokay = t.Head;
			if (  t.tname_s == cstr(Lower("L_Mouth"))  )  t.tokay = t.Head;
			if (  t.tname_s == cstr(Lower("L_eyelid_up"))  )  t.tokay = t.Head;
			if (  t.tname_s == cstr(Lower("L_eyelid_down"))  )  t.tokay = t.Head;
			if (  t.tname_s == cstr(Lower("l_Heel"))  )  t.tokay = g.L_Calf;
				if (  t.tname_s == cstr(Lower((pBitPrefix+cstr("_R_UpperArm")).Get()))  )  t.tokay = t.R_UpperArm;
				if (  t.tname_s == cstr(Lower((pBitPrefix+cstr("_R_Forearm")).Get()))  )  t.tokay = t.R_Forearm;
				if (  t.tname_s == cstr(Lower((pBitPrefix+cstr("_R_Hand")).Get()))  )  t.tokay = t.R_Hand;
				if (  t.tname_s == cstr(Lower((pBitPrefix+cstr("_R_Finger0")).Get()))  )  t.tokay = t.R_Hand;
				if (  t.tname_s == cstr(Lower((pBitPrefix+cstr("_R_Finger0Nub")).Get()))  )  t.tokay = t.R_Hand;
				if (  t.tname_s == cstr(Lower((pBitPrefix+cstr("_R_Thigh")).Get()))  )  t.tokay = t.R_Thigh;
				if (  t.tname_s == cstr(Lower((pBitPrefix+cstr("_R_Calf")).Get()))  )  t.tokay = t.R_Calf;
				if (  t.tname_s == cstr(Lower((pBitPrefix+cstr("_R_Foot")).Get()))  )  t.tokay = t.R_Calf;
			if (  t.tname_s == cstr(Lower("R_Mouth_Up"))  )  t.tokay = t.Head;
			if (  t.tname_s == cstr(Lower("R_Mouth_Down"))  )  t.tokay = t.Head;
			if (  t.tname_s == cstr(Lower("R_Cheek"))  )  t.tokay = t.Head;
			if (  t.tname_s == cstr(Lower("R_brow_inner"))  )  t.tokay = t.Head;
			if (  t.tname_s == cstr(Lower("R_brow_outer"))  )  t.tokay = t.Head;
			if (  t.tname_s == cstr(Lower("R_Mouth"))  )  t.tokay = t.Head;
			if (  t.tname_s == cstr(Lower("R_eyelid_up"))  )  t.tokay = t.Head;
			if (  t.tname_s == cstr(Lower("R_eyelid_down"))  )  t.tokay = t.Head;
			if (  t.tname_s == cstr(Lower("r_Heel"))  )  t.tokay = t.R_Calf;
			if (  t.tname_s == cstr(Lower("Crotch"))  )  t.tokay = g.Pelvis;
			if (  t.tname_s == cstr(Lower("Head_Bone"))  )  t.tokay = t.Head;
			if (  t.tname_s == cstr(Lower("Jaw_Bone"))  )  t.tokay = t.Head;
			if (  t.tname_s == cstr(Lower("Jaw_End"))  )  t.tokay = t.Head;
			if (  t.tname_s == cstr(Lower("M_Mouth_Up"))  )  t.tokay = t.Head;
			if (  t.tname_s == cstr(Lower("M_Mouth_Down"))  )  t.tokay = t.Head;
			if (  t.tname_s == cstr(Lower("FIRESPOT"))  )  t.tokay = t.R_Hand;
			if (  t.tokay != -1 ) 
			{
				//  limb context recorded
				t.modellastknownlimb=t.tokay;
			}
			else
			{
				//  if limb not recognised, add to complete ragdoll
				t.tlmb = getlimbbyname(t.tphyobj, t.tname_s.Get()) ; if (  t.tlmb != -1 ) { BPhys_RagDollBoneAddLimbID (  t.modellastknownlimb, t.tlmb  ) ; t.limidrecord[t.tlmb] = 1; }
			}
				// 221215 - this code below should NEVER work, check for zero, then check for one..
			if (  t.limidrecord[t.c-1] == 0 ) 
			{
				t.tlmb = t.c-1 ; if (  t.limidrecord[t.tlmb] == 1  )  BPhys_RagDollBoneAddLimbID (  t.modellastknownlimb,t.tlmb );
			}
		}

			// 221215 - finally add everything else that was not captured (so ragdoll does not explode verts)
			//for ( t.c = 1 ; t.c <= ChecklistQuantity(); t.c++ )
			//{
			//	if (  t.limidrecord[t.c-1] == 0 ) 
			//	{
			//		t.tlmb = t.c-1; BPhys_RagDollBoneAddLimbID ( t.modellastknownlimb, t.tlmb );
			//	}
			//}

		//  Now we create a joint between the bones we created, joints are positioned at the original objects joints and set the rotation and limits
		if (  getlimbbyname(t.tphyobj, "Bip01_Neck") != -1  )  BPhys_RagDollAddTwistJoint (  t.Torso, t.Head, getlimbbyname(t.tphyobj, "Bip01_Neck"),g.NeckJoint.rotationVec3, g.NeckJoint.limitsVec3 );
		if (  getlimbbyname(t.tphyobj, "Bip01_Spine") != -1  )  BPhys_RagDollAddHingeJoint (  g.Pelvis, t.Torso, getlimbbyname(t.tphyobj, "Bip01_Spine"), g.WaistJoint.rotationVec3, g.WaistJoint.limitsVec2 );
		if (  getlimbbyname(t.tphyobj, "Bip01_L_UpperArm") != -1  )  BPhys_RagDollAddTwistJoint (  t.Torso, t.L_UpperArm, getlimbbyname(t.tphyobj, "Bip01_L_UpperArm"), g.LeftShoulderJoint.rotationVec3, g.LeftShoulderJoint.limitsVec3 );
		if (  getlimbbyname(t.tphyobj, "Bip01_L_Forearm") != -1  )  BPhys_RagDollAddHingeJoint (  t.L_UpperArm, g.L_Forearm, getlimbbyname(t.tphyobj, "Bip01_L_Forearm"), g.LeftElbowJoint.rotationVec3, g.LeftElbowJoint.limitsVec2 );
		if (  getlimbbyname(t.tphyobj, "Bip01_R_Forearm") != -1  )  BPhys_RagDollAddHingeJoint (  t.R_UpperArm, t.R_Forearm, getlimbbyname(t.tphyobj, "Bip01_R_Forearm"), g.RightElbowJoint.rotationVec3, g.RightElbowJoint.limitsVec2 );
		if (  getlimbbyname(t.tphyobj, "Bip01_L_Hand") != -1  )  BPhys_RagDollAddHingeJoint (  g.L_Forearm, g.L_Hand, getlimbbyname(t.tphyobj, "Bip01_L_Hand"), g.leftWristJoint.rotationVec3, g.leftWristJoint.limitsVec2 );
		if (  getlimbbyname(t.tphyobj, "Bip01_R_Hand") != -1  )  BPhys_RagDollAddHingeJoint (  t.R_Forearm, t.R_Hand, getlimbbyname(t.tphyobj, "Bip01_R_Hand"), g.rightWristJoint.rotationVec3, g.rightWristJoint.limitsVec2 );
		if (  getlimbbyname(t.tphyobj, "Bip01_L_Thigh") != -1  )  BPhys_RagDollAddTwistJoint (  g.Pelvis, t.L_Thigh, getlimbbyname(t.tphyobj, "Bip01_L_Thigh"), g.HipJoint.rotationVec3, g.HipJoint.limitsVec3 );
		if (  getlimbbyname(t.tphyobj, "Bip01_R_UpperArm") != -1  )  BPhys_RagDollAddTwistJoint (  t.Torso, t.R_UpperArm, getlimbbyname(t.tphyobj, "Bip01_R_UpperArm"), g.RightShoulderJoint.rotationVec3, g.RightShoulderJoint.limitsVec3 );
		if (  getlimbbyname(t.tphyobj, "Bip01_R_Thigh") != -1  )  BPhys_RagDollAddTwistJoint (  g.Pelvis, t.R_Thigh, getlimbbyname(t.tphyobj, "Bip01_R_Thigh"), g.HipJoint.rotationVec3, g.HipJoint.limitsVec3 );
		if (  getlimbbyname(t.tphyobj, "Bip01_L_Calf") != -1  )  BPhys_RagDollAddHingeJoint (  t.L_Thigh, g.L_Calf, getlimbbyname(t.tphyobj, "Bip01_L_Calf"), g.KneeJoint.rotationVec3, g.KneeJoint.limitsVec2 );
		if (  getlimbbyname(t.tphyobj, "Bip01_R_Calf") != -1  )  BPhys_RagDollAddHingeJoint (  t.R_Thigh, t.R_Calf, getlimbbyname(t.tphyobj, "Bip01_R_Calf"), g.KneeJoint.rotationVec3, g.KneeJoint.limitsVec2 );
			if (  getlimbbyname(t.tphyobj, (pBitPrefix+cstr("_Neck")).Get()) != -1  )  BPhys_RagDollAddTwistJoint (  t.Torso, t.Head, getlimbbyname(t.tphyobj, (pBitPrefix+cstr("_Neck")).Get()),g.NeckJoint.rotationVec3, g.NeckJoint.limitsVec3 );
			if (  getlimbbyname(t.tphyobj, (pBitPrefix+cstr("_Spine")).Get()) != -1  )  BPhys_RagDollAddHingeJoint (  g.Pelvis, t.Torso, getlimbbyname(t.tphyobj, (pBitPrefix+cstr("_Spine")).Get()), g.WaistJoint.rotationVec3, g.WaistJoint.limitsVec2 );
			if (  getlimbbyname(t.tphyobj, (pBitPrefix+cstr("_L_UpperArm")).Get()) != -1  )  BPhys_RagDollAddTwistJoint (  t.Torso, t.L_UpperArm, getlimbbyname(t.tphyobj, (pBitPrefix+cstr("_L_UpperArm")).Get()), g.LeftShoulderJoint.rotationVec3, g.LeftShoulderJoint.limitsVec3 );
			if (  getlimbbyname(t.tphyobj, (pBitPrefix+cstr("_L_Forearm")).Get()) != -1  )  BPhys_RagDollAddHingeJoint (  t.L_UpperArm, g.L_Forearm, getlimbbyname(t.tphyobj, (pBitPrefix+cstr("_L_Forearm")).Get()), g.LeftElbowJoint.rotationVec3, g.LeftElbowJoint.limitsVec2 );
			if (  getlimbbyname(t.tphyobj, (pBitPrefix+cstr("_R_Forearm")).Get()) != -1  )  BPhys_RagDollAddHingeJoint (  t.R_UpperArm, t.R_Forearm, getlimbbyname(t.tphyobj, (pBitPrefix+cstr("_R_Forearm")).Get()), g.RightElbowJoint.rotationVec3, g.RightElbowJoint.limitsVec2 );
			if (  getlimbbyname(t.tphyobj, (pBitPrefix+cstr("_L_Hand")).Get()) != -1  )  BPhys_RagDollAddHingeJoint (  g.L_Forearm, g.L_Hand, getlimbbyname(t.tphyobj, (pBitPrefix+cstr("_L_Hand")).Get()), g.leftWristJoint.rotationVec3, g.leftWristJoint.limitsVec2 );
			if (  getlimbbyname(t.tphyobj, (pBitPrefix+cstr("_R_Hand")).Get()) != -1  )  BPhys_RagDollAddHingeJoint (  t.R_Forearm, t.R_Hand, getlimbbyname(t.tphyobj, (pBitPrefix+cstr("_R_Hand")).Get()), g.rightWristJoint.rotationVec3, g.rightWristJoint.limitsVec2 );
			if (  getlimbbyname(t.tphyobj, (pBitPrefix+cstr("_L_Thigh")).Get()) != -1  )  BPhys_RagDollAddTwistJoint (  g.Pelvis, t.L_Thigh, getlimbbyname(t.tphyobj, (pBitPrefix+cstr("_L_Thigh")).Get()), g.HipJoint.rotationVec3, g.HipJoint.limitsVec3 );
			if (  getlimbbyname(t.tphyobj, (pBitPrefix+cstr("_R_UpperArm")).Get()) != -1  )  BPhys_RagDollAddTwistJoint (  t.Torso, t.R_UpperArm, getlimbbyname(t.tphyobj, (pBitPrefix+cstr("_R_UpperArm")).Get()), g.RightShoulderJoint.rotationVec3, g.RightShoulderJoint.limitsVec3 );
			if (  getlimbbyname(t.tphyobj, (pBitPrefix+cstr("_R_Thigh")).Get()) != -1  )  BPhys_RagDollAddTwistJoint (  g.Pelvis, t.R_Thigh, getlimbbyname(t.tphyobj, (pBitPrefix+cstr("_R_Thigh")).Get()), g.HipJoint.rotationVec3, g.HipJoint.limitsVec3 );
			if (  getlimbbyname(t.tphyobj, (pBitPrefix+cstr("_L_Calf")).Get()) != -1  )  BPhys_RagDollAddHingeJoint (  t.L_Thigh, g.L_Calf, getlimbbyname(t.tphyobj, (pBitPrefix+cstr("_L_Calf")).Get()), g.KneeJoint.rotationVec3, g.KneeJoint.limitsVec2 );
			if (  getlimbbyname(t.tphyobj, (pBitPrefix+cstr("_R_Calf")).Get()) != -1  )  BPhys_RagDollAddHingeJoint (  t.R_Thigh, t.R_Calf, getlimbbyname(t.tphyobj, (pBitPrefix+cstr("_R_Calf")).Get()), g.KneeJoint.rotationVec3, g.KneeJoint.limitsVec2 );

		//  Remaining ragdoll settings
		BPhys_RagDollSetDamping (   0.08f, 0.95f );
		BPhys_RagDollSetSleepingThresholds (  1.8f, 2.8f );
		BPhys_RagDollSetDeactivationTime (  0.2f );

		//  Complete ragdoll creation
		BPhys_RagDollEnd (  );
		UnDim (  t.limidrecord );

		//  add to ragdoll database
		for ( t.ri = 0 ; t.ri<=  g.RagdollsMax; t.ri++ )
		{
			if (  t.Ragdolls[t.ri].obj == 0  )  break;
		}
		if (  t.ri>g.RagdollsMax ) 
		{
			++g.RagdollsMax;
			g.RagdollsMax=g.RagdollsMax*2;
			Dim (  t.Ragdolls,g.RagdollsMax );
		}
		t.Ragdolls[t.ri].obj = t.tphyobj;

		// 210516 - and finally set friction for ragdoll (perhaps it helps LUA crash issue)?!
		//ODESetBodyFriction ( t.tphyobj, t.entityelement[t.tphye].eleprof.phyfriction );

		//  hide debug ragdoll capsules
		t.temp = BPhys_RagDollHideBones(t.tphyobj);

	}

		// once we have a working ragdoll, we leave
		if ( bProducedRagdoll==true ) return;
}
}

void ragdoll_destroy ( void )
{
	//  receives tphyobj
	if (  t.tphyobj>0 ) 
	{
		if (  ObjectExist(t.tphyobj) == 1  )  ResetObjectPivot (  t.tphyobj );
		for ( t.ri = 0 ; t.ri<=  g.RagdollsMax; t.ri++ )
		{
			if (  t.tphyobj == t.Ragdolls[t.ri].obj ) 
			{
				if (  BPhys_DeleteRagdoll(t.tphyobj) == 1 ) 
				{
					t.Ragdolls[t.ri].obj=0 ; t.ri=g.RagdollsMax ; break;
				}
			}
		}
	}
}

int newvector3 ( int x_f, int y_f, int z_f )
{
	int r = 0;
	++g.ragdollvectorindex;
	r=MakeVector3(g.ragdollvectorindex);
	if (  ArrayCount(t.ragdollvector) < g.ragdollvectorindex ) 
	{
		Dim (  t.ragdollvector,g.ragdollvectorindex  );
	}
	t.ragdollvector[g.ragdollvectorindex]=1;
	SetVector3 (  g.ragdollvectorindex,x_f,y_f,z_f );
//endfunction ragdollvectorindex
	return g.ragdollvectorindex;
}

int newvector2 ( int x_f, int y_f )
{
	int r = 0;
	++g.ragdollvectorindex;
	r=MakeVector2(g.ragdollvectorindex);
	if (  ArrayCount(t.ragdollvector) < g.ragdollvectorindex ) 
	{
		Dim (  t.ragdollvector,g.ragdollvectorindex  );
	}
	t.ragdollvector[g.ragdollvectorindex]=2;
	SetVector2 (  g.ragdollvectorindex,x_f,y_f );
//endfunction ragdollvectorindex
	return g.ragdollvectorindex;
}

int vector3exist ( int vec )
{
	int exists = 0;
	exists=0;
	if (  vec <= ArrayCount(t.ragdollvector) ) 
	{
		if (  t.ragdollvector[vec] != 0 ) 
		{
			exists=1;
		}
	}
//endfunction exists
	return exists
;
}

int deletevector3 ( int vec )
{
	int result = 0;
	if (  t.ragdollvector[vec] == 1 ) 
	{
		result=DeleteVector3(vec);
	}
	if (  t.ragdollvector[vec] == 2 ) 
	{
		result=DeleteVector2(vec);
	}
	t.ragdollvector[vec]=0;
//endfunction result
	return result
;
}

int getlimbbyname ( int objid, char* limbname_s )
{	
	int limbindex = 0;
	int c = 0;
	limbindex=-1;
	cstr limbname;
	limbname=Lower(limbname_s);
	PerformCheckListForLimbs (  objid );
	for ( c = 1 ; c<=  ChecklistQuantity(); c++ )
	{
		if (  cstr(Lower(ChecklistString(c))) == limbname ) 
		{
			limbindex=c-1 ; return limbindex;
		}
	}
//endfunction limbindex
	return limbindex
;
}

