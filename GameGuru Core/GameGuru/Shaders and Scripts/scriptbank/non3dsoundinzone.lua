-- LUA Script - precede every function and global member with lowercase name of script + '_main'

function non3dsoundinzone_init(e)
end

function non3dsoundinzone_main(e)
 if g_Entity[e]['plrinzone']==1 then
  LoopNon3DSound(e,0)
  Destroy(e)
  ActivateIfUsed(e)
 end
end
