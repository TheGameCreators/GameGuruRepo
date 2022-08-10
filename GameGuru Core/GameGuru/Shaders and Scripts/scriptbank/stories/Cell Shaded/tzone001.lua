gCollectedGem015 = 0

function tzone001_init(e)
end

function tzone001_main(e)
 if g_Entity[e]['plrinzone']==1 and gCollectedGem015==1 then
  PlaySound(e,0)
--  Destroy(e)
--  ActivateIfUsed(e)
  TransportToIfUsed(e)
 end
end
