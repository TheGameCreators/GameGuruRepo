gCollectedGem030 = 0

function tzone002_init(e)
end

function tzone002_main(e)
 if g_Entity[e]['plrinzone']==1 and gCollectedGem030==1 then
  PlaySound(e,0)
--  Destroy(e)
--  ActivateIfUsed(e)
  TransportToIfUsed(e)
 end
end
